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


Simulator::Simulator(SimulatorInputs input): logger_{input.logger_},
    lane_{input.lane_}, totalTime_{input.totalTime_}, dt_{input.dt_}{}

void Simulator::run(){
    double t = 0;

    while (t < totalTime_){
        lane_.updateLane(dt_);
        t += dt_;
        
        if (lane_.done()){
            break;
        }
    }
    // Fits all logs in memory.
    logger_->writeData();
}

int Traffic::Simulate(std::string configfile){
    ParserFactory parserFac(configfile);
    std::shared_ptr<Parser> parser;
    try {
       parser = parserFac.makeParser();
    } catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    
    SimulatorInputs inputs = parser->parse();
    Simulator s(inputs);
    s.run();
    return 0;
}