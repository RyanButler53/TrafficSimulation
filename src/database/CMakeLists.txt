add_library(initdb DatabaseInit.cpp)
target_link_libraries(initdb PRIVATE pqxx_wrapper)