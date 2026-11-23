/**
 * @file checkpoint.hpp
 * @author Ryan Butler
 * @brief Defines the data for a simulation checkpoint
 * @version 0.1
 * @date 2026-11-22
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once

#include "shared/reflectionUtil.hpp"
#include <string>
#include <vector>

/**
 * @brief Struct encapsulating the state of the FlowGenerator. 
 * 
 */
struct FlowGeneratorState {
    uint64_t rngState_;
    std::string distState_;
    std::string v0State_;
    std::string vDesState_;
    int numGens_; // Used to tick the time forward

    friend bool operator==(const FlowGeneratorState& f1, const FlowGeneratorState& f2) {
        return equality::operatorEquals(f1, f2);
    }
};

/**
 * @brief Struct representing the entire state of the CarFactory. 
 * 
 */
struct CarFactoryState {
    std::string a_dist_;
    std::string b_dist_;
    std::string c_dist_;
    std::string p_dist_;
    size_t nextCarId_;
    uint64_t rng_;

    friend bool operator==(const CarFactoryState& f1, const CarFactoryState& f2) {
        return equality::operatorEquals(f1, f2);
    }
};

/**
 * @brief Represents a snapshot of time of the entire state of the components of the system (not cars)
 * @details Contains data required to restore the state of rng in Car Factory and RNG 
 */
struct Checkpoint {
    double timestamp_; ///< Timestamp the checkpoint represents.
    
    CarFactoryState factoryState_;  ///< State of the Car Factory 
    std::vector<std::pair<size_t, FlowGeneratorState>> flowStates_; /// State of the flow Generator

};
