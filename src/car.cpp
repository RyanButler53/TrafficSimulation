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
#include "car.hpp"
#include <algorithm>
#include <memory>


Car::Car(double x0, double v0, double t0, std::shared_ptr<CarLogger> logger, std::shared_ptr<FollowStrategy> follow):
        id_{Car::getId()}, pos_{x0}, vel_{v0}, timestep_{t0}, len_{4.9}, logger_{logger}, followStrategy_{follow}{
        // logger_ = std::shared_ptr<CarLogger>(logger);
        leadStrategy_ = std::make_shared<ConstantLead>(v0);
        // followStrategy_ = std::shared_ptr<FollowStrategy>(follow);
    }

Car::Car(double x0, double v0, double t0, std::shared_ptr<CarLogger> logger, 
         std::shared_ptr<FollowStrategy> follow, std::shared_ptr<LeadStrategy> lead):
        id_{Car::getId()},pos_{x0}, vel_{v0}, timestep_{t0}, len_{4.9}, logger_{logger}, 
        leadStrategy_{lead}, followStrategy_{follow}{}

// Initialize Car ID Static variable
size_t Car::carId_ = 0;

void Car::step(double dt){
    vel_ = leadStrategy_->nextVelocity(dt);
    update(dt);
}

void Car::step(const Car& lead, double dt){

    // Get information about lead car
    double xlead = lead.getPosition();
    double vlead = lead.getVelocity();
    double leadLen = lead.getLength();

    // Find the new velocity
    double gap = xlead - leadLen - pos_;
    vel_ = followStrategy_->update(vel_, vlead, gap, dt);
    
    // Update Position and time
    update(dt);
}

void Car::update(double dt){
    pos_ += vel_*dt;
    timestep_ += dt;
    log();
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
