#include "car.hpp"
#include <algorithm>

// 26.82 m/s = 60 mph, typical highway speed. 
Car::Car():pos_{0.0}, vel_{26.82}, desiredVel_{26.82},
            maxAccel_{3.5}, maxDecel_{-4.572},timestep_{0},
            len_{4.9}, accel_{2.0}, braking_{3.0}, rxnTime_{1.8} {}

void Car::step(double dt){
    pos_ += vel_ * dt;
    timestep_ += dt;
}

void Car::step(const Car lead, double dt){
    double dist = lead.pos_ - lead.len_ - pos_;
    double dv = vel_ - lead.vel_;
    // double deltaVterm = 
    double desiredDist = dist + std::max(0.0, (vel_*rxnTime_ +(vel_*dv)/(2*std::sqrt(accel_*braking_))));
    double dvdt = accel_ * (1 - std::pow((vel_/desiredVel_), 4) - desiredDist/dist);
    // Do the update
    vel_ += dvdt * dt;
    pos_ += (vel_ + 0.5 * dvdt * dt ) * dt;
}

void Car::log() const {
    std::cout << pos_ << "," <<vel_ <<","<<desiredVel_<<","<<timestep_<<"\n";
}

void Car::log(std::ostream& os) const {
    os << pos_ << "," <<vel_ <<","<<desiredVel_<<","<<timestep_;
}

Car Car::infinity() {
    Car c;
    c.pos_ = std::numeric_limits<double>::max();
    return c;
}

// Caching Functions
void Car::store() {
    oldVel_ = vel_;
    oldPos_ = pos_;
}

Car Car::restore() {
    Car c;
    c.vel_ = oldVel_;
    c.pos_ = oldPos_;
    c.len_ = len_;
    return c;
}

std::ostream& operator<<(std::ostream& os, const Car& c){
    c.log(os);
    return os;
}
