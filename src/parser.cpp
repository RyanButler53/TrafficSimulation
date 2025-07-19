#include "parser.hpp"
#include "simInputs.hpp"
#include <iostream>
#include <string>
#include <cmath>
#include <list>
#include <ranges>
#include <algorithm>


void Parser::parseGeneral() {
    std::string logdir;
    try
    {
        totaltime_ = cfg_["time"].as<double>();
        dt_ = cfg_["timestep"].as<double>();
        logdir = cfg_["logdir"].as<std::string>();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    // Just File Logger for now, may break into separate method for DB
    logger_ = std::make_shared<FileLogger>(logdir);
}

void Parser::parseCarFactory(){
    std::string drivertype = cfg_["driverType"].as<std::string>();
    YAML::Node driverParams = cfg_["driverParams"];

    if (drivertype == "Gipps"){
        double a = driverParams["a"].as<double>();
        double b = driverParams["b"].as<double>();
        double bmax = driverParams["bmax"].as<double>();
        factory_ = std::make_shared<GippsCarFactory>(a,b,bmax, logger_);
    } else if (drivertype == "IDM") {
        double a = driverParams["a"].as<double>();
        double b = driverParams["b"].as<double>();
        double s0 = driverParams["s0"].as<double>();  
        factory_ = std::make_shared<IDMCarFactory>(a,b,s0, logger_);
    } else {
        throw InvalidConfigError("Valid driverType values are [\"Gipps\" and \"IDM\"]");
    }
}

std::shared_ptr<LeadStrategy> Parser::parseLeadStrategy(YAML::Node leadNode){
    std::string leadtype = leadNode["leadType"].as<std::string>();
    if (leadtype == "function"){
        // looking for Sine, Constant, Piecewise
        // Do piecewise later. 
        YAML::Node sine = leadNode["function"]["sine"];
        double a = sine["a"].as<double>();
        double b = sine["b"].as<double>();
        double c = sine["c"].as<double>();
        std::function<double(double)> fn = [=](double t)->double {return a * std::sin(b*t) + c;};
        return std::make_shared<FunctionLead>(fn);
        
    } else if (leadtype == "discrete") {
        std::vector<double> velocities;
        YAML::Node discrete = cfg_["discrete"]["velocities"];
        std::transform(discrete.begin(), discrete.end(), std::back_inserter(velocities), [](const YAML::Node& n){return n.as<double>();});
        return std::make_shared<DiscreteLead>(velocities);
    } else if (leadtype == "constant") {
        double v = cfg_["constant"]["velocity"].as<double>();
        return std::make_shared<ConstantLead>(v);
    } else {
        throw std::invalid_argument("Invalid leadType value. Valid leadType values are [\"function\", \"discrete\", and \"constant\"]");
    }
}

void DiscreteParser::fillLane(Lane& lane){
    YAML::Node carsNode = cfg_["cars"];
    // Get all the cars in a list/vector. Sort by x coordinate in reverse. 
    std::vector<Car> cars;
    for (YAML::Node n : carsNode){
        double x0 = n["x0"].as<double>();
        double v0 = n["v0"].as<double>();
        double vdes = n["vdes"].as<double>();
        cars.push_back(factory_->makeCar(x0, v0, vdes));
    }
    // Gets the REVERSE sort so the most forward car is at the front
    std::ranges::sort(cars, [](const Car& c1, const Car& c2)->bool {return c1.getPosition() > c2.getPosition();});
    
    // Set the lead strategy 
    YAML::Node leadCar = cfg_["leadCar"];
    std::shared_ptr<LeadStrategy> leadStrat = parseLeadStrategy(leadCar);
    cars[0].setLeadStrategy(leadStrat);
    
    // Add to lane
    for (Car& c : cars){
        lane.addCar(c);
    }
}

DiscreteParser::DiscreteParser(YAML::Node cfg):Parser(cfg){}

// Put all the parsing together
SimulatorInputs Parser::parse() {

    parseGeneral();

    parseCarFactory();

    Lane lane;
    fillLane(lane);

    return {logger_, lane, totaltime_, dt_};
}
