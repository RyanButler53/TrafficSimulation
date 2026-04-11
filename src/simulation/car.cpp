/**
 * @file car.cpp
 * @author Ryan Butler (rmbutler@outlook.com)
 * @brief Implements the Car class. 
 * @version 0.1
 * @date 2025-07-06
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "sim/car.hpp"
#include <algorithm>
#include <memory>
#include <optional>
#include <format>

template <FollowModel Model>
Car::Car(size_t id, double x0, double v0, double t0, double p, Model follow):
        id_{id}, pos_{x0}, vel_{v0}, timestep_{t0}, len_{4.9}, politeness_{p}, followStrategy_{follow}{
    }


double Car::acceleration(double dt) const {
    // With no lead car 
    Car lead(0, 1000000, vel_, 0, 0, nullptr);
    return acceleration(lead, dt).value();
}


std::expected<double, std::string> Car::acceleration(const Car& lead, double dt) const {
    // Get information about lead car
    double xlead = lead.getPosition();
    double vlead = lead.getVelocity();
    double leadLen = lead.getLength();

    // Check for an accident
    if (xlead <= pos_){
        std::string msg = std::format("Accident at t = {}: Car {}: x = {:.2f} Leader: x = {:.2f}", timestep_, id_, pos_, xlead);
        return std::unexpected(msg);
    }

    // Find the new velocity
    double gap = xlead - leadLen - pos_;
    double vf = followStrategy_.update(vel_, vlead, gap, dt);

    // a = dv/dt
    return (vf - vel_)/ dt;
}

void Car::update(double dt){
    pos_ += vel_*dt;
    timestep_ += dt;
}

void Car::update(double acceleration, double dt){
    vel_ += acceleration * dt;
    update(dt);
}

CarSnapshot Car::snapshot(double t, uint16_t lane) const {
    return {id_, pos_, vel_, t, lane};
}