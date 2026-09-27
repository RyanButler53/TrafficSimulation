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
            EXPECT_LE(oldLane, nlanes());
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

void  AlgTest::simpleFlowGenerationCheck(const std::vector<RawData>& raw, size_t n){
    std::vector<size_t> laneStarted(n);
    std::ranges::fill(laneStarted, 0);
    for (const RawData& r  : raw){
        ++laneStarted[r.l_.front()];
    }
    for (auto i : std::views::iota(0, 3)){
        EXPECT_GT(laneStarted[i], 0) << "Lane " << i << " has no cars starting in it";
    }
}

// 3 lane
std::string Test3Lane::testName(){
    return "3LaneAlgorithm";
}

std::filesystem::path Test3Lane::filename(){
    return "3LaneTest.yml";
}

size_t Test3Lane::nlanes() const {
    return 2;
}

void Test3Lane::generateInput(){
    YAML::Node cfg = TestUtil::getConfigNode_3Lane();
    cfg["jobinfo"]["logtype"] = "test";
    cfg["jobinfo"]["jobname"] = testName();
    cfg["jobinfo"]["timestep"] = 0.1; // dt = 0.1 tests streaming

    // Heterogeneous traffic
    cfg["cars"]["driverParams"]["a_stdev"] = 0.1;
    cfg["cars"]["driverParams"]["b_stdev"] = 0.2;
    cfg["cars"]["driverParams"]["bmax_stdev"] = 0.2;
    cfg["cars"]["driverParams"]["p_stdev"] = 0.02;

    TestUtil::configToFile(cfg, filename());
}

void Test3Lane::inBounds(const std::vector<RawData>& raw){
    completeLaneBoundsCheck(raw, 2000);
}

void Test3Lane::flowGeneration(const std::vector<RawData>& raw){
    simpleFlowGenerationCheck(raw, 3);
}

// Zero Flow case

std::string TestZeroFlow::testName(){
    return "ZeroFlowAlgorithm";
}

std::filesystem::path TestZeroFlow::filename(){
    return "ZeroFlow.yml";
}

size_t TestZeroFlow::nlanes() const {
    return 1;
}

void TestZeroFlow::generateInput(){
    YAML::Node cfg = TestUtil::getConfigNode();
    cfg["highway"]["lanes"][0]["flow"]["rate"] = 0;
    cfg["jobinfo"]["time"] = 1000;
    cfg["jobinfo"]["logtype"] = "test";
    cfg["jobinfo"]["jobname"] = testName();

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

// Lane closure case

std::string TestLaneClosure::testName(){
    return "LaneClosureAlgorithm";
}

std::filesystem::path TestLaneClosure::filename(){
    return "LaneClosure.yml";
}

size_t TestLaneClosure::nlanes() const {
    return 3;
}

void TestLaneClosure::generateInput(){
    YAML::Node cfg;

    // General Information
    cfg["jobinfo"]["jobname"] = testName();
    cfg["jobinfo"]["type"] = "continuous";
    cfg["jobinfo"]["time"] = 500;
    cfg["jobinfo"]["timestep"] = 0.2;
    cfg["jobinfo"]["seed"] = 70;
    cfg["jobinfo"]["logtype"] = "test";
    cfg["jobinfo"]["highway-type"] = "cpu";
 
    // Driver Model
    cfg["cars"]["driverType"] = "Gipps";
    cfg["cars"]["driverParams"]["a"] = 1.981;
    cfg["cars"]["driverParams"]["b"] = -2.8955;
    cfg["cars"]["driverParams"]["bmax"] = -3.505;
    cfg["cars"]["driverParams"]["p"] = 0.2;
    cfg["cars"]["driverParams"]["a_stdev"] = 0.1;
    cfg["cars"]["driverParams"]["b_stdev"] = 0.1;
    cfg["cars"]["driverParams"]["bmax_stdev"] = 0.1;
    cfg["cars"]["driverParams"]["p_stdev"] = 0.04;
 
    // Highway Parameters
    cfg["highway"]["parameters"]["changePressure"] = 0.6;
    cfg["highway"]["parameters"]["switchThreshold"] = 1600;
    cfg["highway"]["parameters"]["bias"] = 0.2;
 
    // Lanes: must have flow, start, end and position
    cfg["highway"]["lanes"][0]["flow"]["rate"] = 200;
    cfg["highway"]["lanes"][0]["flow"]["v0"] = 20;
    cfg["highway"]["lanes"][0]["flow"]["vdes"] = 20;
    cfg["highway"]["lanes"][0]["flow"]["v0_stdev"] = 2;
    cfg["highway"]["lanes"][0]["flow"]["vdes_stdev"] = 2;
    cfg["highway"]["lanes"][0]["start"] = 0;
    cfg["highway"]["lanes"][0]["end"] = 2000;
    cfg["highway"]["lanes"][0]["position"] = 0; // right lane
 
    cfg["highway"]["lanes"][1]["flow"]["rate"] = 200;
    cfg["highway"]["lanes"][1]["flow"]["v0"] = 0;
    cfg["highway"]["lanes"][1]["flow"]["vdes"] = 30;
    cfg["highway"]["lanes"][1]["flow"]["v0_stdev"] = 0;
    cfg["highway"]["lanes"][1]["flow"]["vdes_stdev"] = 2;
    cfg["highway"]["lanes"][1]["start"] = 5000;
    cfg["highway"]["lanes"][1]["end"] = 10000;
    cfg["highway"]["lanes"][1]["position"] = 0; // right lane
 
    cfg["highway"]["lanes"][2]["flow"]["rate"] = 400;
    cfg["highway"]["lanes"][2]["flow"]["v0"] = 20;
    cfg["highway"]["lanes"][2]["flow"]["vdes"] = 25;
    cfg["highway"]["lanes"][2]["flow"]["v0_stdev"] = 2;
    cfg["highway"]["lanes"][2]["flow"]["vdes_stdev"] = 2;
    cfg["highway"]["lanes"][2]["start"] = 0;
    cfg["highway"]["lanes"][2]["end"] = 10000;
    cfg["highway"]["lanes"][2]["position"] = 1; // middle lane
 
    cfg["highway"]["lanes"][3]["flow"]["rate"] = 600;
    cfg["highway"]["lanes"][3]["flow"]["v0"] = 25;
    cfg["highway"]["lanes"][3]["flow"]["vdes"] = 25;
    cfg["highway"]["lanes"][3]["flow"]["v0_stdev"] = 2;
    cfg["highway"]["lanes"][3]["flow"]["vdes_stdev"] = 2;
    cfg["highway"]["lanes"][3]["start"] = 0;
    cfg["highway"]["lanes"][3]["end"] = 10000;
    cfg["highway"]["lanes"][3]["position"] = 2; // left lane
 
    cfg["highway"]["lanes"][4]["flow"]["rate"] = 0;
    cfg["highway"]["lanes"][4]["start"] = 6000;
    cfg["highway"]["lanes"][4]["end"] = 10000;
    cfg["highway"]["lanes"][4]["position"] = 3; // newly added leftmost lane
    TestUtil::configToFile(cfg, filename());
}

void TestLaneClosure::inBounds(const std::vector<RawData>& raw){
    completeLaneBoundsCheck(raw, 10000);
    for (const RawData& car : raw){
        for (size_t timestep : std::views::iota(0UL, car.l_.size())){
            if (car.l_[timestep] == 0){
                EXPECT_TRUE((car.x_[timestep] < 2000) || (car.x_[timestep] >= 5000.0)) << "Car found in invalid location";
            }
        }
    }
}

void TestLaneClosure::flowGeneration(const std::vector<RawData>& raw){
    simpleFlowGenerationCheck(raw, 3);
    // Does testing flow generation involve showing that lane 3 gets real use? 
}

