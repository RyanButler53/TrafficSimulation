#include "sim/flowGenerator.hpp"
#include <ranges>

FlowGenerator::FlowGenerator():rate_{0}{}

FlowGenerator::FlowGenerator(double rate, double x0, double v0, double vdes, std::shared_ptr<CarFactory> factory, double dt, uint64_t seed):
    rate_{rate}, x0_{x0}, v0_{v0}, vdes_{vdes}, factory_{factory}, dt_{dt}, totalTimesteps_{3600.0/dt}
{
    if (!seed) seed = time(nullptr);
    rng_ = std::mt19937(seed);
    dist_ = std::uniform_real_distribution<double>(0,1);

}

std::optional<Car> FlowGenerator::generateFlow(double dt){

    std::optional<Car> c = std::nullopt;

    double prob = rate_/(totalTimesteps_ - rate_ * (TIME_GAP - dt));
    
    // Only generate if the next flow can happen outside the 2s gap
    if ((time_ >= nextGeneration_) && (dist_(rng_) < prob)){
        c = std::make_optional<Car>(factory_->makeCar(x0_, v0_, vdes_, time_));
        nextGeneration_ = time_ + TIME_GAP;
    }

    time_ += dt;
    return c;
}
