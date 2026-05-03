/**
 * @file strategy.cpp
 * @author Ryan Butler
 * @brief Implements the Car Following strategies Gipps and Intelligent
 * @version 0.1
 * @date 2025-07-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "sim/strategy.hpp"

#include <algorithm>
#include <cmath>
#include <format>
#include <iostream>

std::function<double(double, double, double, double)> makeGippsUpdateFunc(double a, double b, double bmax, double vdes){
    auto function = [a, b, bmax, vdes](double v, double vlead, double gap, double dt){
        double ratio = v/vdes;
        double bt = b * dt;
        
        // free road velocity
        double free_road = v + 2.5 * a * dt * (1 - ratio) * std::sqrt(0.025 +ratio);
    
        // Braking velocity
        double breaking = bt + std::sqrt((bt * bt) - b *((2 * gap) - v*dt - (vlead * vlead / bmax)));
        double result = std::min(free_road, breaking);
        if (result < 0.0){
            std::cout << "Negative velocity!" << std::endl;
        }
        return result;
    };
    return function;
}

std::function<double(double, double, double, double)> makeIdmUpdateFunc(double a, double b, double s0, double vdes){

    auto function = [a, b, s0, vdes](double v, double vlead, double gap, double dt){
            // Convienence Variables
        double dv = v - vlead;
        double ratio = v/vdes;

        // Compute Velocity
        double sDes = s0 + std::max(0.0, v*dt + (v*dv)/(2*std::sqrt(2*a*b)));
        double final_accel = a* (1 - std::pow(ratio, 4) - std::pow(sDes/gap,2));
        return v + final_accel * dt;
    };
    return function;
}