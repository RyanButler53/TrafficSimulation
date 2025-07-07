#include <iostream>
#include <vector>
#include "simulator.hpp"


Simulator::Simulator(SimulatorInputs input): logger_{input.logger_},
    lane_{input.lane_}, totalTime_{input.totalTime_}, dt_{input.dt_}{}

void Simulator::run(){
    double t = 0;

    while (t < totalTime_){
        lane_.updateLane(dt_);
        t += dt_;
    }
    logger_->write();
}
