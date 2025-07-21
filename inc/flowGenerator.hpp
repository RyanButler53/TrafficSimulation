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
    std::mt19937 rng_;

    std::uniform_real_distribution<double> dist_;

    /// @brief Rate of vechicle inflow. Units of vehicles/hr
    double rate_; 

    // Car Data
    double x0_;
    double v0_;
    double vdes_;

    double time_{0};

public:
    FlowGenerator(); // Defaults to no flow at all. Rate of 0
    FlowGenerator(double rate, double x0, double v0, double vdes, uint64_t seed = 0);
    ~FlowGenerator() = default;

    /**
     * @brief Probabalistically generates flow 
     * @param dt timestep (seconds) to possibly generate a car. 
     * @return std::optional<Car> Optionally generates a car. 
     */
    std::optional<Car> generateFlow(double dt);

    /**
     * @brief Checks if the flow is nonzero. 
     * 
     * @return true if there is nonzero flow. 
     * @return false No flow
     */
    bool hasFlow() const {return rate_ != 0;}

    void setFactory(std::shared_ptr<CarFactory> factory);
};

