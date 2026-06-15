cmake_minimum_required(VERSION 3.31.0) # for docker

add_subdirectory(googletest)
include(GoogleTest)

# Address and Undefined behavior sanitizers need to be add separately
add_compile_options($<$<CONFIG:Debug>:-fsanitize=address>)
add_link_options($<$<CONFIG:Debug>:-fsanitize=address>)

add_compile_options($<$<CONFIG:Debug>:-fsanitize=undefined>)
add_link_options($<$<CONFIG:Debug>:-fsanitize=undefined>)
# Database Stuff
find_package(libpqxx CONFIG REQUIRED)

# Optional Dependencies
find_package(OpenSSL)
find_package(CURL)

add_library(testutil STATIC testUtil.cpp)
target_link_libraries(testutil PUBLIC yaml-cpp::yaml-cpp PRIVATE pqxx_wrapper)


macro(TrafficTest testname srcs)
    add_executable(${testname} ${srcs})
    target_link_libraries(${testname} PRIVATE GTest::gtest GTest::gtest_main ${ARGN})
    gtest_discover_tests(${testname} )
endmacro()

macro(SingleFixtureTest testname srcs)
    add_executable(${testname} ${srcs})
    target_link_libraries(${testname} PRIVATE GTest::gtest GTest::gtest_main ${ARGN})
    add_test(NAME ${testname} COMMAND ${testname})
endmacro()

TrafficTest(parsing parsing.cpp simulation yaml-cpp::yaml-cpp)


TrafficTest(regression regression.cpp simulation
                                      jobManager
                                      testutil
                                      pqxx_wrapper
                                      $<IF:$<BOOL:${OpenSSL_FOUND}>,OpenSSL::Crypto,""> )
target_compile_definitions(regression PUBLIC $<IF:$<BOOL:${OpenSSL_FOUND}>,"WITH_OPEN_SSL","">)

SingleFixtureTest(dbManagerTest dbManager.cpp testutil
                                        jobManager 
                                        dbmanager
                                        pqxx_wrapper)


SingleFixtureTest(algorithmTest algorithmTest.cpp simulation 
                                            testutil
                                            dbmanager)

if (CURL_FOUND)
    # Bring in Json Dependency
    message(STATUS "Found CURL: Fetching JSON and enabling API Test")
    include(FetchContent)
    FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
    FetchContent_MakeAvailable(json)

    SingleFixtureTest(apiTest apiTest.cpp api
                                    nlohmann_json::nlohmann_json
                                    testutil
                                    CURL::libcurl)
endif()
