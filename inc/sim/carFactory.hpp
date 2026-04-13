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
    std::normal_distribution<double> politeness_dist_;
    mutable size_t carid_;

public:
    CarFactory(double politeness, double p_stdev);
    virtual ~CarFactory() = default;
};


class GippsCarFactory : public CarFactory {
    std::mt19937 rng_;
    std::normal_distribution<double> a_dist;
    std::normal_distribution<double> b_dist;
    std::normal_distribution<double> bmax_dist;

    public: 
    GippsCarFactory(double a, double b, double bmax, double p,
                    double a_stdev , double b_stdev , double bmax_stdev, double p_stdev);

    ~GippsCarFactory() = default;

    Car<Gipps> makeCar(double x0, double v0, double vdes, double t0);
};

class IDMCarFactory : public CarFactory {

    std::mt19937 rng_;
    std::normal_distribution<double> a_dist;
    std::normal_distribution<double> b_dist;
    std::normal_distribution<double> s0_dist;

    public: 
    IDMCarFactory(double a, double b, double s0, double p,
        double a_stdev , double b_stdev , double s0_stdev, double p_stdev);
    ~IDMCarFactory() = default;

    Car<Intelligent> makeCar(double x0, double v0, double vdes, double t0);
};