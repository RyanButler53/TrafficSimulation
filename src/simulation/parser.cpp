#include "sim/parser.hpp"
#include "sim/simInputs.hpp"
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
    std::string logdir = ParseField<std::string>(cfg_, "logdir").value_or("."); // assumes user has rw access to current dir. 
    if (logtype == "db"){
        logger_ = std::make_shared<DBLogger>(jobname, configPath_);
    } else if (logtype == "test"){
        logger_ = std::make_shared<DBLoggerTest>(jobname, configPath_);
    } else {
        logger_ = std::make_shared<FileLogger>(logdir);
    }
    
    if (!logger_){
        return std::unexpected("Error constructing the logger");
    } 
    return {};
}

std::expected<void, std::string> Parser::parseCarFactory(){
    // this can raise an exception...
    std::string drivertype = ParseField<std::string>(cfg_, "driverType").value_or("Gipps");
    YAML::Node driverParams = cfg_["driverParams"] ;

    if (drivertype == "Gipps"){
        double a = ParseField<double>(driverParams, "a").value_or(2.0);
        double b = ParseField<double>(driverParams, "b").value_or(-3.0);
        double bmax = ParseField<double>(driverParams, "bmax").value_or(-3.5);
        factory_ = std::make_shared<GippsCarFactory>(a,b,bmax, logger_);
    } else if (drivertype == "IDM") {
        double a = ParseField<double>(driverParams, "a").value_or(2.0);
        double b = ParseField<double>(driverParams, "a").value_or(3.0);
        double s0 =ParseField<double>(driverParams, "a").value_or(5);
        factory_ = std::make_shared<IDMCarFactory>(a,b,s0, logger_);
    } else {
        return std::unexpected("Valid driverType values are [\"Gipps\" and \"IDM\"]");
    }
    return {};
}

std::expected<std::shared_ptr<LeadStrategy>, std::string> Parser::parseLeadStrategy(YAML::Node leadNode){
    std::string leadtype = ParseField<std::string>(leadNode, "leadType").value_or(""); // hold error

    // Utility to get yaml[fntype][key] and error check. 
    std::function<YAML::Node(std::string, std::string)> getNode = [this, &leadNode](std::string fntype, std::string key){
        return  ParseField<YAML::Node>(leadNode, fntype).and_then([this, key](YAML::Node n){
            return ParseField<YAML::Node>(n, key);
        }).value_or(YAML::Node());
    };
    if (leadtype == "function"){
        // looking for Sine, Constant, Piecewise
        // Do piecewise later. 
        // YAML::Node sine = leadNode["function"]["sine"];
        YAML::Node sine = getNode("function", "sine");
        double a = ParseField<double>(sine, "a").value_or(40);
        double b = ParseField<double>(sine, "b").value_or(0);
        double c = ParseField<double>(sine, "a").value_or(0);
        return std::make_shared<FunctionLead>(a,b,c);
        
    } else if (leadtype == "discrete") {
        std::vector<double> velocities;
        YAML::Node discrete = getNode("discrete", "velocities");
        /// @warning This operation may throw an error if there is a bad conversion. 
        std::transform(discrete.begin(), discrete.end(), std::back_inserter(velocities), [](const YAML::Node& n){return n.as<double>();});
        return std::make_shared<DiscreteLead>(velocities);
    } else if (leadtype == "constant") {
        // double v = ParseField<YAML::Node>(leadNode, "constant").and_then([this](auto& n){return ParseField<double>(n, "velocity");}).value_or(30);
        double v = ParseField<YAML::Node>(leadNode, "constant")
                                        .and_then([this](const YAML::Node& n){return ParseField<double>(n, "velocity");})
                                        .value_or(30.0);
        return std::make_shared<ConstantLead>(v);
    } else {
        return std::unexpected("Invalid leadType value. Valid leadType values are [\"function\", \"discrete\", and \"constant\"]");
    }
}

// Put all the parsing together
std::expected<SimulatorInputs, std::string> Parser::parse() {

    std::expected<void, std::string> result = parseGeneral().and_then([this](){return parseCarFactory();})
                                                            .and_then([this](){return parseLane();});
    if (result.has_value()){
        return SimulatorInputs{logger_, lane_, totaltime_, dt_};
    } else {
        return std::unexpected(result.error());
    }
}

// Discrete Parser specific functions

std::expected<void, std::string> DiscreteParser::parseLane(){

    std::expected<YAML::Node, std::string> carsNode = ParseField<YAML::Node>(cfg_, "cars");
    if (!carsNode){ return std::unexpected("Must have field \"cars\"");}
    // Get all the cars in a list/vector. Sort by x coordinate in reverse. 
    std::vector<Car> cars;
    for (YAML::Node n : *carsNode){
        try {
            double x0 = n["x0"].as<double>();
            double v0 = n["v0"].as<double>();
            double vdes = n["vdes"].as<double>();
            cars.push_back(factory_->makeCar(x0, v0, vdes, 0));
        } catch(const std::exception& e) { // any exception here is a failure condition. 
            return std::unexpected(std::format("Error parsing data about a car: {}", e.what()));
        }
        

    }
    // Gets the REVERSE sort so the most forward car is at the front
    std::ranges::sort(cars, [](const Car& c1, const Car& c2)->bool {return c1.getPosition() > c2.getPosition();});
    
    // Set the lead strategy 
    std::expected<std::shared_ptr<LeadStrategy>, std::string> leadStrat = ParseField<YAML::Node>(cfg_, "leadCar")
                                                                         .and_then([this](auto n){return parseLeadStrategy(n);});

    // if no lead strategy, return unexpected
    if (!leadStrat){ return std::unexpected(leadStrat.error()); }
    cars[0].setLeadStrategy(*leadStrat);
    
    // Add to lane
    for (Car& c : cars){
        lane_.addCar(c);
    }
    return {};
}

std::expected<void, std::string> ContinuousParser::parseLane(){
    // YAML::Node lanenode = cfg_["lane"];
    std::expected<std::shared_ptr<LeadStrategy>, std::string> leadStrat = parseLeadStrategy(cfg_["leadCar"]);
    
    // If the flow is specified in the lane, parse and use that. Breaks down in single lane case
    if (cfg_["flow"]){
        std::function<void(FlowGenerator)> setupLane = [this](FlowGenerator f){
            f.setFactory(factory_);
            lane_.setFlowGen(f);
        };
        std::expected<void, std::string> flow = parseFlow(cfg_["flow"]).transform(setupLane);
    }
    // Handle start and ending point of lane. (End defaults to infinity)
    double start = ParseField<double>(cfg_, "start").value_or(0);
    lane_.setEnd(ParseField<double>(cfg_, "end").value_or(std::numeric_limits<double>::infinity()));
 
    // Lead Car:
    double v0 = cfg_["leadCar"]["v0"].as<double>();
    double vdes = cfg_["leadCar"]["vdes"].as<double>();
    Car c = factory_->makeCar(start, v0, vdes, 0);
    lane_.addCar(c);
    return {};
}

std::expected<FlowGenerator, std::string> ContinuousParser::parseFlow(YAML::Node flowNode){
    double rate = ParseField<double>(flowNode, "rate").value_or(100); // 100 veh/hr default
    double v0 = ParseField<double>(flowNode, "v0").value_or(30); // 30 m/s default
    double vdes = ParseField<double>(flowNode, "vdes").value_or(30); // 30 m/s desired default
    double x0 = ParseField<double>(flowNode, "x0").value_or(0);

    if (rate < 0 or v0 < 0 or vdes < 0){
        return std::unexpected("Flow rate, initial velocity and desired velocity must be positive");
    }
    return FlowGenerator(rate, x0, v0, vdes, seed_);

}