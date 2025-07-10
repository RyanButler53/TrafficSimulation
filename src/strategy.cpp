#include "strategy.hpp"

#include <algorithm>
#include <cmath>

// Gipps Driver Model

Gipps::Gipps(double accel, double braking, double bMax, double vDes):
    vDes_{vDes}, a_{accel}, b_{braking},bMax_{bMax} {}

double Gipps::update(double v, double vlead, double gap, double dt) const {

    // Prevent duplicate computations
    double ratio = v/vDes_;
    double bt = b_ * dt;
    
    // free road velocity
    double free_road = v + 2.5 * a_ * dt * (1 - ratio) * std::sqrt(0.025 +ratio);

    // Braking velocity
    double breaking = bt + std::sqrt((bt * bt) - b_ *((2 * (gap)) - v*dt - (vlead * vlead / bMax_)));
    return std::min(free_road, breaking);
}

// Intelligent Driver Model

Intelligent::Intelligent(double accel, double braking, double s0, double vDes):
    a_{accel}, b_{braking}, s0_{s0}, vDes_{vDes}{}

double Intelligent::update(double v, double vlead, double gap, double dt) const {
    
    // Convienence Variables
    double dv = v - vlead;
    double ratio = v/vDes_;

    // Compute Velocity
    double sDes = s0_ + std::max(0.0, v*dt + (v*dv)/(2*std::sqrt(2*a_*b_)));
    double final_accel = a_ * (1 - std::pow(ratio, 4) - std::pow(sDes/gap,2));
    return v + final_accel * dt;
}