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
#include <expected>
#include <functional>
#include <thread>
#include "sim/simulator.hpp"
#include "sim/parser.hpp"
#include "sim/parserFactory.hpp"


Simulator::Simulator(SimulatorInputs input): logger_{input.logger_},
    highway_{input.highway_}, totalTime_{input.totalTime_}, dt_{input.dt_}, comms_{10, CompressionType::UNCOMPRESSED}{

        // Resize vectors to have a maximum memory to stay under the limit 
        size_t snapshotMem = maxMemory_ * 0.9;
        size_t carMem = maxMemory_ * 0.1;
        snapshots_.reserve(snapshotMem);
        cars_.reserve(carMem);
        maxSnapshots_ = snapshotMem / sizeof(CarSnapshot);
    }

std::function<std::string(std::string)> Simulator::errorFunc(std::string prefix){
    return [prefix](const std::string& e){return std::format("Error {}: {}\n", prefix, e);};
}

std::expected<void, std::string> Simulator::mainLoop(){

    // Logger
    std::jthread loggingThread([this](){logger_->run(comms_);});
    auto start = std::chrono::steady_clock::now();
    double t = 0.0;
    std::expected <void, std::string> simStatus;
    while (t < totalTime_){
        simStatus = highway_->update(dt_).transform([this](const auto& cdata){
            for (const auto& car : cdata){cars_.push_back(car);}
        });
        t += dt_;
        highway_->log(t, snapshots_);
        if (!simStatus.has_value()){
            break;
        }

        // At 90% memory capacity, send to the logging thread
        if (snapshots_.size() > 0.9 * maxSnapshots_){
            comms_.send(std::move(cars_));
            cars_.clear();
            comms_.send(std::move(snapshots_));
            snapshots_.clear();
        }
    }
    comms_.send(std::move(cars_));
    comms_.send(std::move(snapshots_));
    comms_.endOfData();

    simStatus = simStatus.transform_error(Simulator::errorFunc("simulating error"));
    auto end = std::chrono::steady_clock::now();
    long ms = std::chrono::duration_cast<std::chrono::microseconds>((end - start)).count();

    SimulationStats stats{double(ms / 1000000.0)};
    auto statsStatus = logger_->writeStats(stats).transform_error(Simulator::errorFunc("writing stats"));
    std::string errmsg  = simStatus.error_or("") + statsStatus.error_or("");
    return (errmsg.empty()) ? std::expected<void, std::string>{} : std::unexpected(errmsg);
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