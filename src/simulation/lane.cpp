/**
 * @file lane.cpp
 * @author Ryan Butler (rmbutler@outlook.com)
 * @brief Implements the lane class. Only works for a single lane
 * @version 0.1
 * @date 2025-07-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "sim/lane.hpp"

Lane::Lane(std::list<Car>& cars):cars_{std::move(cars)}{}

std::expected<void, std::string> Lane::updateLane(double dt){
    if (cars_.empty()){
        std::cout << "Empty lane" << std::endl;
        addFlow(dt);
        return {};
    }
    
    std::list<Car>::iterator current = cars_.begin();
    std::list<Car>::iterator next = ++cars_.begin();
    while (next != cars_.end()){
        Car& lead = *next;
        std::optional<std::string> status = current->step(lead, dt);
        if (status) {return std::unexpected(*status);}
        ++current;
        ++next;
    }

    // current points to the lead car
    current->step(dt);

    addFlow(dt);

    // Remove cars past the end
    while (!cars_.empty() && cars_.back().getPosition() > end_) {
        cars_.pop_back();
    }
    return {};
}

void Lane::addFlow(double dt){
    flowgen_.generateFlow(dt).transform([this](Car c){cars_.push_front(c); return c;});
}

void Lane::addCar(Car& c){
    cars_.push_front(c);
}

void Lane::setFlowGen(FlowGenerator& flowgen){
    flowgen_ = flowgen;
}

bool Lane::done() const {
    return !flowgen_.hasFlow() and cars_.empty();
}