/**
 * @file LeadStrategy.hpp
 * @author Ryan Butler
 * @brief Header file for the possible Lead Functions
 * @version 0.1
 * @date 2025-06-22
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include <vector>
#include <functional>

struct LeadStrategy {
    LeadStrategy() = default;
    virtual ~LeadStrategy(){}

    virtual double nextVelocity(double dt) = 0;
};

/**
 * @brief Class for holding a constant velocity. 
 * 
 */
class ConstantLead : public LeadStrategy {
    
    double v_;

    public:
    ConstantLead(double v):v_{v}{}
    ~ConstantLead() = default;

    double nextVelocity(double dt) override {return v_;}
};

/**
 * @brief Class for discretly setting velocities and times
 * 
 */
class DiscreteLead : public LeadStrategy {
    
    std::vector<double> velocities_;
    std::vector<double>::iterator next_;

    public:
    DiscreteLead(std::vector<double> vs):velocities_{vs},next_{velocities_.begin()}{}
    ~DiscreteLead() = default;

    double nextVelocity(double dt){
        double v = *next_;
        ++next_;
        if (next_ == velocities_.end()){
            next_ = velocities_.begin(); // loop around. 
        }
        return v;
    }

};

/**
 * @brief Class for using a Function for the car's velocity when
 * in the leading position. 
 * 
 */
class FunctionLead : public LeadStrategy {
    std::function<double(double)> func_;
    double t_;

    public:
    FunctionLead(std::function<double(double)> f):func_{f}, t_{0}{}

    // this actually won't work...
    FunctionLead(std::function<double(double)> f, double t0):func_{f}, t_{t0}{}

    ~FunctionLead() = default;

    double nextVelocity(double dt){
        double v = func_(t_);
        t_ += dt;
        return v;
    }
};