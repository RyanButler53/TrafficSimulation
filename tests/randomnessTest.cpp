#include <gtest/gtest.h>
#include <ranges>
#include <memory>

#include "sim/random.hpp"


// Test if the randomness is able to be stored and reloaded appropriately
class RandomnessTest : public ::testing::TestWithParam<int>{
  protected:
    int n_;
    std::unique_ptr<UniformDistribution> uniform_;
    std::unique_ptr<NormalDistribution>  normal_;
    std::unique_ptr<NormalDistribution>  zeroStdev_;
    RngEngine rng_;

    void SetUp() override {
        n_ = GetParam();
        uniform_   = std::make_unique<UniformDistribution>(0, 1);
        normal_    = std::make_unique<NormalDistribution>(20, 1);
        zeroStdev_ = std::make_unique<NormalDistribution>(5, 0);
        rng_       = RngEngine{std::mt19937{140}};

        for (auto _ : std::views::iota(0, n_)) {
            uniform_->getValue(rng_);
            normal_->getValue(rng_);
            zeroStdev_->getValue(rng_);
        }
    }

    // Saves state, restores into a fresh distribution + engine, and checks
    // that both produce the same next n_ values.
    template <typename Dist>
    void expectRestoreMatches(Dist& original) {
        const std::string savedState = original.getState();
        const uint64_t rngState = rng_.getState();

        Dist restored(savedState);
        RngEngine restoredRng(std::mt19937{140});
        restoredRng.discard(rngState);

        for (auto _ : std::views::iota(0, n_)) {
            EXPECT_EQ(original.getValue(rng_), restored.getValue(restoredRng));
        }
    }
};

TEST_P(RandomnessTest, RestoreUniformDist)    { expectRestoreMatches(*uniform_); }
TEST_P(RandomnessTest, RestoreNormalDist)     { expectRestoreMatches(*normal_); }
TEST_P(RandomnessTest, RestoreZeroStdevDist)  { expectRestoreMatches(*zeroStdev_); }

TEST_P(RandomnessTest, ZeroStdevIsConstant) {
    for (auto _ : std::views::iota(0, n_))
        EXPECT_EQ(zeroStdev_->getValue(rng_), 5);
}

INSTANTIATE_TEST_SUITE_P(
    WarmupRolls,
    RandomnessTest,
    ::testing::Values(10, 10000));

TEST(RandomnessTest, DifferentValues) {
    auto dist = std::make_unique<UniformDistribution>(0, 1);
    auto rng = RngEngine{std::mt19937{140}};
    double sum = 0;

    for (auto _ : std::views::iota(0, 100))
        sum += dist->getValue(rng);
    EXPECT_NEAR(sum, 50, 5);
}