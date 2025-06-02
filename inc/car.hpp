#include <ostream>
#include <iostream>

struct Car
{
    
    /// @brief Position is defined. Meters. 
    double pos_;
    
    /// @brief Current Velocity. Meters per second.
    double vel_;

    /// @brief Desired Velocity. (m/s)
    double desiredVel_;

    /// @brief Maximum Acceleration (m/s/s)
    const double maxAccel_;

    /// @brief Max Deceleration. m/s/s
    const double maxDecel_;

    /// @brief Current Timestep (seconds)
    double timestep_;

    /// @brief Car Length. (meters)
    double len_;

    /// @brief Gap between car and tail end of car in front of it. 
    double gap_;

    // IDM model parameters
    double accel_; // Acceleration
    double braking_; // Braking ability. Affects slowdowns
    double rxnTime_; // Reaction time

    // Stored previous state
    double oldPos_;
    double oldVel_;
    
    Car();
    ~Car() = default;

    // Simple step, no acceleration, open road
    void step(double dt);

    /**
     * @brief Takes a step forward in time. Takes the car in front of it into account
     * @param inFront Copy of car in front. (Original car has moved, cannot use its current data)
     */
    void step(const Car inFront, double dt);
    void log() const ;
    void log(std::ostream& os) const;


    /**
     * @brief Returns a car that is at position infinity. Allows for updating a car freely. 
     */
    static Car infinity();

    /**
     * @brief stores the current position and velocity of 
     */
    void store();

    /**
     * @brief Returns a car copy constructed from the previous state
     */
    Car restore();
};

std::ostream& operator<<(std::ostream& os, const Car& c);



