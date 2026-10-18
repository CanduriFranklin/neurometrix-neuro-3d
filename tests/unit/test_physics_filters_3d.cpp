#include "domain/services/physics_filters_3d.hpp"
#include <gtest/gtest.h>
#include <Eigen/Dense>

using namespace neurometrix::core3d::domain;

TEST(PhysicsFilters3D, TVPreservesConstant) {
    const std::uint32_t h = 8, w = 8, d = 4;
    Eigen::MatrixXf vol = Eigen::MatrixXf::Constant(h * w * d, 1, 42.0f);
    const auto out = PhysicsFilters3D::total_variation_denoise(vol, h, w, d, 0.1f, 10);
    EXPECT_LT((out - vol).cwiseAbs().maxCoeff(), 1e-3f);
}

TEST(PhysicsFilters3D, TVReducesNoise) {
    const std::uint32_t h = 8, w = 8, d = 4;
    Eigen::MatrixXf clean = Eigen::MatrixXf::Constant(h * w * d, 1, 100.0f);
    Eigen::MatrixXf noisy = clean;
    for (int i = 0; i < noisy.rows(); ++i)
        noisy(i) += 10.0f * std::sin(0.5f * i);

    const float before = (noisy - clean).squaredNorm();
    const auto d_ = PhysicsFilters3D::total_variation_denoise(noisy, h, w, d, 0.3f, 30);
    const float after = (d_ - clean).squaredNorm();
    EXPECT_LT(after, before);
}

TEST(PhysicsFilters3D, PeronaMalikPreservesConstant) {
    const std::uint32_t h = 8, w = 8, d = 4;
    Eigen::MatrixXf vol = Eigen::MatrixXf::Constant(h * w * d, 1, 7.5f);
    const auto out = PhysicsFilters3D::perona_malik(vol, h, w, d, 25.0f, 5);
    EXPECT_LT((out - vol).cwiseAbs().maxCoeff(), 1e-3f);
}