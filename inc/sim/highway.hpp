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
#include "shared/environment.hpp"

struct Highway {

    virtual ~Highway(){}
    /**
     * @brief Steps the highway forward dt seconds. This must be implemented for all 
     * derived classes handling their own structuring of the cars. 
     * 
     * @param dt Delta timestep. 
     * @return std::expected<void, std::string> Nothing on success, string on error. 
     */
    virtual std::expected<std::vector<CarData>, std::string> update(double dt) = 0;

    /**
     * @brief Converts the state of the highway at the current timestep into car snapshots
     * @details Each derived class stores cars differently and has a different conversion algorithm. 
     */
    virtual void log(double t, std::vector<CarSnapshot>& data) = 0;

    /**
     * @brief Returns the lane environment of the highway.
     * @return Environment struct containing lane info and flow rates
     */
    virtual Environment environment() = 0;

};

 // Each derived class of the highway owns the cars 

class CpuHighway : public Highway {

    /**
     * @brief Map storing flow generators and the lane indexes they correspond to. 
     * There can be multiple flow generators per lane for lanes that have different start/end
     * segments. 
     * 
     */
    std::vector<std::pair<size_t, FlowGenerator>> flowGenerators_;

    /**
     * @brief CPU Highway's internal representation of the cars. Each element of the vector holds a sorted std::set of cars
     * that represents all the car objects in a lane. The set can hold cars from multiple different segments. 
     * 
     */
    std::vector<std::set<Car>> lanes_;

    /**
     * @brief Holds the highways lane info struct. This should never be a nullptr.
     */
    std::unique_ptr<LaneInfo> laneInfo_;

    /**
     * @brief Number of unique lanes (and lane indexes)
     * @warning The number of lanes and the number of segments is NOT gauranteed to be the same
     * 
     */
    size_t nLanes_;

    /**
     * @brief Utility threshold for a lane change to happen. 
     * @todo This should be configureable. 
     * 
     */
    const double changeThreshold_ = 0.1;
    
    /**
     * @brief Gets a cache of acceleration values to use for lookup during lane chance calculatons
     * @details Uses each cars' update function to calculate acceleration if no lane change occurs. 
     * Assumes that the map is empty (does not clear it)
     * 
     * @param accelerationCache Vector of maps to store cached values. Each map maps the x position to the acceleration value (m/s)
     * @param dt Timestep to calculate acceleration for 
     * @return std::optional<std::string> String error message if there is an error, nullopt otherwise. 
     */
    std::optional<std::string> getAccelerationCache(std::vector<std::unordered_map<double, double>>& accelerationCache, double dt);

    /**
     * @brief Updates the positions and velocities of all cars. The lane is always unchanged. 
     * This operation is independnt for each car and can be run in parallel
     * 
     * @param accelerationCache Acceleration cache. Maps X position to acceleration value
     * @param dt Timestep to move each car by. 
     */
    void moveVehicles(std::vector<std::unordered_map<double, double>>& accelerationCache, double dt);

    /**
     * @brief Computes the "free" road acceleration. 
     * @details Encapsulates the logic to handle the case when there is no  offical car in front of c, but is a end of a lane segment
     * 
     * @param c Car to calculate acceleration for
     * @param ilane Lane the car is in
     * @param dt Timestep to calculate acceleration over
     * @return std::expected<double, std::string> Acceleration of the car or error message. 
     */
    std::expected<double, std::string> leadCarAcceleration(const Car& c, size_t ilane, double dt);

    public: 

    /**
     * @brief Construct a new Cpu Highway object with numLanes lanes, flow generators and lane info
     * 
     * @param numLanes Number of lanes in the highway. This is constant and doesn't change through out the simulation 
     * @param flows Vector of Flow generation objects. There is one flow generator per lane segment
     * @param lanes Lane info struct for the highway. See \ref LaneInfo
     */
    CpuHighway(size_t numLanes, std::vector<std::pair<size_t, FlowGenerator>> flows, std::unique_ptr<LaneInfo> lanes);
    
    std::expected<std::vector<CarData>, std::string> update(double dt) override;
    
    void log(double t, std::vector<CarSnapshot>& data) override;

    Environment environment() override;
};

// #ifdef TRAFFIC_WITH_KOKKOS
// // class KokkosHighway : public Highway {

// // };
// #endif

// #ifdef TRAFFIC_WITH_METAL
// // class MetalHighway : public Highway{

// // };
// #endif
