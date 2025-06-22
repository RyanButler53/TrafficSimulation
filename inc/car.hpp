#include <ostream>
#include <iostream>
#include "strategy.hpp"
#include "leadStrategy.hpp"

class Car
{
    
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
    LeadStrategy* leadStrategy_;

    /// @brief Car Following Strategy Either Intelligent or Gipps Driver Models
    FollowStrategy* followStrategy_;

    // Private Methods

    /**
     * @brief Updates position and time
     * 
     * @param dt Timestep to incrememtn by 
     */
    void Car::update(double dt);
    
    public: 

    Car(double x0, double v0, double t0, FollowStrategy* follow);
    Car(double x0, double v0, double t0, FollowStrategy* follow, LeadStrategy* lead);
    ~Car();

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


    void log() const;
    void log(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const Car& c);

