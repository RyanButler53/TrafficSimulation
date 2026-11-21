cmake_minimum_required(VERSION 3.31.0)
set(VERSION 0.0.0)
project(TrafficSimulation)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 26) # uses reflection
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options("-freflection")
set(cmake_language CXX)

enable_testing()

include_directories(inc)
add_subdirectory(src)
add_subdirectory(scripts)
add_subdirectory(tests)
