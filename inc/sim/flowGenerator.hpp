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

    /**
     * @brief Sets random distributions for initial velocity, desired velocity and uniform main distribution
     * 
     * @param v0Dist Initial velocity distribution
     * @param vDesDist Desired velocity distribution
     * @param mainDist Uniform 0-1 distribution. Parameterized for mocking
     */
    void setRng(RandomGenerator::ptr v0Dist, RandomGenerator::ptr vDesDist, RandomGenerator::ptr mainDist);

    /**
     * @brief Returns the position that cars are generated at
     * 
     * @return double x position in meters
     */
    double position() const;

    /**
     * @brief Returns the flow rate for this current flow generator
     * 
     * @return double rate in vehicles per hour. 
     */
    double rate() const;
    
    /**
     * @brief Probabalistically generates flow 
     * @param rearPosition x value of the "back bumper" of the car in front. Will not generate if this is less than x0
     * @param vlead Velocity of the leading car. Used if the car is too close
     * @return std::optional<Car> Car generated. Nullopt if no car is generated
     */
    std::optional<Car> generateFlow(double rearPosition, double vlead);
    std::optional<Car> generateFlow();

};

