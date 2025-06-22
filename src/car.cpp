#include "car.hpp"
#include <algorithm>

// 26.82 m/s = 60 mph, typical highway speed. 
Car::Car():pos_{0.0}, vel_{26.82},timestep_{0}, len_{4.9}{}


void Car::step(const Car& lead, double dt){

    // Get information about lead car
    double xlead = lead.getPosition();
    double vlead = lead.getVelocity();
    double leadLen = lead.getLength();

    // Find the new velocity
    double gap = xlead - leadLen - pos_;
    vel_ = updateStrategy_->update(vel_, vlead, gap, dt);
    
    // Update Position and time
    pos_ += vel_*dt;
    timestep_ += dt;
    log();
}

void Car::log() const {
    std::cout << pos_ << "," <<vel_ <<","<<timestep_<<"\n";
}

void Car::log(std::ostream& os) const {
    os << pos_ << "," <<vel_ <<","<<timestep_;
}

// Car Car::infinity(double x, double v) {
//     Car c;
//     c.pos_ = std::numeric_limits<double>::max();
//     c.vel_;
//     return c;
// }

std::ostream& operator<<(std::ostream& os, const Car& c){
    c.log(os);
    return os;
}
