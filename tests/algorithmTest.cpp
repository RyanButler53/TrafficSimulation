// Algorithm test for Traffic Simulator
#include <gtest/gtest.h>
#include <expected>
#include <filesystem>
#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <ranges>
#include "sim/simulator.hpp"
#include "yaml-cpp/yaml.h"

#include "sim/parser.hpp"
#include "sim/parserFactory.hpp"
#include "sim/simulator.hpp"
#include "api/DBManager.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>

#include "testUtil.hpp"

class AlgorithmTest : public ::testing::Test{

    protected:
    std::vector<RawData> rawData_;

    void SetUp() override{
        YAML::Node cfg = TestUtil::getConfigNode_3Lane();
        cfg["logtype"] = "test";
        cfg["jobname"] = "Algorithm";

        // Homogeneous traffic
        cfg["driverParams"]["a_stdev"] = 0.1;
        cfg["driverParams"]["b_stdev"] = 0.2;
        cfg["driverParams"]["bmax_stdev"] = 0.2;
        cfg["driverParams"]["p_stdev"] = 0.02;

        YAML::Emitter cfgout;
        std::ofstream fileCfg("Algorithm.yaml");
        cfgout << cfg;
        fileCfg << cfgout.c_str();
        fileCfg.close();

        TestUtil::clearDB();

        auto simResult = Traffic::Simulate("Algorithm.yaml");
        ASSERT_TRUE(simResult.has_value()) << "Simulator Failed: " << simResult.error();
        DBManager reader(true);
        std::expected<std::vector<RawData>, std::string> raw = reader.queryData("Algorithm");
        ASSERT_TRUE(raw.has_value()) << raw.error();
        rawData_ = *raw;
    }

    void TearDown() override {
        TestUtil::clearDB();
        if (std::filesystem::exists("Algorithm.yaml")) std::filesystem::remove("Algorithm.yaml");
    }
};

TEST_F(AlgorithmTest, CheckAlgorithm){
    std::array<size_t, 3> laneStarted;
    bool leftLaneChange;
    bool rightLaneChange;
    const float roadEnd = 2000.0;
    for (RawData& r  : rawData_){
        EXPECT_TRUE(std::ranges::is_sorted(r.x_));
        EXPECT_TRUE(std::ranges::all_of(r.v_, [](float v){return v >= 0.0;}));
        EXPECT_TRUE(r.x_.back() <= roadEnd);

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

        // Initial Lane check: 
        ++laneStarted[r.l_.front()];
        
    }

    EXPECT_TRUE(leftLaneChange) << "No left lane changes found";
    EXPECT_TRUE(rightLaneChange) << "No right lane changes found";

    for (auto i : std::views::iota(0, 3)){
        EXPECT_GT(laneStarted[i], 0) << "Lane " << i << " has no cars starting in it";
    }
}