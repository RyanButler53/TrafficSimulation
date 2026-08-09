#include <gtest/gtest.h>
#include <ranges>
#include <filesystem>
#include <fstream>

#include "AlgTest.hpp"
#include "testUtil.hpp"

void AlgTest::forwardMovement(const std::vector<RawData>& raw){
    for (const RawData& r : raw){
        EXPECT_TRUE(std::ranges::is_sorted(r.x_));
        EXPECT_TRUE(std::ranges::all_of(r.v_, [](float v){return v >= 0.0;}));
    }
}

void AlgTest::laneChanges(const std::vector<RawData>& raw){
    bool leftLaneChange = false;
    bool rightLaneChange = false;
    for (const RawData& r  : raw){
        // Check for both left and right lane changes
        for (size_t i : std::views::iota(0UL, r.l_.size() - 1)){
            int oldLane = r.l_[i];
            int newLane = r.l_[i+1];
            if (oldLane < newLane){
                leftLaneChange = true;
            } else if (oldLane > newLane) {
                rightLaneChange = true;
            }

            EXPECT_GE(oldLane, 0);
            EXPECT_LE(oldLane, 2);
        }
    }
    EXPECT_TRUE(leftLaneChange) << "No left lane changes found";
    EXPECT_TRUE(rightLaneChange) << "No right lane changes found";
}

void AlgTest::completeLaneBoundsCheck(const std::vector<RawData>& raw, float roadEnd){
    for (const RawData& r  : raw){
        EXPECT_GE(r.x_.front(), 0.0);
        EXPECT_LE(r.x_.back(), roadEnd);
    }
}

// 3 lane
std::string Test3Lane::testName(){
    return "3LaneAlgorithm";
}

std::filesystem::path Test3Lane::filename(){
    return "3LaneTest.yml";
}

void Test3Lane::generateInput(){
    YAML::Node cfg = TestUtil::getConfigNode_3Lane();
    cfg["logtype"] = "test";
    cfg["jobname"] = testName();
    cfg["timestep"] = 0.1; // dt = 0.1 tests streaming

    // Heterogeneous traffic
    cfg["driverParams"]["a_stdev"] = 0.1;
    cfg["driverParams"]["b_stdev"] = 0.2;
    cfg["driverParams"]["bmax_stdev"] = 0.2;
    cfg["driverParams"]["p_stdev"] = 0.02;

    TestUtil::configToFile(cfg, filename());
}

void Test3Lane::inBounds(const std::vector<RawData>& raw){
    completeLaneBoundsCheck(raw, 2000);
}

void Test3Lane::flowGeneration(const std::vector<RawData>& raw){
    std::array<size_t, 3> laneStarted{0,0,0};
    for (const RawData& r  : raw){
        ++laneStarted[r.l_.front()];
    }
    for (auto i : std::views::iota(0, 3)){
        EXPECT_GT(laneStarted[i], 0) << "Lane " << i << " has no cars starting in it";
    }
}

// Zero Flow case

std::string TestZeroFlow::testName(){
    return "ZeroFlowAlgorithm";
}

std::filesystem::path TestZeroFlow::filename(){
    return "ZeroFlow.yml";
}

void TestZeroFlow::generateInput(){
    YAML::Node cfg = TestUtil::getConfigNode();
    cfg["lanes"][0]["flow"]["rate"] = 0;
    cfg["time"] = 1000;
    cfg["logtype"] = "test";
    cfg["jobname"] = testName();

    TestUtil::configToFile(cfg, filename());
}

void TestZeroFlow::inBounds(const std::vector<RawData>& raw){
    completeLaneBoundsCheck(raw, 2000);
}

void TestZeroFlow::flowGeneration(const std::vector<RawData>& raw){
    std::array<size_t, 2> laneStarted{0,0};
    for (const RawData& r  : raw){
        ++laneStarted[r.l_.front()];
    }
    EXPECT_EQ(laneStarted[0], 0) << "A car started in lane 0";
    EXPECT_GT(laneStarted[1], 0) << "No Cars started in lane 1";

}