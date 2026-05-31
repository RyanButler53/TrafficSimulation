#include "sim/parser.hpp"
#include "sim/simInputs.hpp"
#include "sim/highway.hpp"
#include <iostream>
#include <string>
#include <cmath>
#include <list>
#include <ranges>
#include <functional>
#include <expected>
#include <algorithm>


std::expected<void, std::string> Parser::parseGeneral() {

    // Gauranteed to be filled in
    totaltime_ = ParseField<double>(cfg_, "time").value_or(100.0);
    dt_ = ParseField<double>(cfg_, "timestep").value_or(1.0);
    seed_ = ParseField<uint64_t>(cfg_, "seed").value_or(0);

    // Job name is not gauranteed to be present, probably needs to match the DB

    std::string logtype = ParseField<std::string>(cfg_, "logtype").value_or("file");
    std::string jobname = ParseField<std::string>(cfg_, "jobname").value(); // needs a default that is the specified by api call. 
    std::string logdir = ParseField<std::string>(cfg_, "logdir").value_or(std::format("./{}", jobname)); // assumes user has rw access to current dir. 
    std::string drivertype = ParseField<std::string>(cfg_, "driverType").value_or("Gipps");

    if (logtype == "db" or logtype == "test"){
        return DBLogger::make(jobname, configPath_, drivertype, logtype == "test").transform([this](std::shared_ptr<DBLogger> log){logger_ = log;});
    } else {
        logger_ = std::make_shared<FileLogger>(logdir);
        return {};
    }
}

std::expected<void, std::string> Parser::parseCarFactory(){
    std::string drivertype = ParseField<std::string>(cfg_, "driverType").value_or("Gipps");

    if (drivertype == "Gipps"){
        double a = ParseField<double>(cfg_, "driverParams", "a").value_or(2.0);
        double b = ParseField<double>(cfg_, "driverParams", "b").value_or(-3.0);
        double bmax = ParseField<double>(cfg_, "driverParams",  "bmax").value_or(-3.5);
        double p = ParseField<double>(cfg_, "driverParams", "p").value_or(0.2);
        double a_stdev = ParseField<double>(cfg_, "driverParams", "a_stdev").value_or(0);
        double b_stdev = ParseField<double>(cfg_, "driverParams", "b_stdev").value_or(0);
        double bmax_stdev = ParseField<double>(cfg_, "driverParams", "bmax_stdev").value_or(0);
        double p_stdev = ParseField<double>(cfg_, "driverParams", "p_stdev").value_or(0);
        factory_ = std::make_shared<GippsCarFactory>(a, b, bmax, p, a_stdev, b_stdev, bmax_stdev, p_stdev, seed_);
    } else if (drivertype == "IDM") {
        double a = ParseField<double>(cfg_, "driverParams", "a").value_or(2.0);
        double b = ParseField<double>(cfg_, "driverParams", "b").value_or(3.0);
        double s0 =ParseField<double>(cfg_, "driverParams", "s0").value_or(5);
        double p = ParseField<double>(cfg_, "driverParams", "p").value_or(0.2);
        double a_stdev = ParseField<double>(cfg_, "driverParams", "a_stdev").value_or(0);
        double b_stdev = ParseField<double>(cfg_, "driverParams", "b_stdev").value_or(0);
        double s0_stdev = ParseField<double>(cfg_, "driverParams", "s0_stdev").value_or(0);
        double p_stdev = ParseField<double>(cfg_, "driverParams", "p_stdev").value_or(0);
        factory_ = std::make_shared<IDMCarFactory>(a, b, s0, p, a_stdev, b_stdev, s0_stdev, p_stdev, seed_);
    } else {
        return std::unexpected("Valid driverType values are [\"Gipps\" and \"IDM\"]");
    }
    return {};
}

// Put all the parsing together
std::expected<SimulatorInputs, std::string> Parser::parse() {
    return parseGeneral().and_then([this](){return parseCarFactory();})
                         .and_then([this](){return parseHighway();})
                         .transform([this](){return SimulatorInputs{logger_, highway_, totaltime_, dt_};});

}

std::expected<void, std::string> ContinuousParser::parseHighway(){
    
    // If the flow is specified in the lane, parse and use that. 
    YAML::Node laneNode = cfg_["lanes"];
    if (!laneNode){
        return std::unexpected("Must provide a list of lanes with Flow generation and x values.");
    } 
    std::string hwyType = ParseField<std::string>(cfg_, "highway-type").value_or("cpu"); // cpu, kokkos, metal
    double roadEnd =ParseField<double>(cfg_, "road-end").value_or(1000); // All lanes end after 1000m

    std::vector<FlowGenerator> flows(laneNode.size());
    for (const YAML::Node& node : laneNode) {

        double rate = ParseField<double>(node, "flow", "rate").value_or(100);
        double v0 = ParseField<double>(node, "flow", "v0").value_or(30);
        double vdes = ParseField<double>(node, "flow", "vdes").value_or(35);
        double start = ParseField<double>(node, "start").value_or(0);
        double end = ParseField<double>(node, "end").value_or(1000);
        size_t position = ParseField<double>(node, "position").value_or(0);

        flows[position] = FlowGenerator(rate, start, v0, vdes, factory_, dt_, seed_);
    }

    // Make correct highway depending on highway type
    if (hwyType == "cpu"){
        highway_ = std::make_shared<CpuHighway>(flows.size(), flows, roadEnd);
    } else {
        return std::unexpected("No other highway implementation implemented");
    }

    return {};
}
