// Defines a Factory to create Car objects

#include "car.hpp"

enum class FollowStrategyType : char {
    GIPPS = 0,
    INTELLIGENT = 1
};

class CarFactory
{
private:
    FollowStrategyType fs_;

public:
    CarFactory(FollowStrategyType fs):fs_{fs}{}
    ~CarFactory() = default;

    // Makes a car object
    Car make();
};

// CarFactory::CarFactory(/* args */)
// {
// }

// CarFactory::~CarFactory()
// {
// }
