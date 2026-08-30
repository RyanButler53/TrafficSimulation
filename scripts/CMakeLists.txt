add_executable(clearProdDB migrateSchema.cpp)
target_link_libraries(clearProdDB PUBLIC database)
