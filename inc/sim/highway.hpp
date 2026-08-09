/**
 * @file highway.hpp
 * @author Ryan Butler (you@domain.com)
 * @brief Outlines the Highway Class. 
 * @version 0.1
 * @date 2026-03-28
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include <expected>
#include <set>
#include <unordered_map>
#include <utility>
#include <memory>

#include "car.hpp"
#include "flowGenerator.hpp"
#include "laneInfo.hpp"

struct Highway {

    virtual ~Highway(){}
    /**
     * @brief Steps the highway forward dt seconds. 
     * 
     * @param dt Timestemp
     * @return std::expected<void, std::string> Nothing on success, string on error. 
     */
    virtual std::expected<std::vector<CarData>, std::string> update(double dt) = 0;

    /**
     * @brief Converts the state of the highway at the current timestep into car snapshots
     * @details Each derived class stores cars differently and has a different conversion algorithm. 
     * @return std::vector<CarSnapshot> 
     */
    virtual void log(double t, std::vector<CarSnapshot>& data) = 0;

};

 // Each derived class of the highway owns the cars 

class CpuHighway : public Highway {

    std::vector<std::pair<size_t, FlowGenerator>> flowGenerators_;
    std::vector<std::set<Car>> lanes_;
    std::unique_ptr<LaneInfo> laneInfo_;
    size_t nLanes_;

    // TODO make these all configurable
    const double changeThreshold_ = 0.1;
    const double a_bias = 0.2;
    
    /**
     * @brief Gets a cache of acceleration values to use for lookup during lane chance calculatons
     * @details Uses each cars' update function to calculate acceleration if no lane change occurs. 
     * Assumes that the map is empty (does not clear it)
     * 
     * @param accelerationCache Vector of maps to store cached values. 
     * @param dt Timestep to calculate acceleration for 
     * @return std::optional<std::string> String error message if there is an error, nullopt otherwise. 
     */
    std::optional<std::string> getAccelerationCache(std::vector<std::unordered_map<double, double>>& accelerationCache, double dt);

    void moveVehicles(std::vector<std::unordered_map<double, double>>& accelerationCache, double dt);

    /**
     * @brief Computes the "free" road acceleration. 
     * @details Encapsulates the logic to handle the case when there is no  offical car in front of c, but is a end of a lane segment
     * 
     * @param c Car to calculate acceleration for
     * @param endOfCurrentSegment End of car's current segment (cached to avoid inteval tree queries)
     * @param ilane Lane the car is in
     * @return std::expected<double, std::string> Acceleration of the car or error message. 
     */
    std::expected<double, std::string> leadCarAcceleration(const Car& c, double endOfCurrentSegment, size_t ilane, double dt);

    public: 

    CpuHighway(size_t numLanes, std::vector<std::pair<size_t, FlowGenerator>> flows, std::unique_ptr<LaneInfo> lanes);
    std::expected<std::vector<CarData>, std::string> update(double dt) override;
    void log(double t, std::vector<CarSnapshot>& data) override;
};

// #ifdef TRAFFIC_WITH_KOKKOS
// // class KokkosHighway : public Highway {

// // };
// #endif

// #ifdef TRAFFIC_WITH_METAL
// // class MetalHighway : public Highway{

// // };
// #endif
