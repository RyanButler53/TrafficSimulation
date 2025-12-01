

# Find Dependencies
find_package(oatpp 1.3.0 REQUIRED)
find_package(libpqxx CONFIG REQUIRED)

add_library(api api.cpp)
target_link_libraries(api PUBLIC oatpp::oatpp pqxx_wrapper PRIVATE dbReader jobManager initdb)

# Separate dbreader and jobmanager into separate libs for testing
add_library(dbReader dbReader.cpp)
target_link_libraries(dbReader PUBLIC pqxx_wrapper)

add_library(jobManager jobManager.cpp)
target_link_libraries(jobManager PUBLIC shared PRIVATE setup simulation)
