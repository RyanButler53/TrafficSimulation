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

# Database Stuff
find_package(libpqxx CONFIG REQUIRED)

add_executable(dumpdb dumpDB.cpp)
target_link_libraries(dumpdb PRIVATE libpqxx::pqxx)

# Optional Dependencies
find_package(OpenSSL)
find_package(CURL)

include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)

add_executable(regression regression.cpp)
target_compile_definitions(regression PUBLIC $<IF:$<BOOL:${OpenSSL_FOUND}>,"WITH_OPEN_SSL","">)
target_link_libraries(regression PUBLIC simulation 
                                        jobManager
                                        $<IF:$<BOOL:OpenSSL_FOUND>,OpenSSL::Crypto,""> 
                                        GTest::gtest 
                                        GTest::gtest_main
                                        pqxx_wrapper)

add_executable(dbreader dbReader.cpp)
target_link_libraries(dbreader PUBLIC simulation 
                                      jobManager 
                                      dbReader
                                      GTest::gtest 
                                      GTest::gtest_main 
                                      pqxx_wrapper)


add_executable(apiTest apiTest.cpp)
target_link_libraries(apiTest PUBLIC api
                                     nlohmann_json::nlohmann_json
                                     yaml-cpp::yaml-cpp
                                     GTest::gtest
                                     GTest::gtest_main
                             PRIVATE CURL::libcurl
                            #  PRIVATE nlohmann_json::nlohmann_json
)
