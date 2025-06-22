/**
 * @file lane.hpp
 */
#pragma once

#include "car.hpp"
#include <list>
/**
 * @class Lane Class. Represents a lane of cars. 
 */
class Lane
{
private:
    
    /**
     * @brief List of cars. Leading car is the end of the list
     */
    std::list<Car> cars_;
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

