
# Yaml Parsing Library
include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG master
)
FetchContent_MakeAvailable(yaml-cpp)

set (SHARED_SRC
    car.cpp
    lane.cpp
    logger.cpp
    strategy.cpp)

set (SETUP_SRC
    carFactory.cpp
    parser.cpp
    )

set (SIMULATION_SRC
    simulator.cpp)

add_library(shared ${SHARED_SRC})
add_library(setup ${SETUP_SRC})
add_library(simulation ${SIMULATION_SRC})

target_link_libraries(setup yaml-cpp::yaml-cpp)

