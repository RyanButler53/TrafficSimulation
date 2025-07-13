/**
 * @file lane.cpp
 * @author Ryan Butler (rmbutler@outlook.com)
 * @brief Implements the lane class. Only works for a single lane
 * @version 0.1
 * @date 2025-07-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "lane.hpp"

Lane::Lane(std::list<Car>& cars):cars_{std::move(cars)}{

}

void Lane::updateLane(double dt){
    if (cars_.empty()){
        std::cout << "Empty lane" << std::endl;
        return;
    }
    
    std::list<Car>::iterator current = cars_.begin();
    std::list<Car>::iterator next = ++cars_.begin();
    while (next != cars_.end()){
        Car& lead = *next;
        current->step(lead, dt);
        ++current;
        ++next;
    }

    // current points to the lead car
    current->step(dt);
}

void Lane::addCar(Car& c){
    cars_.push_front(c);
}
