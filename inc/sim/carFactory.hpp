/**
 * @file carFactory.hpp
 * @author Ryan Buutler (rmbutler@outlook.com)
 * @brief Defines the interface for a car factory
 * @version 0.1
 * @date 2025-07-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include <memory>
#include <random>
#include "car.hpp"

class CarFactory {

protected: 
    std::shared_ptr<CarLogger> logger_;
    mutable size_t carid_;

public:
    CarFactory(std::shared_ptr<CarLogger> logger);
    virtual ~CarFactory() = default;

    virtual Car makeCar(double x0, double v0, double vdes, double t0 = 0) = 0;
};

class GippsCarFactory : public CarFactory {
    std::mt19937 rng_;
    std::normal_distribution<double> a_dist;
    std::normal_distribution<double> b_dist;
    std::normal_distribution<double> bmax_dist;

    public: 
    GippsCarFactory(double a, double b, double bmax,
                    double a_stdev , double b_stdev , double bmax_stdev,
                    std::shared_ptr<CarLogger> logger);

    ~GippsCarFactory() = default;

    Car makeCar(double x0, double v0, double vdes, double t0) override;
};

class IDMCarFactory : public CarFactory {

    std::mt19937 rng_;
    std::normal_distribution<double> a_dist;
    std::normal_distribution<double> b_dist;
    std::normal_distribution<double> s0_dist;

    public: 
    IDMCarFactory(double a, double b, double s0, 
        double a_stdev , double b_stdev , double s0_stdev, 
        std::shared_ptr<CarLogger> logger);
    ~IDMCarFactory() = default;

    Car makeCar(double x0, double v0, double vdes, double t0) override;
};