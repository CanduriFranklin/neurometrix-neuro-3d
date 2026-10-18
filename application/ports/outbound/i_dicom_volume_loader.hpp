#pragma once

#include "../../../domain/value_objects/volume_3d.hpp"

#include <cstdint>
#include <expected>
#include <string_view>

namespace neurometrix::core3d::application {

enum class DicomError : std::uint8_t {
    FileNotFound,
    InvalidHeader,
    MissingMandatoryTag,
    CorruptedPixelData,
    UnsupportedModality,
    UnsupportedBodyPart,
    InsufficientSlices,        // fewer than N slices for 3D
    IOError,
    UnknownError,
};

class IDicomVolumeLoader {
public:
    virtual ~IDicomVolumeLoader() = default;

    virtual bool can_read(std::string_view path) const noexcept = 0;

    virtual std::expected<domain::Volume3D, DicomError>
    load(std::string_view path) const noexcept = 0;
};

} // namespace neurometrix::core3d::application