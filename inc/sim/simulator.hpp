/**
 * @file simulator.hpp 
 * @author Ryan Butler (rmbutler@outlook.com)
 * @brief Defines the Simulator Class (Interface?)
 * @version 0.2
 * @date 2025-07-01
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include "lane.hpp"
#include "simInputs.hpp"
#include <expected>

class Simulator
{
private:

    std::shared_ptr<CarLogger> logger_;
    Lane lane_;
    double totalTime_;
    double dt_;

    std::expected<void, std::string> mainLoop();
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


