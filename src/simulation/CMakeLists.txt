include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG master
)
FetchContent_MakeAvailable(yaml-cpp)

set (SHARED_SRC
    car.cpp
    flowGenerator.cpp
    lane.cpp
    logger.cpp
    strategy.cpp)

set (SETUP_SRC
    carFactory.cpp
    parserFactory.cpp
    parser.cpp
    )

set (SIMULATION_SRC
    simulator.cpp)

find_package(libpqxx CONFIG REQUIRED)

add_library(shared ${SHARED_SRC})
add_library(setup ${SETUP_SRC})
add_library(simulation ${SIMULATION_SRC})

target_link_libraries(setup yaml-cpp::yaml-cpp)
target_link_libraries(shared PUBLIC libpqxx::pqxx)

