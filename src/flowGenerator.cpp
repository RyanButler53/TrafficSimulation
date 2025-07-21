#include "flowGenerator.hpp"

FlowGenerator::FlowGenerator():rate_{0}{}

FlowGenerator::FlowGenerator(double rate, double x0, double v0, double vdes, uint64_t seed):
    rate_{rate}, x0_{x0}, v0_{v0}, vdes_{vdes}
{
    if (!seed) seed = time(nullptr);
    rng_ = std::mt19937(seed);
    dist_ = std::uniform_real_distribution<double>(0,3600);
}

std::optional<Car> FlowGenerator::generateFlow(double dt){
    std::optional<Car> c = std::nullopt;
    if (dist_(rng_)< rate_ * dt){
        c = std::make_optional<Car>(factory_->makeCar(x0_, v0_, vdes_, time_));
        std::cout << "Adding a car at t = " << time_ << std::endl;
    }
    time_ += dt;
    return c;
}

void FlowGenerator::setFactory(std::shared_ptr<CarFactory> factory){
    factory_ = factory;
}

