/**
 * @file simulator.cpp
  * @author Ryan Butler (rmbutler@outlook.com)
 * @brief Implements the simulator class
 * @note Will need to be expanded to handle continuous time simulations
 * @version 0.1
 * @date 2025-07-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include <iostream>
#include <vector>
#include "sim/simulator.hpp"
#include "sim/parser.hpp"
#include "sim/parserFactory.hpp"
#include <expected>


Simulator::Simulator(SimulatorInputs input): logger_{input.logger_},
    lane_{input.lane_}, totalTime_{input.totalTime_}, dt_{input.dt_}{}

std::expected<void, std::string> Simulator::run(){
    if (!logger_->init().has_value()){
        return std::unexpected("Error initalizing Logger");
    }

    double t = 0;

    while (t < totalTime_){
        lane_.updateLane(dt_);
        t += dt_;
        
        if (lane_.done()){
            break;
        }
    }
    // Fits all logs in memory.
    return logger_->writeData();
}

int Traffic::Simulate(std::string configfile){
    ParserFactory parserFac(configfile);
    std::expected<std::unique_ptr<Parser>, std::string> parser = parserFac.makeParser();
    if (!parser){ 
        std::cerr << parser.error() << std::endl;
        return 1;
    }
    std::expected<SimulatorInputs, std::string> inputs = parser.value()->parse();
    if (!inputs){
        std::cerr << inputs.error() << std::endl;
        return 1;
    }
    Simulator s(inputs.value());
    std::expected<void, std::string> result = s.run();
    if (!result){
        std::cerr << result.error();
        return 1;
    }
    return 0;
}