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

#include "car.hpp"
#include "flowGenerator.hpp"

struct Highway {

    virtual ~Highway(){}
    /**
     * @brief Steps the highway forward dt seconds. 
     * 
     * @param dt Timestemp
     * @return std::expected<void, std::string> Nothing on success, string on error. 
     */
    virtual std::expected<void, std::string> update(double dt) = 0;

};

 // Each derived class of the highway owns the cars 

class CpuHighway : public Highway {

    std::vector<FlowGenerator> flowGenerators_;
    std::shared_ptr<std::set<Car>[]> lanes_;
    size_t nLanes_;


    // TODO make these all configurable
    const double changeThreshold_ = 0.1;
    const double a_bias = 0.3;
    
    std::optional<std::string> getAccelerationCache(std::vector<std::unordered_map<double, double>>& accelerationCache, double dt);

    void moveVehicles(std::vector<std::unordered_map<double, double>>& accelerationCache, double dt);

    public: 

    CpuHighway(size_t numLanes, std::vector<FlowGenerator> flows);
    std::expected<void, std::string> update(double dt) override;
};

// #ifdef TRAFFIC_WITH_KOKKOS
// // class KokkosHighway : public Highway {

// // };
// #endif

// #ifdef TRAFFIC_WITH_METAL
// // class MetalHighway : public Highway{

// // };
// #endif
