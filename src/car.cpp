#include "car.hpp"

// 26.8s2 m/s = 60 mph, typical highway speed. 
Car::Car():pos_{0.0}, vel_{26.82}, desiredVel_{26.82},
                    maxAccel_{3.5}, maxDecel_{-4.572},timestep_{0},len_{4.9}{}

void Car::step(double dt){
    // Moving to the next timestep. 
    // Simple as x + v*dt
    // Acceleration later. 
    pos_ += vel_ * dt;
    timestep_ += dt;
}

void Car::log() const {
    std::cout << pos_ << "," <<vel_ <<","<<desiredVel_<<","<<timestep_<<"\n";
}

void Car::log(std::ostream& os) const {
    os << pos_ << "," <<vel_ <<","<<desiredVel_<<","<<timestep_;
}

std::ostream& operator<<(std::ostream& os, const Car& c){
    c.log(os);
    return os;
}