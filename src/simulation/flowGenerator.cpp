#include "sim/flowGenerator.hpp"
#include <ranges>
#include <print>

FlowGenerator::FlowGenerator():rate_{0}{}

FlowGenerator::FlowGenerator(double rate, double x0, double v0, double vdes, std::shared_ptr<CarFactory> factory, double dt, uint64_t seed):
    rate_{rate}, x0_{x0}, v0_{v0}, vdes_{vdes}, factory_{factory}, dt_{dt}, flowsLeft_{rate}, timestepsLeft_{3600.0/dt}
{
    if (!seed) seed = time(nullptr);
    rng_ = std::mt19937(seed);
    dist_ = std::uniform_real_distribution<double>(0,1);

}

double FlowGenerator::position() const {return x0_;}

// Generate flow when there is no car in front
std::optional<Car> FlowGenerator::generateFlow(double dt){
    return generateFlow(dt, std::numeric_limits<double>::max(), 500);
}

std::optional<Car> FlowGenerator::generateFlow(double dt, double rearPosition, double vlead){

    std::optional<Car> c = std::nullopt;

    double prob = flowsLeft_/timestepsLeft_;
    
    // Only generate if the next flow can happen outside the 2s gap
    if ((dist_(rng_) < prob) && rearPosition > x0_){
        --flowsLeft_;
        // Lead car velocity dictates maximum incoming flow speed. 
        double v0 = std::min(v0_, vlead);
        c = std::make_optional<Car>(factory_->makeCar(x0_, v0, vdes_, time_));
        flowsLeft_ = std::clamp<double>(flowsLeft_, 0.0, rate_);
    }
    timestepsLeft_ -= 1;

    // Reset at the beginning of each hour. 
    if (timestepsLeft_ <= 0){
        timestepsLeft_ = 3600.0/dt_;
        flowsLeft_ = rate_;
    }
    time_ += dt;

    return c;
}
