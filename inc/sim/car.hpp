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

    /**
     * @brief Constructs a car that represents a lead car in a free road environment
     * 
     * @return Car at "infinity"
     */
    Car infinity() const;
    
    /**
     * @brief Creates a new car at X. This car can only be used for comparison. 
     * 
     * @param x x position of the car
     */
    Car(double x);

    /**
     * @brief Returns the desired velocity. Stored in the follow strategy
     * 
     * @return double velocity in m/s
     */
    double vDes() const {return followStrategy_.vDes;}

    public: 

    /**
     * @brief Construct a new Car object with initial position, velocity, timestamp and follow model
     * 
     * @param id Car ID. 
     * @param x0 Initial position
     * @param v0 initial velocity
     * @param t0 Initial timestamp
     * @param politeness Lane changing politeness
     * @param follow Car following follow model
     */
    Car(size_t id, double x0, double v0, double t0, double politeness,
        FollowModel follow);

    // Getters:
    size_t getId() const {return id_;}
    double getPosition() const {return pos_;}
    double getVelocity() const {return vel_;}
    double getRearPosition() const {return pos_ - len_;}
    double politeness() const {return politeness_;}
    double braking() const {return followStrategy_.maxbraking;}


    /**
     * @brief Calculates the acceleration of the car at current state
     * 
     * @param dt Timestep
     * @return double acceleration a
     */
    double acceleration(double dt) const;

    /**
     * @brief Calculates the acceleration of the car with a leader car
     * 
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
     * @brief Returns a car snapshot of the car at the durrent time 
     * 
     * @param lane Current lane the car is in
     * @return CarSnapshot 
     */
    CarSnapshot snapshot(double t, uint16_t lane) const;

    /**
     * @brief Returns the car metadata in the CarData struct. Only called once
     * This dat includes follow model and lane change parameters. 
     * @return CarData 
     */
    CarData data() const;

    /**
     * @brief Creates a car that is only used for comparison with other cars
     * 
     * @param x X value to compare at. 
     * @return Car at specified x value
     */
    static Car compare(double x);

    /**
     * @brief Creates a stopped car at a position x. This car can be be used for acceleration calculations
     * 
     * @param x X value to compare at. 
     * @return Car at specified x value
     */
    static Car stoppedCar(double x);

    /**
     * @brief Comparison operator. Used to store in an std::map
     * 
     * @param other Car to compare against
     */
    bool operator<(const Car& other) const{
        return pos_ < other.pos_;
    }

};


