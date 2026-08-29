#include "sim/laneInfo.hpp"
#include <gtest/gtest.h>
#include <ranges>
#include <algorithm>

class LaneIntervalHolderTest : public ::testing::Test {
    protected:
    LaneInterval lanes_;

    void SetUp(){
        ASSERT_TRUE(lanes_.insert(0, 20, 0));
        ASSERT_TRUE(lanes_.insert(50, 80, 0));
        ASSERT_TRUE(lanes_.insert(0, 55, 1));
        ASSERT_TRUE(lanes_.insert(75, 100, 1));
        ASSERT_TRUE(lanes_.insert(0, 55, 2));
        ASSERT_TRUE(lanes_.insert(80, 100, 2));
    }

};

TEST_F(LaneIntervalHolderTest, getSegment){
    std::optional<LaneBoundary> result;

    result = lanes_.getLaneSegment(0, 15);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), LaneBoundary(0, 20));

    result = lanes_.getLaneSegment(0, 20);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result, LaneBoundary(0, 20));

    result = lanes_.getLaneSegment(0, 55);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result, LaneBoundary(50, 80));

    result = lanes_.getLaneSegment(1, 0);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result, LaneBoundary(0, 55));

    result = lanes_.getLaneSegment(1, 40);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result, LaneBoundary(0, 55));

    result = lanes_.getLaneSegment(1, 75);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result, LaneBoundary(75, 100));

    result = lanes_.getLaneSegment(1, 80);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result, LaneBoundary(75, 100));

    result = lanes_.getLaneSegment(2, 40);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result, LaneBoundary(0, 55));

    result = lanes_.getLaneSegment(2, 80);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result, LaneBoundary(80, 100));

    result = lanes_.getLaneSegment(2, 100);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result, LaneBoundary(80, 100));
}

TEST_F(LaneIntervalHolderTest, nonSegments){
    EXPECT_FALSE(lanes_.getLaneSegment(0, 35).has_value());
    EXPECT_FALSE(lanes_.getLaneSegment(0, -15).has_value());
    EXPECT_FALSE(lanes_.getLaneSegment(0, 120).has_value());
    EXPECT_FALSE(lanes_.getLaneSegment(3, 20).has_value());
    EXPECT_FALSE(lanes_.getLaneSegment(2, 60).has_value());

}



class LaneInfoTest : public ::testing::Test {
    protected:
    LaneInfo info_;
    void SetUp(){
        ASSERT_TRUE(info_.addSegment(0, 20, 0));
        ASSERT_TRUE(info_.addSegment(50, 80, 0));
        ASSERT_TRUE(info_.addSegment(0, 55, 1));
        ASSERT_TRUE(info_.addSegment(75, 100, 1));
        ASSERT_TRUE(info_.addSegment(0, 55, 2));
        ASSERT_TRUE(info_.addSegment(80, 100, 2));
    }
};

TEST_F(LaneInfoTest, BadAdd){
    EXPECT_FALSE(info_.addSegment(15, 40, 0));
    EXPECT_FALSE(info_.addSegment(20, 50, 0));
}

TEST_F(LaneInfoTest, laneValid){
    EXPECT_TRUE(info_.laneValid(40, 1));
    // Boundary case
    EXPECT_TRUE(info_.laneValid(55, 1));

    // Not valid
    EXPECT_FALSE(info_.laneValid(65, 2));
    EXPECT_FALSE(info_.laneValid(30, 3));
}

TEST_F(LaneInfoTest, endOfSegment){
    std::expected<double, std::string> result;

    result = info_.endOfSegment(20, 2);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 55);

    result = info_.endOfSegment(10, 3);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "Lane 3 is not present at x = 10");

    result = info_.endOfSegment(55, 1);
    ASSERT_TRUE(result.value());
    EXPECT_EQ(result.value(), 55);

    result = info_.endOfSegment(115, 2);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "Lane 2 is not present at x = 115");

    result = info_.endOfSegment(100, 1);
    ASSERT_TRUE(result.value());
    EXPECT_EQ(result.value(), 100);
}

TEST_F(LaneInfoTest, endOfLane){
    EXPECT_EQ(info_.endOfRoad(), 100);

    EXPECT_EQ(info_.endOfLane(0), 80);
    EXPECT_EQ(info_.endOfLane(1), 100);
    EXPECT_EQ(info_.endOfLane(2), 100);

    info_.addSegment(95, 115, 0);

    EXPECT_EQ(info_.endOfRoad(), 115);
    EXPECT_EQ(info_.endOfLane(0), 115);
}

TEST_F(LaneInfoTest, lastSegment){

    EXPECT_FALSE(info_.lastSegment(55, 0));
    EXPECT_FALSE(info_.lastSegment(18, 0));
    EXPECT_FALSE(info_.lastSegment(25, 0)); // x = 25 doesn't exist. 

    EXPECT_FALSE(info_.lastSegment(50, 1));
    EXPECT_TRUE(info_.lastSegment(80, 1));
    EXPECT_FALSE(info_.lastSegment(115, 1));

    EXPECT_FALSE(info_.lastSegment(25, 2));
    EXPECT_TRUE(info_.lastSegment(80, 2));
    EXPECT_TRUE(info_.lastSegment(85, 2));
    EXPECT_TRUE(info_.lastSegment(100, 2));
    EXPECT_FALSE(info_.lastSegment(115, 2));

}


class BiasCalculation : public ::testing::Test {
    protected:
    std::unique_ptr<LaneInfo> l;
    void SetUp() override {
        
        l =  std::make_unique<LaneInfo>(0.2, 0.4, 1600);
        l->addSegment(0, 2000, 0);
        l->addSegment(0, 6000, 1);
        l->addSegment(0, 10000, 2);
        l->addSegment(4000, 10000, 0);
        l->addSegment(6400, 10000, 1);
    }
};

TEST_F(BiasCalculation, intoEndOfLane){
    EXPECT_DOUBLE_EQ(l->calculateBias(400, 1, Direction::RIGHT), 0.2);
    EXPECT_DOUBLE_EQ(l->calculateBias(600, 1, Direction::RIGHT), 0.15);
    EXPECT_NEAR(l->calculateBias(1200, 1, Direction::RIGHT), 0.0, 1e-10);
    EXPECT_DOUBLE_EQ(l->calculateBias(1800, 1, Direction::RIGHT), -0.15);
    EXPECT_DOUBLE_EQ(l->calculateBias(2000, 1, Direction::RIGHT), -0.2);

}

TEST_F(BiasCalculation, outOfEndOfLane){

    EXPECT_DOUBLE_EQ(l->calculateBias(400, 0, Direction::LEFT), -0.2);
    EXPECT_DOUBLE_EQ(l->calculateBias(600, 0, Direction::LEFT), -0.15);
    EXPECT_NEAR(l->calculateBias(1200, 0, Direction::LEFT), 0.0, 1e-10);
    EXPECT_DOUBLE_EQ(l->calculateBias(1800, 0, Direction::LEFT), 0.15);
    EXPECT_DOUBLE_EQ(l->calculateBias(2000, 0, Direction::LEFT), 0.2);

}

TEST_F(BiasCalculation, nearEndOfLaneRight){
    EXPECT_DOUBLE_EQ(l->calculateBias(400, 2, Direction::RIGHT), 0.2);
    EXPECT_DOUBLE_EQ(l->calculateBias(600, 2, Direction::RIGHT), 0.175);
    EXPECT_DOUBLE_EQ(l->calculateBias(1200, 2, Direction::RIGHT), 0.1);
    EXPECT_DOUBLE_EQ(l->calculateBias(1800, 2, Direction::RIGHT), 0.025);
    EXPECT_NEAR(l->calculateBias(2000, 2, Direction::RIGHT), 0, 1e-10);
}

TEST_F(BiasCalculation, nearEndOfLaneLeft){
    EXPECT_DOUBLE_EQ(l->calculateBias(400, 1, Direction::LEFT), -0.2);
    EXPECT_DOUBLE_EQ(l->calculateBias(600, 1, Direction::LEFT), -0.175);
    EXPECT_DOUBLE_EQ(l->calculateBias(1200, 1, Direction::LEFT), -0.1);
    EXPECT_DOUBLE_EQ(l->calculateBias(1800, 1, Direction::LEFT), -0.025);
    EXPECT_DOUBLE_EQ(l->calculateBias(2000, 1, Direction::LEFT), 0);

    // After x = 2000, normal bias rules apply again. 
    EXPECT_DOUBLE_EQ(l->calculateBias(2001, 1, Direction::LEFT), -0.2);

}

TEST_F(BiasCalculation, intoOfEndOfLaneLeft){

    EXPECT_DOUBLE_EQ(l->calculateBias(4400, 1, Direction::RIGHT), 0.2);
    EXPECT_DOUBLE_EQ(l->calculateBias(4600, 1, Direction::RIGHT), 0.25);
    EXPECT_DOUBLE_EQ(l->calculateBias(5200, 1, Direction::RIGHT), 0.4);
    EXPECT_DOUBLE_EQ(l->calculateBias(5800, 1, Direction::RIGHT), 0.55);
    EXPECT_DOUBLE_EQ(l->calculateBias(6000, 1, Direction::RIGHT), 0.6);
}

TEST_F(BiasCalculation, outOfEndOfLaneLeft){

    EXPECT_DOUBLE_EQ(l->calculateBias(4400, 0, Direction::LEFT), -0.2);
    EXPECT_DOUBLE_EQ(l->calculateBias(4600, 0, Direction::LEFT), -0.25);
    EXPECT_DOUBLE_EQ(l->calculateBias(5200, 0, Direction::LEFT), -0.4);
    EXPECT_DOUBLE_EQ(l->calculateBias(5800, 0, Direction::LEFT), -0.55);
    EXPECT_DOUBLE_EQ(l->calculateBias(6000, 0, Direction::LEFT), -0.6);
}

TEST_F(BiasCalculation, lastSegmentLeftLane){
    for (size_t i : std::views::iota(0,3500)){
        ASSERT_DOUBLE_EQ(l->calculateBias(6500 + double(i), 1, Direction::RIGHT), 0.2) << "X: " << 6500 + double(i);
        ASSERT_DOUBLE_EQ(l->calculateBias(6500 + double(i), 0, Direction::LEFT), -0.2) << "X: " << 6500 + double(i);
    }
}
