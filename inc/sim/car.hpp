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
#include <expected>
#include "strategy.hpp"
#include "logStructs.hpp"

template <FollowModel Model_t>
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

    /// @brief Car Following Strategy Either Intelligent or Gipps Driver Models
    FollowModel followStrategy_;

    // Private Methods

    /**
     * @brief Updates position and time
     * 
     * @param dt Timestep to incrememnt by 
     */
    void update(double dt);
    
    public: 

    // Constructors
    Car(size_t id, double x0, double v0, double t0, double politeness,
        FollowModel follow);

    // Getters:
    double getPosition() const {return pos_;}
    double getVelocity() const {return vel_;}
    double getLength() const {return len_;}
    double politeness() const {return politeness_;}
    double braking() const {return followStrategy_.braking();}


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

    CarSnapshot snapshot(double t, uint16_t lane) const;

    bool operator<(const Car& other) const{
        return pos_ < other.pos_;
    }

};


