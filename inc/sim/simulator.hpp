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

    std::shared_ptr<CarLogger> logger_;   ///< Logger that writes car data to its sink
    std::shared_ptr<Highway> highway_;    ///< Highway with all lanes set up. 
    double totalTime_;                    ///< Total time that the simulation will run for
    double dt_;                           ///< Delta timestep.
    int thinning_;                        ///< Frequency of logging car snapshots. 


    const size_t maxMemory_ = 1024 * 1024; ///< Set the max memory the simulation can hold at 1 MB
    CommunicationsManager comms_; ///< Communications handler for sending data to the logger

    // Store for logs. 
    std::vector<CarSnapshot> snapshots_; ///< Simulations internally stored snapshots of recent timesteps
    std::vector<CarData> cars_; ///< Simulations internally stored car metadata of recently generated cars
    size_t maxSnapshots_; ///< Maximum number of snapshots that can be stored before sending it to the logger
    size_t maxCars_; ///< Maximum number of car metadata structs that can be stored before logging

    /**
     * @brief Runs the main simulator loop to run all timestamps of the simulation
     * 
     * @return std::expected<void, std::string> Nothing on success, error string on error
     */
    std::expected<void, std::string> mainLoop();

    /**
     * @brief Convenience function for transforming and combining error messages
     * 
     * @param prefix Prefix to append to the end of the simulation error string. 
     * @return std::function<std::string(std::string)> Function to pass to std::expected::or_else() 
     * to add this prefix to the error message
     */
    static std::function<std::string(std::string)> errorFunc(std::string prefix);

public:
    /**
     * @brief Construct a new Simulator object with specified inputs
     * 
     * @param input 
     */
    Simulator(SimulatorInputs input);
    ~Simulator() =default;

    /**
     * @brief Runs the simulator
     * 
     * @return std::expected<void, std::string> Nothing on success, string on error
     */
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


