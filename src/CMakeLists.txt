add_subdirectory(simulation)
add_subdirectory(api)


# traffic driver executable
add_executable(traffic trafficJob.cpp)
target_link_libraries(traffic PRIVATE simulation)

find_package(oatpp 1.3.0 REQUIRED)

# Api executable
add_executable(trafficApi trafficApi.cpp)
target_include_directories(trafficApi PUBLIC inc)
target_link_libraries(trafficApi PUBLIC oatpp::oatpp PRIVATE dbReader jobManager)
