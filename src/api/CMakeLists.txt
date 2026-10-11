

# Find Dependencies
find_package(oatpp 1.3.0 REQUIRED)
find_package(libpqxx CONFIG REQUIRED)

add_library(api api.cpp)
target_link_libraries(api PUBLIC oatpp::oatpp PRIVATE pqxx_wrapper jobManager database)



add_library(jobManager jobManager.cpp)
target_link_libraries(jobManager PUBLIC shared PRIVATE setup simulation)
