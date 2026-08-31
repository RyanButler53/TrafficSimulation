/**
 * @file testlib.hpp
 * @author Ryan Butler
 * @brief Testing utility functions
 * @version 0.1
 * @date 2026-06-03
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include "yaml-cpp/yaml.h"
#include <filesystem>


namespace TestUtil {

// Gets 2 lane config node
YAML::Node getConfigNode();

// 3 lane config node
YAML::Node getConfigNode_3Lane();


/**
 * @brief Clears out the test database. Pass through to Database::clearDB
 * 
 */
void clearDB();

/**
 * @brief Writes a config specified by a YAML node to the specified file
 * 
 * @param cfg config data
 * @param fname Filename to write to
 */
void configToFile(YAML::Node cfg, std::string fname);

/**
 * @brief Cleans up a file or vector of files if they exist
 * 
 * @param file File/files to remove
 */
void conditionalFileCleanup(std::string file);
void conditionalFileCleanup(std::vector<std::string> files);

/**
 * @brief Cleans up a file or vector of folders if they exist
 * 
 * @param file folder/folders to remove
 */
void conditionalFolderCleanup(std::filesystem::path folder);
void conditionalFolderCleanup(std::vector<std::filesystem::path> folders);

} // namespace TestUtil
