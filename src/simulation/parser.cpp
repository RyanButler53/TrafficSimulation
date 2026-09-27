#include "sim/parser.hpp"
#include "sim/simInputs.hpp"
#include "sim/highway.hpp"
#include "sim/laneInfo.hpp"
#include <iostream>
#include <string>
#include <cmath>
#include <list>
#include <memory>
#include <ranges>
#include <random>
#include <functional>
#include <expected>
#include <algorithm>


std::expected<void, std::string> Parser::parseGeneral(YAML::Node node) {

    if (!node){
        return std::unexpected("Missing \"jobinfo\" field in config file");
    }

    // Gauranteed to be filled in
    totaltime_ = ParseField<double>(node, "time").value_or(100.0);
    dt_ = ParseField<double>(node, "timestep").value_or(1.0);
    seed_ = ParseField<uint64_t>(node, "seed").value_or(0);
    thinning_ = std::max(ParseField<int>(node, "thinning").value_or(1), 1);
    highwayType_ = ParseField<std::string>(node, "highway-type").value_or("cpu");

    auto jobname = ParseField<std::string>(node, "jobname");
    if (!jobname){
        return std::unexpected(jobname.error());
    } else {
        jobname_ = *jobname;
    }

    std::string logtype = ParseField<std::string>(node, "logtype").value_or("file");
    std::string logdir = ParseField<std::string>(node, "logdir").value_or(std::format("./{}", jobname_)); // assumes user has rw access to current dir. 
    std::string drivertype = ParseField<std::string>(node, "driverType").value_or("Gipps");

    if (logtype == "db" or logtype == "test"){
        return DBLogger::make(jobname_, configPath_, drivertype, logtype == "test").transform([this](std::shared_ptr<DBLogger> log){logger_ = log;});
    } else if (logtype == "time-series"){
        logger_ = std::make_shared<TimeSeriesLogger>(logdir, configPath_);
    } else if (logtype == "file") {
        logger_ = std::make_shared<IndividualCarLogger>(logdir, configPath_);
    } else {
        logger_ = std::make_shared<NullLogger>();
    }
    return {};
}

std::expected<void, std::string> Parser::parseCarFactory(YAML::Node cfg){
    // if (!cfg){
    //     return std::unexpected("Missing \"cars\" field in config file");
    // }
    std::string drivertype = ParseField<std::string>(cfg, "driverType").value_or("Gipps");

    if (drivertype == "Gipps"){
        double a = ParseField<double>(cfg, "driverParams", "a").value_or(2.0);
        double b = ParseField<double>(cfg, "driverParams", "b").value_or(-3.0);
        double bmax = ParseField<double>(cfg, "driverParams",  "bmax").value_or(-3.5);
        double p = ParseField<double>(cfg, "driverParams", "p").value_or(0.2);
        double a_stdev = ParseField<double>(cfg, "driverParams", "a_stdev").value_or(0);
        double b_stdev = ParseField<double>(cfg, "driverParams", "b_stdev").value_or(0);
        double bmax_stdev = ParseField<double>(cfg, "driverParams", "bmax_stdev").value_or(0);
        double p_stdev = ParseField<double>(cfg, "driverParams", "p_stdev").value_or(0);
        factory_ = std::make_shared<GippsCarFactory>(a, b, bmax, p, a_stdev, b_stdev, bmax_stdev, p_stdev, seed_);
    } else if (drivertype == "IDM") {
        double a = ParseField<double>(cfg, "driverParams", "a").value_or(2.0);
        double b = ParseField<double>(cfg, "driverParams", "b").value_or(3.0);
        double s0 =ParseField<double>(cfg, "driverParams", "s0").value_or(5);
        double p = ParseField<double>(cfg, "driverParams", "p").value_or(0.2);
        double a_stdev = ParseField<double>(cfg, "driverParams", "a_stdev").value_or(0);
        double b_stdev = ParseField<double>(cfg, "driverParams", "b_stdev").value_or(0);
        double s0_stdev = ParseField<double>(cfg, "driverParams", "s0_stdev").value_or(0);
        double p_stdev = ParseField<double>(cfg, "driverParams", "p_stdev").value_or(0);
        factory_ = std::make_shared<IDMCarFactory>(a, b, s0, p, a_stdev, b_stdev, s0_stdev, p_stdev, seed_);
    } else {
        return std::unexpected("Valid driverType values are [\"Gipps\" and \"IDM\"]");
    }
    return {};
}

std::expected<void, std::string> Parser::parseHighway(YAML::Node hwyNode){
    
    // If the flow is specified in the lane, parse and use that.

    if (!hwyNode){
        return std::unexpected("Must provide a non-null highway node");
    } 
    YAML::Node laneNode = hwyNode["lanes"];
    if (!laneNode){
        return std::unexpected("Must provide a list of lanes with Flow generation and x values.");
    }

    std::vector<std::pair<size_t, FlowGenerator>> flows;
    std::unordered_set<size_t> lanePositions;
    
    double bias = ParseField<double>(hwyNode, "parameters", "bias").value_or(0.2);
    double changePressure = ParseField<double>(hwyNode, "parameters", "changePressure").value_or(0.2);
    double switchThreshold = ParseField<double>(hwyNode, "parameters", "switchThreshold").value_or(1600);

    std::unique_ptr<LaneInfo> lanes = std::make_unique<LaneInfo>(bias, changePressure, switchThreshold);
    auto rng = std::make_shared<std::mt19937>(seed_);
    for (const YAML::Node& node : laneNode) {
        
        double start = ParseField<double>(node, "start").value_or(0);
        double end = ParseField<double>(node, "end").value_or(1000);
        size_t position = ParseField<double>(node, "position").value_or(0);

        std::optional<FlowGenerator> generator;
        if (node["flow"]){ // Flow can be omitted and wll be set to a default flow of zero
            double rate = ParseField<double>(node, "flow", "rate").value_or(100);
            double v0 = ParseField<double>(node, "flow", "v0").value_or(20);
            double vdes = ParseField<double>(node, "flow", "vdes").value_or(25);
            double v0_stdev = ParseField<double>(node, "flow", "v0_stdev").value_or(0);
            double vdes_stdev=ParseField<double>(node, "flow", "vdes_stdev").value_or(0);
            generator = std::make_optional<FlowGenerator>(rate, start, factory_, dt_, rng);
            generator->setRng(
                std::make_shared<NormalDistribution>(v0, v0_stdev),
                std::make_shared<NormalDistribution>(vdes, vdes_stdev),
                std::make_shared<UniformDistribution>(0, 1) // Ensures rate is correctly hit. 
            );
        }
        // Store values for highway ctor 
        flows.push_back({position, generator.value_or(FlowGenerator())});
        lanes->addSegment(start, end, position);
        lanePositions.insert(position);
    }

    // Make correct highway depending on highway type
    if (highwayType_ == "cpu"){
        highway_ = std::make_shared<CpuHighway>(lanePositions.size(), flows, std::move(lanes));
    } else {
        return std::unexpected("No other highway implementation implemented");
    }

    return {};
}


// Put all the parsing together
std::expected<SimulatorInputs, std::string> ContinuousParser::parse() {
    return parseGeneral(cfg_["jobinfo"]).and_then([this](){return parseCarFactory(cfg_["cars"]);})
                         .and_then([this](){return parseHighway(cfg_["highway"]);})
                         .and_then([this](){return parseInitialState(cfg_["inital state"]);})
                         .transform([this](){return SimulatorInputs{logger_, highway_, totaltime_, dt_, thinning_, jobname_};});

}

