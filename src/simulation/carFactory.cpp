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

CarFactory::CarFactory(double politeness, double p_stdev):
    carid_{0}, politeness_dist_{politeness, p_stdev}{}
