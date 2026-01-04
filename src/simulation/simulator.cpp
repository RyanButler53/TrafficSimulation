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
#include "database/databaseInit.hpp"
#include <expected>
#include <functional>

Simulator::Simulator(SimulatorInputs input): logger_{input.logger_},
    lane_{input.lane_}, totalTime_{input.totalTime_}, dt_{input.dt_}{}


std::expected<void, std::string> Simulator::mainLoop(){
    double t = 0;
    while (t < totalTime_){
        lane_.updateLane(dt_);
        t += dt_;
        
        if (lane_.done()){
            break;
        }
    }
    return {};
}

std::expected<void, std::string> Simulator::run(){

    return logger_->updateStatus("RUNNING")
                    .and_then([this]{return mainLoop();})
                    .and_then([this]{return logger_->writeData();})
                    .and_then([this]{return logger_->updateStatus("DONE");})
                    .or_else([this](std::string msg){return logger_->logFailure(msg);});
}

std::expected<void, std::string> Traffic::Simulate(std::string configfile){
    if (initDB::initDB(false)){
        return {};
    }
    ParserFactory parserFac(configfile);
    return parserFac.makeParser() // this hits a DB. 
                    .and_then(std::mem_fn(&Parser::parse))
                    .and_then([](SimulatorInputs inputs){return Simulator(inputs).run();});

}