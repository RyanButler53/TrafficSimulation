/**
 * @file random.cpp
 * @author Ryan Butler
 * @brief Implements various class relating to rng
 * @version 0.1
 * @date 2026-11-22
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "sim/random.hpp"

RngEngine::result_type RngEngine::operator()(){
    ++draws;
    return eng();
}

void RngEngine::discard(size_t n){
    eng.discard(n);
    draws += n;
}

// Uniform Distribution

UniformDistribution::UniformDistribution(double min, double max):
dist_{min, max}{}

UniformDistribution::UniformDistribution(std::string state){
std::stringstream ss;
ss << state;
ss >> dist_;
}

double UniformDistribution::getValue(RngEngine& rng) {
return dist_(rng);
}

std::string UniformDistribution::getState() {
    std::stringstream ss;
    ss << dist_;
    return ss.str();
}

// Normal Distribution
NormalDistribution::NormalDistribution(double mean, double stdev) {
    if (stdev == 0.0){
        dist_ = mean;
    } else {
        dist_ = std::normal_distribution<double>(mean, stdev);
    }
}

NormalDistribution::NormalDistribution(std::string state){
    if (state.find(' ') != std::string::npos){
        // Confirmed that this contains the distribution not a mean
        dist_.emplace<Dist_t>();
        std::stringstream ss;
        ss << state;
        ss >> std::get<Dist_t>(dist_);
    } else {
        double mean = 0;
        std::string_view sv(state.begin(), state.end());
        std::from_chars(sv.begin(), sv.begin() + sv.size(), mean);
        dist_.emplace<double>(mean);
    }
    
}

double NormalDistribution::getValue(RngEngine& rng) {
    if (std::holds_alternative<double>(dist_)){
        return std::get<double>(dist_);
    } else {
        auto& dist = std::get<Dist_t>(dist_);
        return dist(rng);
    }
}

std::string NormalDistribution::getState() {
    if (std::holds_alternative<double>(dist_)){
        return std::to_string(std::get<double>(dist_));
    } else {
        std::stringstream ss;
        ss << std::get<Dist_t>(dist_);
        return ss.str();
    }
}