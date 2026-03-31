/**
 * @file car.hpp
 * @author Ryan Buutler (rmbutler@outlook.com)
 * @brief Defines the car interface
 * @version 0.1
 * @date 2025-07-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include <ostream>
#include <iostream>
#include <optional>
#include "strategy.hpp"
#include "leadStrategy.hpp"
#include "logger.hpp"

class Car {
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

    /// @brief Politeness during lane changing. 
    double politeness_; 

    /// @brief Lead Car strategy. Constant, Discrete or Functional
    std::shared_ptr<LeadStrategy> leadStrategy_;

    /// @brief Car Following Strategy Either Intelligent or Gipps Driver Models
    std::shared_ptr<FollowStrategy> followStrategy_;

    /// @brief Logger. Could log to files and evantually a DB
    std::shared_ptr<CarLogger> logger_;
    
    // Private Methods

    /**
     * @brief Updates position and time
     * 
     * @param dt Timestep to incrememtn by 
     */
    void update(double dt);
    
    public: 

    // Constructors
    Car(size_t id, double x0, double v0, double t0, double politeness, std::shared_ptr<CarLogger> logger, 
        std::shared_ptr<FollowStrategy> follow);
    Car(size_t id, double x0, double v0, double t0, double politeness, std::shared_ptr<CarLogger>logger, 
        std::shared_ptr<FollowStrategy> follow, std::shared_ptr<LeadStrategy> lead);

    // Getters:
    double getPosition() const {return pos_;}
    double getVelocity() const {return vel_;}
    double getLength() const {return len_;}
    double politeness() const {return politeness_;}
    double braking() const {return followStrategy_->maxBraking();}

    // Set Lead Strategy 
    void setLeadStrategy(std::shared_ptr<LeadStrategy> ls) {leadStrategy_ = ls;}


    /**
     * @brief Calculates the acceleration of the car at current state
     * 
     * @overload Overload for the lead car with no. 
     * @param dt Timestep
     * @return double acceleration a
     */
    double acceleration(double dt) const;

    /**
     * @brief Calculates the acceleration of the car with a leader car
     * 
     * @overload OVerload for car with a leader
     * @param lead lead car
     * @param dt timestep
     * @return std::expected<double, std::string> acceleration on success, string on failure. 
     */
    std::expected<double, std::string> acceleration(const Car& lead, double dt) const;


    /**
     * @brief Update based on acceleration. Forwards to other update overload. 
     * 
     * @param acceleration a
     * @param dt timestep
     */
    void update(double acceleration, double dt);


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
     * @return Empty expected if 
     */
    std::optional<std::string> step(const Car& lead, double dt);

    void log() const;
    void log(std::ostream& os) const;

    bool operator<(const Car& other){
        return pos_ < other.pos_;
    }

};

std::ostream& operator<<(std::ostream& os, const Car& c);

