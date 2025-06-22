#include "lane.hpp"

Lane::Lane(std::list<Car>& cars):cars_{cars}{

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

    // Next is at the end 
    current->step(dt);


}