#include "sim/flowGenerator.hpp"
#include <ranges>
#include <print>

FlowGenerator::FlowGenerator():rate_{0}{}

FlowGenerator::FlowGenerator(double rate, double x0, std::shared_ptr<CarFactory> factory, double dt, std::shared_ptr<RngEngine> rng):
    rate_{rate}, rng_{rng},dist_{nullptr}, v0Dist_{nullptr}, vDesDist_{nullptr},
    x0_{x0}, factory_{factory}, dt_{dt}, flowsLeft_{rate}, timestepsLeft_{3600.0/dt}
{}

double FlowGenerator::position() const {return x0_;}

double FlowGenerator::rate() const {return rate_;}

// TODO: This should live in the constructor
void FlowGenerator::setRng(RandomGenerator::ptr v0Dist, RandomGenerator::ptr vDesDist, RandomGenerator::ptr mainDist){
    v0Dist_ = v0Dist;
    vDesDist_ = vDesDist;
    dist_ = mainDist;
}

std::optional<Car> FlowGenerator::generateFlow(){
    return generateFlow(std::numeric_limits<double>::max(), 500);
}

double FlowGenerator::rollRng(const RandomGenerator::ptr& dist){
    return dist->getValue(*rng_);
}

void FlowGenerator::restoreState(const FlowGeneratorState& state){
    rng_->discard(state.rngState_ - rng_->getState());
    for (auto _ : std::views::iota(0, state.numGens_)){
        time_ += dt_;
    }

    dist_ = std::make_shared<UniformDistribution>(state.distState_);
    v0Dist_ = std::make_shared<NormalDistribution>(state.v0State_);
    vDesDist_ = std::make_shared<NormalDistribution>(state.vDesState_);
}

FlowGeneratorState FlowGenerator::getState() const{
    FlowGeneratorState s;
    s.distState_ = dist_->getState();
    s.v0State_ = v0Dist_->getState();
    s.vDesState_ = vDesDist_->getState();
    s.numGens_ = static_cast<int>(std::roundl(time_/dt_));
    s.rngState_ = rng_->getState();
    return s;
}

std::optional<Car> FlowGenerator::generateFlow(double rearPosition, double vlead){

    std::optional<Car> c = std::nullopt;

    double prob = flowsLeft_/timestepsLeft_;
    
    if ((rollRng(dist_) < prob) && rearPosition > x0_){
        --flowsLeft_;
        // If another car is nearby (within 5 seconds, adjust v0 to be the speed if the lead car)
        double v0 = rollRng(v0Dist_);
        if (rearPosition < x0_ + 5 * v0){
            v0 = std::min(v0, vlead);
        }
        double vDes = rollRng(vDesDist_);
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
