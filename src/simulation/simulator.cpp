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
    highway_{input.highway_}, totalTime_{input.totalTime_}, dt_{input.dt_}{}

std::function<std::string(std::string)> Simulator::errorFunc(std::string prefix){
    return [prefix](const std::string& e){return std::format("Error {}: {},", prefix, e);};
}

std::expected<void, std::string> Simulator::mainLoop(){
    auto start = std::chrono::steady_clock::now();
    double t = 0;
    std::expected <void, std::string> simStatus;
    while (t < totalTime_){
        simStatus = highway_->update(dt_).transform([this](const auto& cdata){
            for (const auto& car : cdata){ logger_->addCar(car); }
        });
        t += dt_;
        logger_->fromHighway(highway_->log(t));
        if (!simStatus.has_value()){
            break;
        }
    }

    simStatus = simStatus.transform_error(Simulator::errorFunc("simulating error"));
    auto logStatus = logger_->writeData().transform_error(Simulator::errorFunc("writing data"));
    auto end = std::chrono::steady_clock::now();
    long ms = std::chrono::duration_cast<std::chrono::microseconds>((end - start)).count();

    SimulationStats stats{double(ms / 1000000.0)};
    auto statsStatus = logger_->writeStats(stats).transform_error(Simulator::errorFunc("writing stats"));

    std::string errmsg  = simStatus.error_or("") + logStatus.error_or("") + statsStatus.error_or("");
    return (!errmsg.empty()) ? std::expected<void, std::string>{} : std::unexpected(errmsg);
}

std::expected<void, std::string> Simulator::run(){

    return logger_->updateStatus("RUNNING")
                    .and_then([this]{return mainLoop();})
                    .and_then([this]{return logger_->updateStatus("DONE");})
                    .or_else([this](std::string msg){return logger_->logFailure(msg);});
}

std::expected<void, std::string> Traffic::Simulate(std::string configfile){

    ParserFactory parserFac(configfile);
    return parserFac.makeParser()
                    .and_then(std::mem_fn(&Parser::parse)) // this hits a DB. 
                    .and_then([](SimulatorInputs inputs){return Simulator(inputs).run();});

}