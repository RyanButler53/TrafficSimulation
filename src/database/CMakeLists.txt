add_library(database DatabaseInit.cpp DBReader.cpp)
target_link_libraries(database PRIVATE pqxx_wrapper)
