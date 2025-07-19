/**
 * @file parserFactory.hpp
 * @author Ryan Butler
 * @brief Declares the interface for the Parser Factory
 * @version 0.1
 * @date 2025-07-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <filesystem>
#include <memory>
#include <yaml-cpp/yaml.h>

#include "parser.hpp"

class ParserFactory{

    YAML::Node cfg_;

    public:

    ParserFactory(std::filesystem::path cfgpath);
    std::unique_ptr<Parser> makeParser();
};