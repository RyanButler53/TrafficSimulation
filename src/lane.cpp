#include "lane.hpp"

Lane::Lane(std::list<Car>& cars):cars_{cars}{

}

void Lane::updateLane(double dt){
    if (cars_.empty()){
        std::cout << "Empty lane" << std::endl;
        return;
    }
    // First car is the "lead" car. Can move and accelerate freely. 
    std::list<Car>::iterator lead = cars_.begin();
    lead->store(); // cache x and v
    lead->step(Car::infinity(), dt);

    std::list<Car>::iterator car = ++lead;
    while (car != cars_.end())
    {
        Car oldLead = lead->restore();
        // Use v and x from the old leading car
        car->step(oldLead, dt);
        car->store(); // store the new v and x values
        ++car;
        ++lead; // incrementing lead should 
    }

}