#pragma once
// Volume3D - Value Object representing a 3D CT volume in Hounsfield Units.

#include <Eigen/Dense>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace neurometrix::core3d::domain {

class Volume3D {
public:
    Volume3D(Eigen::MatrixXf hu_voxels,   // flattened (H*W*D) x 1 or reshaped
             std::uint32_t height,
             std::uint32_t width,
             std::uint32_t depth,
             std::string modality,
             std::string body_part)
        : hu_(std::move(hu_voxels)),
          height_(height),
          width_(width),
          depth_(depth),
          modality_(std::move(modality)),
          body_part_(std::move(body_part)) {}

    [[nodiscard]] const Eigen::MatrixXf& hu() const noexcept { return hu_; }

    // Direct access to a single axial slice (z index).
    [[nodiscard]] Eigen::MatrixXf slice(std::uint32_t z) const {
        const std::uint32_t plane = height_ * width_;
        Eigen::MatrixXf out(height_, width_);
        for (std::uint32_t r = 0; r < height_; ++r) {
            for (std::uint32_t c = 0; c < width_; ++c) {
                out(r, c) = hu_(z * plane + r * width_ + c, 0);
            }
        }
        return out;
    }

    [[nodiscard]] std::uint32_t height() const noexcept { return height_; }
    [[nodiscard]] std::uint32_t width() const noexcept { return width_; }
    [[nodiscard]] std::uint32_t depth() const noexcept { return depth_; }
    [[nodiscard]] bool empty() const noexcept { return hu_.size() == 0; }

    [[nodiscard]] const std::string& modality() const noexcept { return modality_; }
    [[nodiscard]] const std::string& body_part() const noexcept { return body_part_; }

private:
    Eigen::MatrixXf hu_;       // (height*width*depth) x 1, Z-major
    std::uint32_t   height_;
    std::uint32_t   width_;
    std::uint32_t   depth_;
    std::string     modality_;
    std::string     body_part_;
};

} // namespace neurometrix::core3d::domain