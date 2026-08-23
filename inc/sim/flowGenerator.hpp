#pragma once

#include <memory>
#include <optional>
#include <random>

#include "carFactory.hpp"
#include "car.hpp"
#include "random.hpp"

class FlowGenerator
{
private:

    /// @brief Car Factory to make the cars
    std::shared_ptr<CarFactory> factory_;

    /// @brief Randomness generator
    std::shared_ptr<std::mt19937> rng_;
    RandomGenerator::ptr dist_;
    RandomGenerator::ptr v0Dist_;
    RandomGenerator::ptr vDesDist_;

    /// @brief Flows left in the hour
    double flowsLeft_;

    /// @brief Timesteps left in the hour
    double timestepsLeft_;

    /// @brief Rate of vechicle inflow. Units of vehicles/hr
    /// @todo This will be a function of time
    double rate_; 

    // Car Data
    double x0_;

    // Simulation Timestep
    double dt_;

    double time_{0.0};

public:

    /**
     * @brief Construct a new Flow Generator object with no flow
     * 
     */
    FlowGenerator();

    /**
     * @brief Construct a new Flow Generator object with a specified Rate
     * 
     * @param rate Approximate Number of cars that we be generated per hour by this flow generator
     * @param x0 Initial position of each car generated. This is the same for each car
     * @param v0 Maximum initial velocity of a car generated. This is the mean initial velocity unless there is a car nearby
     * @param vdes Mean Desired velocity of each car.
     * @param v0_stdev Standard deviation of intiial velocities. 
     * @param vdes_stdev Standard deviation of desired velocitie
     * @param rng Shared pointer to rng shared between multiple lanes. Ensures different lanes generate cars at different times.
     */


    /**
     * @brief Construct a new Flow Generator with a specified rate. 
     * 
     * @param rate Approximate Number of cars that we be generated per hour by this flow generator
     * @param x0 Initial position of each car generated. This is the same for each car
     * @param factory Car factory 
     * @param dt 
     * @param rng 
     */
    FlowGenerator(double rate, double x0, std::shared_ptr<CarFactory> factory,  double dt, std::shared_ptr<std::mt19937> rng);
    ~FlowGenerator() = default;

    void setRng(RandomGenerator::ptr v0Dist, RandomGenerator::ptr vDesDist, RandomGenerator::ptr mainDist);

    double position() const;

    /**
     * @brief Probabalistically generates flow 
     * @param rearPosition x value of the "back bumper" of the car in front. Will not generate if this is less than x0
     * @param vlead Velocity of the leading car. Used if the car is too close
     * @return std::optional<Car> Optionally generates a car. 
     */
    std::optional<Car> generateFlow(double rearPosition, double vlead);
    std::optional<Car> generateFlow();

};

