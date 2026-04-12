/**
 * @file carFactory.cpp
 * @author your name (you@domain.com)
 * @brief Implements the Car Factory interface for Gipps and IDM models
 * @version 0.1
 * @date 2025-07-06
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "sim/carFactory.hpp"
#include "sim/strategy.hpp"
#include  "sim/logger.hpp"

#include <memory>

CarFactory::CarFactory(double politeness, double p_stdev):
    carid_{0}, politeness_dist_{politeness, p_stdev}{}

GippsCarFactory::GippsCarFactory(double a, double b, double bmax, double p,
    double a_stdev, double b_stdev, double bmax_stdev, double p_stdev):
    CarFactory(p , p_stdev), rng_{std::random_device{}()},
    a_dist{a, a_stdev}, b_dist{b, b_stdev}, bmax_dist{bmax, bmax_stdev} {}

Car GippsCarFactory::makeCar(double x0, double v0, double vdes, double t0) {
    double a = a_dist(rng_);
    double b = b_dist(rng_);
    double bmax = bmax_dist(rng_);
    double p = std::clamp<double>(politeness_dist_(rng_),0, 1);
    auto follow = std::make_shared<Gipps>(a, b, bmax, vdes);
    return Car(carid_++, x0, v0, t0, p, follow);
}

IDMCarFactory::IDMCarFactory(double a, double b, double s0, double p,
    double a_stdev, double b_stdev, double s0_stdev, double p_stdev):
    CarFactory(p, p_stdev), rng_{std::random_device{}()}, 
    a_dist{a, a_stdev}, b_dist{b, b_stdev}, s0_dist{s0, s0_stdev} {}

Car IDMCarFactory::makeCar(double x0, double v0, double vdes, double t0) {
    double a = a_dist(rng_);
    double b = b_dist(rng_);
    double s0 = s0_dist(rng_);
    double p = std::clamp<double>(politeness_dist_(rng_),0, 1);

    auto follow = std::make_shared<Intelligent>(a, b, s0, vdes);
    return Car(carid_++, x0, v0, t0, p, follow);
}
    