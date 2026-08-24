#include "sim/flowGenerator.hpp"
#include <ranges>

FlowGenerator::FlowGenerator():rate_{0}{}

FlowGenerator::FlowGenerator(double rate, double x0, std::shared_ptr<CarFactory> factory, double dt, std::shared_ptr<std::mt19937> rng):
    rate_{rate}, rng_{rng},dist_{nullptr}, v0Dist_{nullptr}, vDesDist_{nullptr},
    x0_{x0}, factory_{factory}, dt_{dt}, flowsLeft_{rate}, timestepsLeft_{3600.0/dt}
{}

double FlowGenerator::position() const {return x0_;}

void FlowGenerator::setRng(RandomGenerator::ptr v0Dist, RandomGenerator::ptr vDesDist, RandomGenerator::ptr mainDist){
    v0Dist_ = v0Dist;
    vDesDist_ = vDesDist;
    dist_ = mainDist;
}

// Generate flow when there is no car in front
std::optional<Car> FlowGenerator::generateFlow(){
    return generateFlow(std::numeric_limits<double>::max(), 500);
}

std::optional<Car> FlowGenerator::generateFlow(double rearPosition, double vlead){

    std::optional<Car> c = std::nullopt;

    double prob = flowsLeft_/timestepsLeft_;
    
    if ((dist_->getValue(*rng_) < prob) && rearPosition > x0_){
        --flowsLeft_;
        // If another car is nearby (within 5 seconds, adjust v0 to be the speed if the lead car)
        double v0 = v0Dist_->getValue(*rng_);
        if (rearPosition < x0_ + 5 * v0){
            v0 = std::min(v0, vlead);
        }
        double vDes = vDesDist_->getValue(*rng_);
        c = std::make_optional<Car>(factory_->makeCar(x0_, v0, vDes, time_));
        flowsLeft_ = std::clamp<double>(flowsLeft_, 0.0, rate_);
    }
    timestepsLeft_ -= 1;

    // Reset at the beginning of each hour. 
    if (timestepsLeft_ <= 0){
        timestepsLeft_ = 3600.0/dt_;
        flowsLeft_ = rate_;
    }
    time_ += dt_;

    return c;
}
