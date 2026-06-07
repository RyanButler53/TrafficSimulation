/**
 * @file DatabaseInit.hpp
 * @author Ryan Butler
 * @brief Shared Resources that Initialized the database
 * @version 0.1
 * @date 2025-11-30
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <expected>
#include <string>

namespace initDB {

std::expected<void, std::string> initDB(bool useTestDB);

}