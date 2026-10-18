#include "domain/services/morphological_ops_3d.hpp"
#include <gtest/gtest.h>
#include <Eigen/Dense>

using namespace neurometrix::core3d::domain;

TEST(Morphology3D, DilationIncreasesOrEquals) {
    const std::uint32_t h = 8, w = 8, d = 4;
    Eigen::MatrixXf vol = Eigen::MatrixXf::Random(h * w * d, 1);
    const auto d_ = MorphologicalOps3D::dilate(vol, h, w, d, 1);
    EXPECT_TRUE((d_.array() >= vol.array() - 1e-6f).all());
}

TEST(Morphology3D, ErosionDecreasesOrEquals) {
    const std::uint32_t h = 8, w = 8, d = 4;
    Eigen::MatrixXf vol = Eigen::MatrixXf::Random(h * w * d, 1);
    const auto e = MorphologicalOps3D::erode(vol, h, w, d, 1);
    EXPECT_TRUE((e.array() <= vol.array() + 1e-6f).all());
}

TEST(Morphology3D, WhiteTopHatConstantIsZero) {
    const std::uint32_t h = 8, w = 8, d = 4;
    Eigen::MatrixXf vol = Eigen::MatrixXf::Constant(h * w * d, 1, 5.0f);
    const auto th = MorphologicalOps3D::white_tophat(vol, h, w, d, 2);
    EXPECT_LT(th.cwiseAbs().maxCoeff(), 1e-3f);
}

TEST(Morphology3D, WhiteTopHatDetectsBrightSpot) {
    const std::uint32_t h = 8, w = 8, d = 8;
    Eigen::MatrixXf vol = Eigen::MatrixXf::Zero(h * w * d, 1);
    // Plant a 3x3x3 bright spot at center (4,4,4)
    const std::uint32_t plane = h * w;
    for (std::uint32_t z = 3; z < 6; ++z)
        for (std::uint32_t y = 3; y < 6; ++y)
            for (std::uint32_t x = 3; x < 6; ++x)
                vol(z * plane + y * w + x) = 100.0f;

    const auto th = MorphologicalOps3D::white_tophat(vol, h, w, d, 2);
    const std::uint32_t center = 4 * plane + 4 * w + 4;
    EXPECT_GT(th(center), 50.0f);
}