

# Find Dependencies
find_package(oatpp 1.3.0 REQUIRED)
find_package(libpqxx CONFIG REQUIRED)

# Separate dbreader and jobmanager into separate libs for testing
add_library(dbReader dbReader.cpp)
target_link_libraries(dbReader PUBLIC libpqxx::pqxx)

add_library(jobManager jobManager.cpp)
target_link_libraries(jobManager PRIVATE setup simulation shared)
