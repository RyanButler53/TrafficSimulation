#pragma once

#include <ostream>
#include <iostream>
#include "strategy.hpp"
#include "leadStrategy.hpp"
#include "logger.hpp"

class Car
{
    /// @brief Unique id for each card
    size_t id_;
    
    /// @brief Position is defined. Meters. 
    double pos_;
    
    /// @brief Current Velocity. Meters per second.
    double vel_;

    /// @brief Current Timestep (seconds)
    double timestep_;

    /// @brief Car Length. (meters)
    double len_;

    /// @brief Gap between car and tail end of car in front of it. 
    double gap_;

    /// @brief Lead Car strategy. Constant, Discrete or Functional
    std::shared_ptr<LeadStrategy> leadStrategy_;

    /// @brief Car Following Strategy Either Intelligent or Gipps Driver Models
    std::shared_ptr<FollowStrategy> followStrategy_;

    /// @brief Logger. Could log to files and evantually a DB
    CarLogger* logger_;

    /// @brief Static: Car ID. Every time the car is created, use this
    static size_t carId_; 
    // Private Methods

    /**
     * @brief Updates position and time
     * 
     * @param dt Timestep to incrememtn by 
     */
    void update(double dt);
    
    public: 

    // Constructors
    Car(double x0, double v0, double t0, CarLogger* logger, FollowStrategy* follow);
    Car(double x0, double v0, double t0, CarLogger* logger, FollowStrategy* follow, LeadStrategy* lead);

    // Getters:
    double getPosition() const {return pos_;}
    double getVelocity() const {return vel_;}
    double getLength() const {return len_;}

    /**
     * @brief Takes a step forward in time. This overload is for when the car is the LEADER
     * 
     * @param dt 
     */
    void step(double dt);

    /**
     * @brief Takes a step forward in time. 
     * @details Takes the car in front of it into account based on the car in
     * front of it and its following strategy
     * 
     * @param lead Leader car. Passed by reference
     * @param dt Timestep. 
     */
    void step(const Car& lead, double dt);


    static size_t getId(){return carId_++;} //Car::carId++;

    void log() const;
    void log(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const Car& c);

