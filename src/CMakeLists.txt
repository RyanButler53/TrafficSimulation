add_subdirectory(simulation)
# add_subdirectory(api)


# driver executable
add_executable(traffic trafficJob.cpp)
target_link_libraries(traffic PUBLIC setup simulation shared)

find_package(oatpp 1.3.0 REQUIRED)
message(STATUS "Found Oatpp: ${oatpp_DIR}")

# Api executable
# add_executable(trafficApi trafficApi.cpp)
# target_include_directories(trafficApi PUBLIC inc)
# target_link_libraries(trafficApi PUBLIC api)
