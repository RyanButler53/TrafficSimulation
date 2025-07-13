cmake_minimum_required(VERSION 3.31.0) # for docker

add_subdirectory(googletest)
include(GoogleTest)

# Address and Undefined behavior sanitizers need to be add separately
add_compile_options($<$<CONFIG:Debug>:-fsanitize=address>)
add_link_options($<$<CONFIG:Debug>:-fsanitize=address>)

add_compile_options($<$<CONFIG:Debug>:-fsanitize=undefined>)
add_link_options($<$<CONFIG:Debug>:-fsanitize=undefined>)

macro (TrafficTest testname src)
    add_executable("${testname}Test" ${src})
    target_link_libraries("${testname}Test" PRIVATE GTest::gtest)
    gtest_discover_tests("${testname}Test")
endmacro()

# TrafficTest(step step.cpp)
add_executable(parsing parsing.cpp)

target_link_libraries(parsing PUBLIC yaml-cpp::yaml-cpp PRIVATE GTest::gtest)

