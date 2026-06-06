/**
 * @file logStructs.hpp
 * @author Ryan Butler
 * @brief Header for car logging structs
 * @version 0.1
 * @date 2026-04-12
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#pragma once
#include <cstddef>
#include <cstdint>

/**
 * @brief Struct containing the minimum data about each car at a given timestep
 * @details Enough information to reconstruct and visualize the simulation with ffmpeg. 
 */
struct CarSnapshot {
    std::size_t id; // car id
    double x; /// position
    double v; // velocity
    double t; // time
    uint16_t l; // lane
};


/**
 * @brief Struct containing data about a specific car
 * @details Data is specific to the car follow model and is consistent across all timesteps
 */
struct CarData {
    double a; // acceleration
    double b; // braking
    double c; // max braking in Gipps, min gap in IDM
    double p; // Lane changing politeness
    size_t id; // car id
};

/// @brief Simple struct to hold simulator stats
struct SimulationStats {
    double runtime_;
};
