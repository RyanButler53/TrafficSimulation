/**
 * @file lane.hpp
 */
#pragma once

#include "car.hpp"
#include <list>
#include <numeric>
/**
 * @class Lane Class. Represents a lane of cars. 
 * @warning Leading car is at the END of the list;
 */
class Lane
{
private:
    
    /**
     * @brief List of cars. Leading car is the end of the list
     */
    std::list<Car> cars_;

    /**
     * @brief Start of the lane. Typically x = 0
     * 
     */
    double start_;

    /**
     * @brief End of a lane. 
     * @todo Determine how to do differentiate this between "must merge" and "disappear after"
     */
    double end_ = std::numeric_limits<double>::infinity();
public:

    // Cars should be added in order. 
    Lane() = default;
    Lane(std::list<Car>& cars);
    ~Lane()= default;


    /**
     * @brief Updates a line by timestep dt
     * @param dt Timestep
     */
    void updateLane(double dt);

    void addCar(Car& c);

};

