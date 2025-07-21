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
#include "carFactory.hpp"
#include "strategy.hpp"

#include <memory>

CarFactory::CarFactory(std::shared_ptr<CarLogger> logger):logger_{logger}{}

GippsCarFactory::GippsCarFactory(double a, double b, double bmax, std::shared_ptr<CarLogger> logger):
    CarFactory(logger), a_{a}, b_{b}, bmax_{bmax}{}


Car GippsCarFactory::makeCar(double x0, double v0, double vdes, double t0) const {
    auto  follow = std::make_shared<Gipps>(a_, b_, bmax_, vdes);
    return Car(x0, v0, t0, logger_, follow);
}

IDMCarFactory::IDMCarFactory(double a, double b, double s0, std::shared_ptr<CarLogger> logger):
    CarFactory(logger), a_{a}, b_{b}, s0_{s0}{}

Car IDMCarFactory::makeCar(double x0, double v0, double vdes, double t0) const {
    auto  follow = std::make_shared<Intelligent>(a_, b_, s0_, vdes);
    return Car(x0, v0, t0, logger_, follow);
}
    