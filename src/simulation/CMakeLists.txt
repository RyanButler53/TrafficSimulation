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
    highway.cpp
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


# Create pqxx Interface library wrapper to prevent duplicate linkage
add_library(pqxx_import STATIC IMPORTED)
find_library(pqxx_lib NAMES libpqxx.a PATHS ${libpqxx_DIR}/../../)
get_target_property(pqxx_inc libpqxx::pqxx INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(pqxx_link_libs libpqxx::pqxx INTERFACE_LINK_LIBRARIES)

set_target_properties(pqxx_import PROPERTIES IMPORTED_LOCATION ${pqxx_lib}
                                              INTERFACE_INCLUDE_DIRECTORIES ${pqxx_inc}
                                              INTERFACE_LINK_LIBRARIES ${pqxx_link_libs})
add_library(pqxx_wrapper INTERFACE)
target_link_libraries(pqxx_wrapper INTERFACE pqxx_import)

target_link_libraries(setup PUBLIC yaml-cpp::yaml-cpp)
target_link_libraries(shared PRIVATE pqxx_wrapper)
target_link_libraries(simulation PUBLIC setup shared initdb)

