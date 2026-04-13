#include "sim/FlowGenerator.hpp"

Flow::Flow(double x0, double v0, double vdes, std::shared_ptr<CarFactory> factory, uint64_t seed)x0_{x0}, v0_{v0}, vdes_{vdes}, factory_{factory}
{
    if (!seed) seed = time(nullptr);
    rng_ = std::mt19937(seed);
    dist_ = std::uniform_real_distribution<double>(0,3600);
}

template<FollowModel Model>
FlowGenerator<Model>::FlowGenerator():rate_{0}{}

FlowGenerator<Model>::FlowGenerator(double x0, double v0, double vdes, std::shared_ptr<CarFactory> factory, uint64_t seed, double rate):
    rate_{rate}{}


std::optional<Car<Model>> FlowGenerator<Model>::generateFlow(double dt){
    std::optional<Car> c = std::nullopt;
    if (dist_(rng_)< rate_ * dt){
        c = std::make_optional<Car>(factory_->makeCar(x0_, v0_, vdes_, time_));
    }
    time_ += dt;
    return c;
}
