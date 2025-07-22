#include "gtest/gtest.h"
// #include "car.hpp"
#include <string>
#include <iostream>
#include "yaml-cpp/yaml.h"


TEST(FunctionalTest, YamlParse){
    
    YAML::Node cfg = YAML::LoadFile("../inputs/input.yml");
    double time = cfg["time"].as<double>();
    double timestep = cfg["timestep"].as<double>();
    std::string leadType = cfg["leadType"].as<std::string>();

    ASSERT_EQ(time, 1500);
    ASSERT_EQ(timestep, 1);
    ASSERT_EQ(leadType, "function");
    
    
}

TEST(DiscreteTest, YamlParse){
    YAML::Node cfg = YAML::LoadFile("../inputs/continuous.yml");
    // std::vector<int> velocities = cfg["discrete"].as<std::vector<int>>();
    YAML::Node laneNode = cfg["flow"];

    for (YAML::const_iterator it=laneNode.begin();it!=laneNode.end();++it) {
        YAML::Node node = it->as<YAML::Node>();
        std::cout << node["leadType"].as<std::string>() << " " << node["x0"]<<std::endl;;
    }
    
}


int main(){
    
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}