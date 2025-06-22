#include <ostream>
#include <iostream>
#include "strategy.hpp"

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

    // Update Strategy. Either Intelligent or Gipps Driver Models
    FollowStrategy* updateStrategy_;
    
    public: 
    
    Car();

    ~Car(){delete updateStrategy_;}

    // Getters:
    double getPosition() const {return pos_;}
    double getVelocity() const {return vel_;}
    double getLength() const {return len_;}



    /**
     * @brief Takes 
     * @param inFront Copy of car in front. (Original car has moved, cannot use its current data)
     */

    /**
     * @brief Takes a step forward in time. 
     * @details Takes the car in front of it into account based on the car in
     * front of it and its following strategy
     * 
     * @param lead Leader car. Passed by reference
     * @param dt Timestep. 
     */
    void step(const Car& lead, double dt);


    void log() const ;
    void log(std::ostream& os) const;



};

std::ostream& operator<<(std::ostream& os, const Car& c);



