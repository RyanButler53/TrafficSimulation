cmake_minimum_required(VERSION 3.31.0) # for docker

add_subdirectory(googletest)
include(GoogleTest)

add_compile_options($<$<CONFIG:Debug>:"-fsanitize=address -fsanitize=undefined">)
add_link_options($<$<Config:Debug>:"-fsanitize=address -fsanitize=undefined">)

macro (TrafficTest testname src)
    add_executable("${testname}Test" ${src})
    target_link_libraries("${testname}Test" PRIVATE GTest::gtest)
    gtest_discover_tests("${testname}Test")
endmacro()

TrafficTest(step step.cpp)
