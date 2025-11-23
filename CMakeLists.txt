cmake_minimum_required(VERSION 3.31.0)
set(VERSION 0.0.0)
project(TrafficSimulation)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set (CMAKE_CXX_STANDARD 23) # uses views::zip and std::expected monads
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set(cmake_language CXX)

include_directories(inc)
add_subdirectory(src)
add_subdirectory(tests)
