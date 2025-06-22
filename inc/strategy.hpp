
/**
 * @class FollowStrategy is a an Abstract Class for implementing a car following strategy
 * Strategies include:
 * Gipps Model
 * Intelligent
 * 
 * @note Car Following Strategy classes must implement update 
 */
struct FollowStrategy {
    
    FollowStrategy() = default;
    virtual ~FollowStrategy() {};

    /**
     * @brief Computes the velocity of the car after time dt
     * 
     * @param v Current velocity
     * @param vlead Velocity of lead car
     * @param gap Gap between the current car and lead car. 
     * @param dt Timestep to increment by. 
     * @return double  new velocity of the car
     * 
     * @warning GAP INCLUDES THE LEAD VEHICLE'S LENGTH
     */
    virtual double update(double v, double vlead, double gap, double dt) const = 0;
    
};

/**
 * @class Gipps
 * @brief Gipps Model is a model of car following. Implements the update function 
 */
class Gipps : public FollowStrategy{

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
    double update(double v, double vlead, double gap, double dt) const override;

};

class Intelligent : public FollowStrategy {

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

    double update(double v, double vlead, double gap, double dt) const override;

};
