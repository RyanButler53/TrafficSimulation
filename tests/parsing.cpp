#include "gtest/gtest.h"
#include <string>
#include "yaml-cpp/yaml.h"
#include "sim/simulator.hpp"
#include "sim/parserFactory.hpp"
#include "testUtil.hpp"


class ParsingTest : public ::testing::Test {

    void  SetUp() override {
        YAML::Node cfg;
        cfg["jobinfo"]["jobname"] = "continuous-flow";
        cfg["jobinfo"]["type"] = "continuous";
        cfg["jobinfo"]["time"] = 150;
        cfg["jobinfo"]["timestep"] = 1;
        cfg["jobinfo"]["seed"] = 70;
        cfg["jobinfo"]["highway-type"] = "cpu";
        cfg["jobinfo"]["thinning"] = 2;

        // Driver params (From traffic flow book example)
        cfg["cars"]["driverType"] = "Gipps";
        cfg["cars"]["driverParams"]["a"] = 1.981;
        cfg["cars"]["driverParams"]["b"] = -2.8955;
        cfg["cars"]["driverParams"]["bmax"] = -5.505;
        cfg["cars"]["driverParams"]["p"] = 0.2;
        // No randomness
        cfg["cars"]["driverParams"]["a_stdev"] = 0;
        cfg["cars"]["driverParams"]["b_stdev"] = 0;
        cfg["cars"]["driverParams"]["bmax_stdev"] = 0;
        cfg["cars"]["driverParams"]["p_stdev"] = 0;

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

        cfg["highway"]["lanes"].push_back(rightlane);
        cfg["highway"]["lanes"].push_back(leftLane);

        TestUtil::configToFile(cfg, "parseTest.yaml");
        cfg["jobinfo"]["thinning"] = -2;
        TestUtil::configToFile(cfg, "invalidThinning.yaml" );
    };

    void TearDown() override {
        std::filesystem::remove("parseTest.yaml");
        std::filesystem::remove("invalidThinning.yaml");
    }
};

TEST_F(ParsingTest, ParseMultiLane){
    std::string configfile = "parseTest.yaml";
    ParserFactory parserFac(configfile);
    SimulatorInputs res = parserFac.makeParser().and_then(std::mem_fn(&Parser::parse)).value();
    ASSERT_DOUBLE_EQ(res.dt_, 1.0);
    ASSERT_EQ(res.totalTime_, 150);
    EXPECT_EQ(res.thinning_, 2);
}

TEST_F(ParsingTest, invalidThinning){
    std::string configfile = "invalidThinning.yaml";
    ParserFactory parserFac(configfile);
    SimulatorInputs res = parserFac.makeParser().and_then(std::mem_fn(&Parser::parse)).value();
    EXPECT_EQ(res.thinning_, 1);
}
