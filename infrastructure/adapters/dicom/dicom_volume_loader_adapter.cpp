#include "dicom_volume_loader_adapter.hpp"

#include "../../../infrastructure/adapters/logging/logger.hpp"

#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dctk.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace neurometrix::core3d::infrastructure::dicom {

namespace fs = std::filesystem;

namespace {

[[nodiscard]] std::string get_string(DcmDataset* ds, const DcmTagKey& tag) {
    OFString v;
    if (ds->findAndGetOFString(tag, v).good()) return std::string{v.c_str()};
    return {};
}

[[nodiscard]] double get_double(DcmDataset* ds, const DcmTagKey& tag, double fb) {
    Float64 v = 0.0;
    if (ds->findAndGetFloat64(tag, v).good()) return static_cast<double>(v);
    OFString s;
    if (ds->findAndGetOFString(tag, s).good() && !s.empty()) {
        try { return std::stod(s.c_str()); } catch (...) {}
    }
    return fb;
}

[[nodiscard]] std::int32_t get_int32(DcmDataset* ds, const DcmTagKey& tag, std::int32_t fb) {
    Sint32 v = 0;
    if (ds->findAndGetSint32(tag, v).good()) return static_cast<std::int32_t>(v);
    OFString s;
    if (ds->findAndGetOFString(tag, s).good() && !s.empty()) {
        try { return static_cast<std::int32_t>(std::stoi(s.c_str())); } catch (...) {}
    }
    return fb;
}

[[nodiscard]] std::string to_upper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return s;
}

struct LoadedSlice {
    Eigen::MatrixXf hu;
    std::int32_t    instance_number{0};
    double          slice_position{0.0};
    std::string     study_uid;
    std::string     series_uid;
    std::string     modality;
    std::string     body_part;
};

// Read one DICOM file, apply RescaleSlope/Intercept -> HU.
// Returns the single axial slice.
[[nodiscard]] std::expected<LoadedSlice, application::DicomError>
read_slice(const fs::path& path) {
    DcmFileFormat fmt;
    if (!fmt.loadFile(path.string().c_str()).good()) {
        return std::unexpected(application::DicomError::InvalidHeader);
    }
    DcmDataset* ds = fmt.getDataset();
    if (ds == nullptr) return std::unexpected(application::DicomError::InvalidHeader);

    LoadedSlice s{};
    s.study_uid  = get_string(ds, DCM_StudyInstanceUID);
    s.series_uid = get_string(ds, DCM_SeriesInstanceUID);
    s.modality   = to_upper(get_string(ds, DCM_Modality));
    s.body_part  = to_upper(get_string(ds, DCM_BodyPartExamined));

    if (s.study_uid.empty()) {
        return std::unexpected(application::DicomError::MissingMandatoryTag);
    }

    s.instance_number = get_int32(ds, DCM_InstanceNumber, 0);
    s.slice_position  = get_double(ds, DCM_SliceLocation, 0.0);

    const double slope     = get_double(ds, DCM_RescaleSlope, 1.0);
    const double intercept = get_double(ds, DCM_RescaleIntercept, 0.0);

    Uint16 rows = 0, cols = 0;
    if (ds->findAndGetUint16(DCM_Rows, rows).bad() ||
        ds->findAndGetUint16(DCM_Columns, cols).bad() ||
        rows == 0 || cols == 0) {
        return std::unexpected(application::DicomError::MissingMandatoryTag);
    }

    const Uint16* pixels = nullptr;
    unsigned long count = 0;
    if (ds->findAndGetUint16Array(DCM_PixelData, pixels, &count).bad() ||
        pixels == nullptr || count == 0) {
        return std::unexpected(application::DicomError::CorruptedPixelData);
    }

    const std::size_t frame_pixels =
        static_cast<std::size_t>(rows) * static_cast<std::size_t>(cols);
    if (count < frame_pixels) {
        return std::unexpected(application::DicomError::CorruptedPixelData);
    }

    s.hu = Eigen::MatrixXf(rows, cols);
    for (std::uint32_t r = 0; r < rows; ++r) {
        for (std::uint32_t c = 0; c < cols; ++c) {
            const double raw = static_cast<double>(pixels[r * cols + c]);
            s.hu(r, c) = static_cast<float>(raw * slope + intercept);
        }
    }
    return s;
}

} // anonymous namespace

struct DicomVolumeLoaderAdapter::Impl {};

DicomVolumeLoaderAdapter::DicomVolumeLoaderAdapter() noexcept
    : pimpl_(std::make_unique<Impl>()) {}

DicomVolumeLoaderAdapter::~DicomVolumeLoaderAdapter() = default;

bool DicomVolumeLoaderAdapter::can_read(std::string_view path) const noexcept {
    try {
        const fs::path p{std::string{path}};
        std::error_code ec;
        return fs::exists(p, ec) && !ec;
    } catch (...) { return false; }
}

std::expected<domain::Volume3D, application::DicomError>
DicomVolumeLoaderAdapter::load(std::string_view path) const noexcept {
    try {
        const fs::path p{std::string{path}};
        std::error_code ec;

        if (!fs::exists(p, ec) || ec) {
            return std::unexpected(application::DicomError::FileNotFound);
        }

        // Collect candidate DICOM files
        std::vector<fs::path> files;
        if (fs::is_directory(p, ec)) {
            for (const auto& entry : fs::directory_iterator(p, ec)) {
                if (ec) return std::unexpected(application::DicomError::IOError);
                if (!entry.is_regular_file(ec) || ec) continue;
                const auto& ep = entry.path();
                if (ep.extension() != ".dcm" && ep.extension() != ".DCM") continue;
                files.push_back(ep);
            }
        } else if (fs::is_regular_file(p, ec) && !ec) {
            files.push_back(p);
        } else {
            return std::unexpected(application::DicomError::IOError);
        }

        if (files.empty()) {
            return std::unexpected(application::DicomError::FileNotFound);
        }

        // For directories: need at least 4 slices to be a volume.
        // For single files: we accept only if the file is multiframe (see below).
        std::vector<LoadedSlice> slices;
        slices.reserve(files.size());

        for (const auto& f : files) {
            auto slice = read_slice(f);
            if (!slice) return std::unexpected(slice.error());
            slices.push_back(std::move(*slice));
        }

        if (slices.size() < 4) {
            logging::get()->warn("[dicom-3d] only {} slices; need >= 4 for 3D",
                                 slices.size());
            return std::unexpected(application::DicomError::InsufficientSlices);
        }

        // Validate modality and body part from first slice
        const auto& first = slices.front();
        if (first.modality != "CT") {
            logging::get()->warn("[dicom-3d] rejecting non-CT modality: '{}'", first.modality);
            return std::unexpected(application::DicomError::UnsupportedModality);
        }
        if (!first.body_part.empty() &&
            first.body_part != "HEAD" && first.body_part != "BRAIN" &&
            first.body_part != "SKULL" && first.body_part != "NEURO") {
            logging::get()->warn("[dicom-3d] rejecting non-head body part: '{}'", first.body_part);
            return std::unexpected(application::DicomError::UnsupportedBodyPart);
        }

        // Order slices: prefer InstanceNumber; fallback SliceLocation
        const bool any_instance =
            std::any_of(slices.begin(), slices.end(),
                        [](const LoadedSlice& s) { return s.instance_number != 0; });
        if (any_instance) {
            std::sort(slices.begin(), slices.end(),
                      [](const LoadedSlice& a, const LoadedSlice& b) {
                          return a.instance_number < b.instance_number;
                      });
        } else {
            std::sort(slices.begin(), slices.end(),
                      [](const LoadedSlice& a, const LoadedSlice& b) {
                          return a.slice_position < b.slice_position;
                      });
        }

        const std::uint32_t h = static_cast<std::uint32_t>(slices.front().hu.rows());
        const std::uint32_t w = static_cast<std::uint32_t>(slices.front().hu.cols());
        const std::uint32_t d = static_cast<std::uint32_t>(slices.size());

        // Pack into a single (H*W*D) x 1 matrix, Z-major:
        // index = z * H * W + y * W + x
        const std::uint32_t plane = h * w;
        Eigen::MatrixXf vol(static_cast<Eigen::Index>(plane) * d, 1);

        for (std::uint32_t z = 0; z < d; ++z) {
            const auto& slice = slices[z];
            if (static_cast<std::uint32_t>(slice.hu.rows()) != h ||
                static_cast<std::uint32_t>(slice.hu.cols()) != w) {
                logging::get()->warn("[dicom-3d] inconsistent slice dimensions");
                return std::unexpected(application::DicomError::CorruptedPixelData);
            }
            for (std::uint32_t y = 0; y < h; ++y) {
                for (std::uint32_t x = 0; x < w; ++x) {
                    vol(z * plane + y * w + x) = slice.hu(y, x);
                }
            }
        }

        logging::get()->info("[dicom-3d] loaded volume {}x{}x{} HU[{}..{}]",
                             h, w, d,
                             static_cast<double>(vol.minCoeff()),
                             static_cast<double>(vol.maxCoeff()));

        return domain::Volume3D(std::move(vol), h, w, d,
                                first.modality, first.body_part);
    } catch (const std::bad_alloc&) {
        return std::unexpected(application::DicomError::CorruptedPixelData);
    } catch (...) {
        return std::unexpected(application::DicomError::UnknownError);
    }
}

} // namespace neurometrix::core3d::infrastructure::dicom