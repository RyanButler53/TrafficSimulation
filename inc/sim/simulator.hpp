/**
 * @file simulator.hpp 
 * @author Ryan Butler (rmbutler@outlook.com)
 * @brief Defines the Simulator Class (Interface?)
 * @version 0.3
 * @date 2025-07-01
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include "comms.hpp"
#include "highway.hpp"
#include "simInputs.hpp"
#include <expected>
#include <functional>

class Simulator
{
private:

    std::shared_ptr<CarLogger> logger_;
    std::shared_ptr<Highway>highway_;
    double totalTime_;
    double dt_;
    int thinning_;

    // 1 mb of max memory
    const size_t maxMemory_ = 1024 * 1024;
    CommunicationsManager comms_;

    // Store for logs. 
    std::vector<CarSnapshot> snapshots_;
    std::vector<CarData> cars_;
    size_t maxSnapshots_;
    size_t maxCars_;


    std::expected<void, std::string> mainLoop();

    // For transforming and chaining error messages
    static std::function<std::string(std::string)> errorFunc(std::string prefix);

public:
    Simulator(SimulatorInputs input);
    ~Simulator() =default;

    std::expected<void, std::string> run();
};

namespace Traffic {

    /**
     * @brief Runs the full traffic simulation based off the config ifle
     * 
     * @param configfile config yaml passed to parser and simulator
     */
    std::expected<void, std::string> Simulate(std::string configfile);
}


