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

add_executable(parsing parsing.cpp)
target_link_libraries(parsing PUBLIC simulation 
                                     yaml-cpp::yaml-cpp 
                              PRIVATE GTest::gtest GTest::gtest_main)


add_executable(regression regression.cpp)
target_compile_definitions(regression PUBLIC $<IF:$<BOOL:${OpenSSL_FOUND}>,"WITH_OPEN_SSL","">)
target_link_libraries(regression PRIVATE simulation 
                                        jobManager
                                        testutil
                                        $<IF:$<BOOL:OpenSSL_FOUND>,OpenSSL::Crypto,""> 
                                        GTest::gtest 
                                        GTest::gtest_main
                                        pqxx_wrapper)

add_executable(dbManagerTest dbManager.cpp)
target_link_libraries(dbManagerTest PRIVATE simulation 
                                      testutil
                                      jobManager 
                                      dbmanager
                                      GTest::gtest 
                                      GTest::gtest_main 
                                      pqxx_wrapper)

add_executable(algorithmTest algorithmTest.cpp)
target_link_libraries(algorithmTest PUBLIC simulation 
                                            testutil
                                            dbmanager
                                            GTest::gtest
                                            GTest::gtest_main
                                            )

if (CURL_FOUND)
    # Bring in Json Dependency
    message(STATUS "Found CURL: Fetching JSON and enabling API Test")
    include(FetchContent)
    FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
    FetchContent_MakeAvailable(json)

    add_executable(apiTest apiTest.cpp)
    target_link_libraries(apiTest PUBLIC api
                                        nlohmann_json::nlohmann_json
                                        testutil
                                        GTest::gtest
                                        GTest::gtest_main
                                PRIVATE CURL::libcurl
)
endif()
