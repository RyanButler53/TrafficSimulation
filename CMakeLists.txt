cmake_minimum_required(VERSION 3.31.0)
set(VERSION 0.0.0)
project(TrafficSimulation)

include_directories(inc)


add_executable(sim src/simulate.cpp src/car.cpp src/lane.cpp src/strategy.cpp)