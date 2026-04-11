/**
 * @file strategy.hpp
 * @author Ryan Butler (rmbutler@outlook.com)
 * @brief Defines the interface for the Follow Strategy and 2 car following strategies
 * @version 0.1
 * @date 2025-07-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include <string>
#include <concepts>
#include <type_traits>

template <typename Model_t>
concept FollowModel = requires(Model_t model,
                                double d
                            ){
    /**
     * @brief Update based on v, vlead, gap and dt
     * 
     */
    {model.update(d, d, d, d)} -> std::same_as<double>;

    /**
     * @brief return a, b, s0/bmax
     * 
     */
    {model.params()} -> std::same_as<std::tuple<double, double, double>>;

    /**
     * @brief Return max braking allowed
     * 
     */
    {model.maxBraking()} -> std::same_as<double>;
};

 /**
 * @class FollowStrategy is a an Abstract Class for implementing a car following strategy
 * Strategies include:
 * Gipps Model
 * Intelligent
 * 
 * @note Car Following Strategy classes must implement update 
 */
// struct FollowStrategy {
    
//     FollowStrategy() = default;
//     virtual ~FollowStrategy() {};

//     /**
//      * @brief Computes the velocity of the car after time dt
//      * 
//      * @param v Current velocity
//      * @param vlead Velocity of lead car
//      * @param gap Gap between the current car and lead car. 
//      * @param dt Timestep to increment by. 
//      * @return double  new velocity of the car
//      * 
//      * @warning GAP INCLUDES THE LEAD VEHICLE'S LENGTH
//      */
//     virtual double update(double v, double vlead, double gap, double dt) const = 0;

//     /**
//      * @brief Returns the string for database logging
//      */
//     virtual std::tuple<double, double, double> params() const = 0;

//     /**
//      * @brief Returns the maximum braking coefficient. 
//      * 
//      */
//     virtual double maxBraking() const = 0;    
// };

/**
 * @class Gipps
 * @brief Gipps Model is a model of car following. Implements the update function 
 */
class Gipps {

    /// @brief acceleration
    double a_; 

    /// @brief braking
    double b_;

    /// @brief estimated max breaking of lead car
    double bMax_; 

    /// @brief desired velocity
    double vDes_;

    public:
    Gipps(double accel, double braking, double bMax, double vDes);
    ~Gipps() = default;

    /**
     * @brief Gipps model implementation of update()
     */
    double update(double v, double vlead, double gap, double dt) const;

    std::tuple<double, double, double> params() const {return {a_, b_, bMax_};};

    double maxBraking() const {return bMax_;}
};

class Intelligent {

    /// @brief Acceleration term 
    double a_;
    
    /// @brief Maximum breaking
    double b_;

    /// @brief Minimum desired separation (gap) between car and lead
    double s0_;

    /// @brief Desired velocity
    double vDes_;


    public:
    Intelligent(double accel, double braking, double s0, double vDes);
    ~Intelligent() = default;

    double update(double v, double vlead, double gap, double dt) const;

    std::tuple<double, double, double> params() const {return {a_, b_, s0_};};

    double maxBraking() const {return b_;}

};
