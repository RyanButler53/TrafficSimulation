/**
 * @file migrateSchema.cpp
 * @author Ryan Butler
 * @brief Small script to clear the production database and 
 * @version 0.1
 * @date 2026-07-27
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "database/databaseInit.hpp"
#include <print>

int main(){
    std::expected<void, std::string> result = Database::clearDB(false);
    if (!result){
        std::println("{}", result.error());
    }
}
