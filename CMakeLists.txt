cmake_minimum_required(VERSION 3.31.0)
set(VERSION 0.0.0)
project(TrafficSimulation)

set (CMAKE_CXX_STANDARD 20) #uses ranges and filesystem
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set(cmake_language CXX)

include_directories(inc)
add_subdirectory(src)
add_subdirectory(tests)

# driver executable
add_executable(traffic src/traffic.cpp)
target_link_libraries(traffic PUBLIC setup simulation shared)