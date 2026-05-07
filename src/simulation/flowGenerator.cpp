#include "sim/flowGenerator.hpp"

FlowGenerator::FlowGenerator():rate_{0}{}

FlowGenerator::FlowGenerator(double rate, double x0, double v0, double vdes, std::shared_ptr<CarFactory> factory, uint64_t seed):
    rate_{rate}, x0_{x0}, v0_{v0}, vdes_{vdes}, factory_{factory}, flowsLeft_{rate}
{
    if (!seed) seed = time(nullptr);
    rng_ = std::mt19937(seed);
    dist_ = std::uniform_real_distribution<double>(0,1);
    // Probability is calculated as flows left / seconds left per hour
}

std::optional<Car> FlowGenerator::generateFlow(double dt, double lastcar){

    double prob = (flowsLeft_ / secondsLeft_) * dt;
    std::optional<Car> c = std::nullopt;
    // Only generate a car if the last car's x value is greater than the start of the lane. 
    if (dist_(rng_) < prob * dt && lastcar > x0_){
        flowsLeft_ -= 1.0;
        c = std::make_optional<Car>(factory_->makeCar(x0_, v0_, vdes_, time_));
        flowsLeft_ = std::clamp<double>(flowsLeft_, 0.0, rate_);
    }
    secondsLeft_ -= dt;

    // Reset at the beginning of each hour. 
    if (secondsLeft_ <= 0){
        secondsLeft_ = 3600.0;
        flowsLeft_ = rate_;
    }
    time_ += dt;
    return c;
}
