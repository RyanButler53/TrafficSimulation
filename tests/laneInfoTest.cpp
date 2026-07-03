#include "sim/laneInfo.hpp"
#include <gtest/gtest.h>

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
