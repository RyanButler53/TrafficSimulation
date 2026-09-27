/**
 * @file parser.hpp
 * @author Ryan Butler (rmbutler@outlook.com)
 * @brief Defines interface for the discrete input parser for discretely defined simulations
 * @version 0.2
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
 * @brief Parser: A base class for Continuous and Discrete parsers. Continuous parsers 
 * have to handle flow based parameters
 * 
 */
class Parser {

    std::string highwayType_;                ///< Parallel highway backend

    protected:
    YAML::Node cfg_;                        ///< Node for the main yaml file.
    std::filesystem::path configPath_;      ///< Path to the input config file.
    std::shared_ptr<CarFactory> factory_;   ///< Factory used to create cars.
    std::shared_ptr<CarLogger> logger_;     ///< Logger that records car data.
    std::shared_ptr<Highway> highway_;      ///< Highway the simulation runs on.
    double totaltime_;                      ///< Total simulation time.
    double dt_;                             ///< Time step.
    int thinning_;                          ///< Logging thinning ratio. n thinning -> log every n timestamps
    uint64_t seed_;                         ///< Seed for rng. Used for Flow Generation
    std::string jobname_;                   ///< Name of the simulation job.


    // Template Utility functions of parsing algorithm

    /**
     * @brief General stuff for all simulations. This function parses the "jobinfo" section of the input file
     * @details Parses fields: Log directory, Time, dt, seed
     * @post Logger member var has been initialized after this
     * @return Nothing on success, error string on error 
     */
    std::expected<void, std::string> parseGeneral(YAML::Node node);

    /**
     * @brief Parses the Car Factory. Can be either Gipps or Intelligent
     * @details This function is meant to parse the "driver" field in the input file
     * @throw Throws an error if factory is not "Gipps" or "IDM" (case sensitive) or 
     * if the driver parameters are incorrect. 
     * @post Car Factory member var has been initialized after this
     * @return Nothing on success, error string on error 
     */
    std::expected<void, std::string> parseCarFactory(YAML::Node node);


    /**
     * @brief Creates the highway and sets up flow generators for the highway
     * @details This function parses the "highway" field of the input file
     * 
     * @param highwayParams Yaml Node containing the highway parameters including lane change parameters
     * and 
     */
    std::expected<void, std::string> parseHighway(YAML::Node highwayParams);

    /**
     * @brief Parses the initial state of the simulation. Builds the cars and puts them on the highway
     * at the initial timestamp
     * 
     * @param initialState 
     * @return std::expected<void, std::string> 
     */
    virtual std::expected<void, std::string> parseInitialState(YAML::Node initialState) = 0;

    /**
     * @brief Parses an individual field from the YAML node with a specified type. 
     * 
     * @tparam T Type to convert to. 
     * @param node Yaml node to parse
     * @param key Key value to get node[key]
     * @return std::expected<T, std::string> T on success, error message on success. Errors can be null node, no field or type conversion error.  
     */
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

    /**
     * @brief Parses a field that is many layers deep in a yaml node. 
     * 
     * @tparam T End type to convert the key to
     * @tparam Fields 
     * @param node Yaml node to parse
     * @param key Current key to use
     * @param keys Variable number of later keys to use
     * @return std::expected<T, std::string> T on success, error message on success.
     */
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

    /**
     * @brief Constructs a parser object.
     * 
     * @param cfg Yaml node representing the entire input file
     * @param cfgpath Path to the original config file. 
     */
    Parser(YAML::Node cfg, std::filesystem::path cfgpath):cfg_{cfg},configPath_{cfgpath}{};
    virtual ~Parser() {}

    /**
     * @brief Parse the entire simulator inputs.
     * @details Calls the protected member functions with various nodes of the input file
     * to build the entire simulator inputs. 
     * 
     * @return Expected SimulatorInputs result or an error string of which step of the parsing failed
     */
    virtual std::expected<SimulatorInputs, std::string> parse() = 0;
};

class ContinuousParser : public Parser {

    public:
    using Parser::Parser;

    std::expected<SimulatorInputs, std::string> parse() override;
    std::expected<void, std::string> parseInitialState(YAML::Node initialState) override {return {};}
    // std::expected<void, std::string> parseHighway(YAML::Node highwayParams) override;

};

// class CheckpointedParser : public Parser {

//     std::expected<YAML::Node, std::string> getPreviousConfig();

//     public:
//     using Parser::Parser;

//     std::expected<void, std::string> parseInitialState(YAML::Node initialState) override;

// };
