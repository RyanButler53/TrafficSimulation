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
    return parserFac.makeParser()
                    .and_then([](std::unique_ptr<Parser> p){return p->parse();})
                    .and_then([](SimulatorInputs inputs){return Simulator(inputs).run();})
                    .transform_error([](std::string err){std::cerr << err << std::endl; return err;})
                    .transform([](){return 0;}).value_or(1);

}