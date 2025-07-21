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

#include "car.hpp"

class CarFactory {

protected: 
    std::shared_ptr<CarLogger> logger_;

public:
    CarFactory(std::shared_ptr<CarLogger> logger);
    virtual ~CarFactory() = default;

    virtual Car makeCar(double x0, double v0, double vdes, double t0 = 0) const = 0;
};

class GippsCarFactory : public CarFactory {
    double a_;
    double b_;
    double bmax_;
    

    public: 
    GippsCarFactory(double a, double b, double bmax, std::shared_ptr<CarLogger> logger);
    ~GippsCarFactory() = default;

    Car makeCar(double x0, double v0, double vdes, double t0) const override;
};

class IDMCarFactory : public CarFactory {

    double a_;
    double b_;
    double s0_;

    public: 
    IDMCarFactory(double a, double b, double s0, std::shared_ptr<CarLogger> logger);
    ~IDMCarFactory() = default;

    Car makeCar(double x0, double v0, double vdes, double t0) const override;
};