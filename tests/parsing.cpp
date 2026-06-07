#include "gtest/gtest.h"
// #include "car.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include "yaml-cpp/yaml.h"
#include "sim/simulator.hpp"
#include "sim/parserFactory.hpp"


class ParsingTest : public ::testing::Test {

    void  SetUp() {
        YAML::Node cfg;
        cfg["jobname"] = "continuous-flow";
        cfg["type"] = "continuous";
        cfg["time"] = 150;
        cfg["timestep"] = 1;
        cfg["seed"] = 70;
        cfg["highway-type"] = "cpu";

        // Driver params (From traffic flow book example)
        cfg["driverType"] = "Gipps";
        cfg["driverParams"]["a"] = 1.981;
        cfg["driverParams"]["b"] = -2.8955;
        cfg["driverParams"]["bmax"] = -5.505;
        cfg["driverParams"]["p"] = 0.2;
        // No randomness
        cfg["driverParams"]["a_stdev"] = 0;
        cfg["driverParams"]["b_stdev"] = 0;
        cfg["driverParams"]["bmax_stdev"] = 0;
        cfg["driverParams"]["p_stdev"] = 0;

        YAML::Node leftLane, rightlane;
        
        rightlane["flow"]["rate"] = 700;
        rightlane["flow"]["v0"] = 0;
        rightlane["flow"]["vdes"] = 40;
        rightlane["start"] = 0;
        rightlane["end"] = 2000;
        rightlane["position"] = 0;

        leftLane["flow"]["rate"] = 800;
        leftLane["flow"]["v0"] = 30;
        leftLane["flow"]["vdes"] = 35;
        leftLane["start"] = 0;
        leftLane["end"] = 2000;
        leftLane["position"] = 1;

        cfg["lanes"].push_back(rightlane);
        cfg["lanes"].push_back(leftLane);

        YAML::Emitter parseOut;
        std::ofstream parseCfg("parseTest.yaml");
        parseOut << cfg;
        parseCfg << parseOut.c_str();
    };

    void TearDown() {

    }
};

TEST_F(ParsingTest, ParseMultiLane){
    std::string configfile = "parseTest.yaml";
    ParserFactory parserFac(configfile);
    SimulatorInputs res = parserFac.makeParser().and_then(std::mem_fn(&Parser::parse)).value();
    ASSERT_DOUBLE_EQ(res.dt_, 1.0);
    ASSERT_EQ(res.totalTime_, 150);

}
