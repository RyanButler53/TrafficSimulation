#include "gtest/gtest.h"
// #include "car.hpp"
#include <string>
#include <iostream>
#include "yaml-cpp/yaml.h"


TEST(FunctionalTest, YamlParse){
    
    YAML::Node cfg = YAML::LoadFile("../inputs/input.yml");
    int numCars = cfg["numCars"].as<int>();
    double time = cfg["time"].as<double>();
    double timestep = cfg["timestep"].as<double>();
    std::string leadType = cfg["leadType"].as<std::string>();

    ASSERT_EQ(numCars, 5);
    ASSERT_EQ(time, 30);
    ASSERT_EQ(timestep, 1);
    
    
}

TEST(DiscreteTest, YamlParse){
    YAML::Node cfg = YAML::LoadFile("../inputs/test.yml");
    // std::vector<int> velocities = cfg["discrete"].as<std::vector<int>>();
    YAML::Node discrete = cfg["discrete"];

    std::vector<double> velocities;
    
    std::transform(discrete.begin(), discrete.end(), std::back_inserter(velocities), [](const YAML::Node& n){return n.as<double>();});
    for (auto x : discrete){
        std::cout << x << " ";
    }
    std::cout << std::endl;
}


int main(){
    
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}