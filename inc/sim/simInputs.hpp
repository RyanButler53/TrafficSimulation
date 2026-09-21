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


/**
 * @brief Struct containing simulator inputs
 * @note This struct can be expanded to have other system configuration inputs and compression type
 * 
 */
struct SimulatorInputs {
    std::shared_ptr<CarLogger> logger_;   ///< Logger that writes car data to its sink
    std::shared_ptr<Highway> highway_;    ///< Highway with all lanes set up. 
    double totalTime_;                    ///< Total time that the simulation will run for
    double dt_;                           ///< Delta timestep.
    int thinning_;                        ///< Frequency of writing out car snapshots. 
    std::string jobname_;                 ///< Name of the simulation job.

};

