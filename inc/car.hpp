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

    // lane data, agressiveness, idm models
public:
    Car();
    ~Car() = default;

    void step(double dt);
    void log() const ;
    void log(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const Car& c);
// std::ostream &operator<<(std::ostream& out, const SplayTree<key_t, value_t> &splaytree);



