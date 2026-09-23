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
#include <expected>
#include <yaml-cpp/yaml.h>

#include "parser.hpp"

/**
 * @brief Small class to build parsers of various types (continuous, discrete, checkpointed). 
 * 
 */
class ParserFactory{

    YAML::Node cfg_; ///< Config representing the whole file
    std::filesystem::path cfgpath_; ///< path to input file
    
    public:

    /**
     * @brief Construct a new Parser Factory object
     * 
     * @param cfgpath Path to original input file
     */
    ParserFactory(std::filesystem::path cfgpath);

    /**
     * @brief Builds a parser based on data in the input file. 
     * 
     * @return std::expected<std::unique_ptr<Parser>, std::string> Parser on success, string on error. 
     */
    std::expected<std::unique_ptr<Parser>, std::string> makeParser();
};