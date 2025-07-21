/**
 * @file lane.hpp
 */
#pragma once

#include <list>
#include <numeric>

#include "car.hpp"
#include "flowGenerator.hpp"

/**
 * @class Lane Class. Represents a lane of cars. 
 * @warning Leading car is at the END of the list;
 */
class Lane
{
private:
    
    /// @brief List of cars. Leading car is the end of the list
    std::list<Car> cars_;

    /// @brief Flow Generator. Defaults to flow generator with a rate of 0
    FlowGenerator flowgen_;

    /// @brief End of a lane. Cars must MERGE by this x value. Defaults to infinity. 
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

    /// @brief Adds a car to the BACK of the lane
    void addCar(Car& c);

    void addFlow(double dt);

    /// @brief Sets the flow gen
    /// @param fgen Flow generarot
    void setFlowGen(FlowGenerator& fgen);

    /// @brief sets the end of the lane. 
    void setEnd(double xf) {end_ = xf;};

    /**
     * @brief checks if the lane has cars left or has flow
     * 
     * @return true if there are cars in the lane or there is flow
     * @return false if there are no cars  in the lane and no flow
     */
    bool done() const;

};

