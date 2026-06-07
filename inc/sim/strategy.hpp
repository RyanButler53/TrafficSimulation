/**
 * @file strategy.hpp
 * @author Ryan Butler (rmbutler@outlook.com)
 * @brief Defines the interface for the Follow Strategy and 2 car following strategies
 * @version 0.2
 * @date 2025-07-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <string>
#include <functional>

 #pragma once

/**
 * @brief Concrete struct to pass into Car objects
 * 
 */
struct FollowModel {
    std::function<double(double, double, double, double)> update;
    double maxbraking;
    double a, b, c;
};

// Figure out cool way to unite this with a concept. So adding a new follow model is as easy as adding another "class"
std::function<double(double, double, double, double)> makeGippsUpdateFunc(double a, double b, double bmax, double vdes);
std::function<double(double, double, double, double)> makeIdmUpdateFunc(double a, double b, double s0, double vdes);