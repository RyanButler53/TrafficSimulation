/**
 * @file simInputs.hpp 
 * @author Ryan Buutler (rmbutler@outlook.com)
 * @brief Defines inputs for the Simulator. Passed between the parsing step and 
 * @version 0.1
 * @date 2025-07-06
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include "highway.hpp"
#include "logger.hpp"


struct SimulatorInputs {
    std::shared_ptr<CarLogger> logger_;
    std::shared_ptr<Highway> highway_;
    double totalTime_;
    double dt_;
};

