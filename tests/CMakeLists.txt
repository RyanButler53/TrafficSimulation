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

# Test utility libraries
add_library(testutil STATIC testUtil.cpp)
target_link_libraries(testutil PUBLIC yaml-cpp::yaml-cpp PRIVATE database)

add_library(testCases STATIC AlgTest.cpp)
target_link_libraries(testCases PRIVATE yaml-cpp::yaml-cpp GTest::gtest)

# Test macros
macro(TrafficTest testname srcs tag)
    add_executable(${testname} ${srcs})
    target_link_libraries(${testname} PRIVATE GTest::gtest GTest::gtest_main ${ARGN})
    gtest_discover_tests(${testname} PROPERTIES LABELS ${tag})
endmacro()

macro(SingleFixtureTest testname srcs tag)
    add_executable(${testname} ${srcs})
    target_link_libraries(${testname} PRIVATE GTest::gtest GTest::gtest_main ${ARGN})
    add_test(NAME ${testname} COMMAND ${testname})
    set_tests_properties(${testname} PROPERTIES LABELS ${tag})
endmacro()

# Test Cases
TrafficTest(parsing parsing.cpp unit simulation yaml-cpp::yaml-cpp testutil)


TrafficTest(regression regression.cpp integration simulation
                                                  jobManager
                                                  testutil
                                                  pqxx_wrapper
                                                  $<IF:$<BOOL:${OpenSSL_FOUND}>,OpenSSL::Crypto,""> )
target_compile_definitions(regression PUBLIC $<IF:$<BOOL:${OpenSSL_FOUND}>,"WITH_OPEN_SSL","">)
target_compile_definitions(regression PUBLIC DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

TrafficTest(queueTest queueTest.cpp unit)
TrafficTest(laneinfoTest laneInfoTest.cpp unit shared)
TrafficTest(flowGeneratorTest flowGeneratorTest.cpp unit shared setup)

SingleFixtureTest(dbManagerTest dbManager.cpp integration testutil
                                                          jobManager 
                                                          dbmanager
                                                          pqxx_wrapper)
target_compile_definitions(dbManagerTest PUBLIC DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")


SingleFixtureTest(algorithmTest algorithmTest.cpp integration simulation 
                                                              testCases
                                                              testutil
                                                              dbmanager)

if (CURL_FOUND)
    # Bring in Json Dependency
    message(STATUS "Found CURL: Fetching JSON and enabling API Test")
    include(FetchContent)
    FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
    FetchContent_MakeAvailable(json)

    SingleFixtureTest(apiTest apiTest.cpp integration api 
                                                      nlohmann_json::nlohmann_json
                                                      testutil
                                                      CURL::libcurl)
    target_compile_definitions(apiTest PUBLIC DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

endif()

add_executable(timing timing.cpp)
target_link_libraries(timing PRIVATE simulation jobManager dbmanager pqxx_wrapper testutil)
