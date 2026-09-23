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

namespace Database {

/**
 * @brief Initializes the database. This function creates the tables if they don't already exist. 
 * If the table exists, it does nothing and returns without an error
 * 
 * @param useTestDB True to use the test database, False to use the Prod DB
 * @return std::expected<void, std::string> Void on success, error message on error
 */
std::expected<void, std::string> initDB(bool useTestDB);

/**
 * @brief Clears out the database of all its tables. This is often used during integration tests
 * 
 * @param useTestDB True to use the test database, False to use the Prod DB
 * @return std::expected<void, std::string> Void on success, error message on error
 */
std::expected<void, std::string> clearDB(bool useTestDB);

}
