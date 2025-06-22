#include "car.hpp"
#include <algorithm>
#include <memory>


Car::Car(double x0, double v0, double t0, FollowStrategy* follow):
    pos_{x0}, vel_{v0}, timestep_{t0}, len_{4.9}{
        leadStrategy_ = std::make_shared<ConstantLead>(v0);
        followStrategy_ = std::shared_ptr<FollowStrategy>(follow);
    }

Car::Car(double x0, double v0, double t0, FollowStrategy* follow, LeadStrategy* lead):
    pos_{x0}, vel_{v0}, timestep_{t0}, len_{4.9}{
        leadStrategy_ = std::shared_ptr<LeadStrategy>(lead);
        followStrategy_ = std::shared_ptr<FollowStrategy>(follow);
    }


// Stepping forward functions

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
    std::cout << pos_ << "," <<vel_ <<","<<timestep_<<"\n";
}

void Car::log(std::ostream& os) const {
    os << pos_ << "," <<vel_ <<","<<timestep_;
}

std::ostream& operator<<(std::ostream& os, const Car& c){
    c.log(os);
    return os;
}
