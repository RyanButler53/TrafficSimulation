#pragma once

#include <memory>
#include <optional>
#include <random>

#include "carFactory.hpp"
#include "car.hpp"

class FlowGenerator
{
private:

    /// @brief Car Factory to make the cars
    std::shared_ptr<CarFactory> factory_;

    /// @brief Randomness generator
    std::shared_ptr<std::mt19937> rng_;

    std::uniform_real_distribution<double> dist_;

    /// @brief Flows left in the hour
    double flowsLeft_;

    /// @brief Timesteps left in the hour
    double timestepsLeft_;

    /// @brief Rate of vechicle inflow. Units of vehicles/hr
    /// @todo This will be a function of time
    double rate_; 

    // Car Data
    double x0_;
    double v0_;
    double vdes_;

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
     * @param rate 
     * @param x0 
     * @param v0 
     * @param vdes 
     * @param seed 
     */
    FlowGenerator(double rate, double x0, double v0, double vdes, std::shared_ptr<CarFactory> factory,  double dt, std::shared_ptr<std::mt19937> rng);
    ~FlowGenerator() = default;

    double position() const;

    /**
     * @brief Probabalistically generates flow 
     * @param dt timestep (seconds) to possibly generate a car. 
     * @param lastcar x value of the "back bumper" of the car in front. Will not generate if this is less than x0
     * @return std::optional<Car> Optionally generates a car. 
     */
    std::optional<Car> generateFlow(double dt, double rearPosition, double vlead);
    std::optional<Car> generateFlow(double dt);

};

