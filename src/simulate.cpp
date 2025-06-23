#include <iostream>
#include <vector>

#include "car.hpp"
#include "lane.hpp"
#include "strategy.hpp"
#include "leadStrategy.hpp"
#include "logger.hpp"

// Main driver code to run a simulation 

int main(){
    // Make a discrete leader
    std::vector<double> vleads { 
            22.776688,
            21.45792,
            20.56384,
            19.66976,
            18.77568,
            18.32864,
            17.43456,
            16.54048,
            15.6464,
            14.75232,
            13.85824,
            12.96416,
            11.176,
            9.83488,
            8.49376,
            9.38784,
            10.72896,
            11.62304,
            12.96416,
            13.85824,
            14.75232,
            16.09344,
            17.43456,
            18.77568,
            20.1168,
            21.90496,
            22.7588064,
            23.02256,
            22.6515168
    };

    CarLogger* logger = new FileLogger();
    DiscreteLead* leadStrat = new DiscreteLead(vleads);
    Gipps* leadFollowStrat = new Gipps(1.981, -2.8955, -3.505, 33.528);
    Gipps* followStrat = new Gipps(1.981, -2.8955, -3.505, 33.528);

    Car leader(0, 23.4114848, 0, logger, leadFollowStrat, leadStrat);
    Car follower(-60.96, 24.274, 0, logger, followStrat);

    std::list<Car> cars{follower, leader};
    Lane l;
    l.addCar(leader);
    l.addCar(follower);

    for (size_t i = 0; i < 29; ++i){
        l.updateLane(1);
    }

    logger->write();
    delete logger;
}