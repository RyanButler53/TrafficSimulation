/**
 * @file simulator.hpp 
 * @author Ryan Butler (rmbutler@outlook.com)
 * @brief Defines the Simulator Class (Interface?)
 * @version 0.1
 * @date 2025-07-01
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include "lane.hpp"
#include "simInputs.hpp"

class Simulator
{
private:

    std::shared_ptr<CarLogger> logger_;
    Lane lane_;
    double totalTime_;
    double dt_;

public:
    Simulator(SimulatorInputs input);
    ~Simulator() =default;

    void run();
};



