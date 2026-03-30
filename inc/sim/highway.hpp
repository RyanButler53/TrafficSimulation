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
#include <expected>
#include <set>

#include "car.hpp"
#include "flowGenerator.hpp"

class Highway {


    public: 

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

    // TODO make these all configurable
    const double delta_a = 0.1;
    const double a_bias = 0.3;
    size_t nLanes_;
    std::shared_ptr<std::set<Car>[]> lanes_;
    

    public: 

    CpuHighway(size_t numLanes);
    std::expected<void, std::string> update(double dt);
};

#ifdef TRAFFIC_WITH_KOKKOS
// class KokkosHighway : public Highway {

// };
#endif

#ifdef TRAFFIC_WITH_METAL
// class MetalHighway : public Highway{

// };
#endif
