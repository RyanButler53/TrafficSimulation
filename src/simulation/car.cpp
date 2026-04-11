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


Car::Car(size_t id, double x0, double v0, double t0, double p, std::shared_ptr<CarLogger> logger, std::shared_ptr<FollowStrategy> follow):
        id_{id}, pos_{x0}, vel_{v0}, timestep_{t0}, len_{4.9}, politeness_{p}, logger_{logger}, followStrategy_{follow}{
        logger->log(id_, x0, v0, t0);
    }


double Car::acceleration(double dt) const {
    // With no lead car 
    return std::get<0>(followStrategy_->params());
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
    double vf = followStrategy_->update(vel_, vlead, gap, dt);

    // a = dv/dt
    return (vf - vel_)/ dt;
}

void Car::step(double dt){
    vel_ = acceleration(dt) * dt;
    update(dt);
}

std::optional<std::string> Car::step(const Car& lead, double dt){

    // Get information about lead car
    double xlead = lead.getPosition();
    double vlead = lead.getVelocity();
    double leadLen = lead.getLength();

    // Check for an accident
    if (xlead <= pos_){
        std::string msg = std::format("Accident at t = {}: Car {}: x = {:.2f} Leader: x = {:.2f}", timestep_, id_, pos_, xlead);
        return std::make_optional(msg);
    }

    // Find the new velocity
    double gap = xlead - leadLen - pos_;
    vel_ = followStrategy_->update(vel_, vlead, gap, dt);
    
    // Update Position and time
    update(dt);
    return std::nullopt;
}

void Car::update(double dt){
    pos_ += vel_*dt;
    timestep_ += dt;
    log();
}

void Car::update(double acceleration, double dt){
    vel_ += acceleration * dt;
    update(dt);
}

// Logging Methods

void Car::log() const {
    logger_->log(id_, pos_, vel_, timestep_);
}

void Car::log(std::ostream& os) const {
    os << pos_ << "," <<vel_ <<","<<timestep_;
}

std::ostream& operator<<(std::ostream& os, const Car& c){
    c.log(os);
    return os;
}
