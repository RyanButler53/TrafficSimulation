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


namespace TestUtil {

// Gets 2 lane config node
YAML::Node getConfigNode();

// 3 lane config node
YAML::Node getConfigNode_3Lane();


// Clear out the testing database
void clearDB();

} // namespace test util
