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
#include "leadStrategy.hpp"
#include "strategy.hpp"
#include "carFactory.hpp"
#include "lane.hpp"
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
    Lane lane_;
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
     * @brief Parses the lead strategy
     * 
     * @throw Throws an error if the strategy does not supply the right data. 
     */
    std::expected<std::shared_ptr<LeadStrategy>, std::string> parseLeadStrategy(YAML::Node leadNode);

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
     * @brief Fills a lane with cars. Hook Method
     * 
     * @param lane Lane to populate with cars
     */
    virtual std::expected<void, std::string> parseLane(){return {};}

    template <typename T>
    std::expected<T, std::string> ParseField(YAML::Node node, std::string key){
        if (!node[key]){
            return std::unexpected("Field not present");
        }
        try {
            return node[key].as<T>();
        } catch(const std::exception& e)  {
            return std::unexpected(e.what());
        }   
    }

    public:
    Parser(YAML::Node cfg, std::filesystem::path cfgpath):cfg_{cfg},configPath_{cfgpath}{};

    /**
     * @brief Common algorithm to parse the inputs. 
     * 
     * @return Expected SimulatorInputs result or an error string
     */
    std::expected<SimulatorInputs, std::string> parse();
};

/**
 * @brief Parses the yaml input file for a discrete model (no continuously flowing cars in)
 * @note Discrete model means all car x0, v0 and vdes is described in the config
 * (Lanes too)
 */
class DiscreteParser : public Parser {

    /**
     * @brief Fills a lane with cars for a discrete simulation
     * 
     * @param lane Lane to populate with cars
     */
    virtual std::expected<void, std::string>  parseLane() override;

    public: 
    using Parser::Parser;
};

class ContinuousParser : public Parser {

    public:

    /**
     * @brief Parses the Flow for either a lane or default. 
     * 
     * @param flowNode 
     * @return FlowGenerator 
     */
    std::expected<FlowGenerator, std::string> parseFlow(YAML::Node flowNode) override;

    std::expected<void, std::string> parseLane() override;

    using Parser::Parser;
};

struct InvalidConfigError : public std::exception {

    std::string msg;

    InvalidConfigError(std::string message) throw():msg{message}{};
    virtual const char* what() throw() {return msg.c_str();};
};