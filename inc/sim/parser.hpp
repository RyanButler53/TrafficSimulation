/**
 * @file parser.hpp
 * @author Ryan Buutler (rmbutler@outlook.com)
 * @brief Defines interface for the discrete input parser for discretely defined simulations
 * @version 0.1
 * @date 2025-07-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include <exception>
#include <filesystem>
#include <memory>
#include <expected>

#include <yaml-cpp/yaml.h>
#include "strategy.hpp"
#include "carFactory.hpp"
#include "highway.hpp"
#include "simInputs.hpp"

/**
 * @class Parser: A base class for Continuous and Discrete parsers. Continuous parsers 
 * have to handle flow based parameters
 * 
 */
class Parser {

    protected:
    YAML::Node cfg_;
    std::filesystem::path configPath_; // Required for reproducing simulation runs
    std::shared_ptr<CarFactory> factory_;
    std::shared_ptr<CarLogger> logger_;
    std::shared_ptr <Highway> highway_;
    double totaltime_;
    double dt_;
    uint64_t seed_;


    // Template Utility functions of parsing algorithm

    /**
     * @brief General stuff for all simulations
     * @details Log directory, Time, dt, seed
     */
    std::expected<void, std::string> parseGeneral();

    /**
     * @brief Parses the Driver Factory. Can be either Gipps or Intelligent
     * @throw Throws an error if factory is not "Gipps" or "IDM" (case sensitive) or 
     * if the driver parameters are incorrect. 
     */
    std::expected<void, std::string> parseCarFactory(void);

    /**
     * @brief Parses the node for Flow Generation parameters
     * 
     * @param flowNode Node with flow generation
     * @return FlowGenerator with zero flow. Override to parse flow. 
     */
    virtual std::expected<FlowGenerator, std::string> parseFlow(YAML::Node flowNode){return FlowGenerator();}

    /**
     * @brief Creates the highway and sets up flow generators for highway construction. 
     * 
     * @param lane Lane to populate with cars
     */
    virtual std::expected<void, std::string> parseHighway() = 0;

    template <typename T>
    static std::expected<T, std::string> ParseField(YAML::Node node, std::string key){
        if (!node){
            return std::unexpected("Nullptr node");
        } else if (!node[key]){
            return std::unexpected("Field not present");
        }
        try {
            return node[key].as<T>();
        } catch(const std::exception& e)  {
            return std::unexpected(std::format("Error parsing {} field: {}", key, e.what()));
        }   
    }

    template <typename T, typename... Fields>
    static std::expected<T, std::string> ParseField(YAML::Node node, std::string key, Fields... keys){
        // Base case: There is only one field. 
        if constexpr (sizeof...(keys) == 0){
            return ParseField<T>(node, key);
        } else {
            return ParseField<YAML::Node>(node, key).and_then([&keys...](YAML::Node n){return ParseField<T>(n, keys...);});
        }
    }

    public:
    Parser(YAML::Node cfg, std::filesystem::path cfgpath):cfg_{cfg},configPath_{cfgpath}{};
    virtual ~Parser() {}

    /**
     * @brief Common algorithm to parse the inputs. 
     * 
     * @return Expected SimulatorInputs result or an error string
     */
    std::expected<SimulatorInputs, std::string> parse();
};

class ContinuousParser : public Parser {

    public:

    std::expected<void, std::string> parseHighway() override;

    using Parser::Parser;
};
