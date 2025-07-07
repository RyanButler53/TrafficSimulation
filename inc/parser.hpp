#pragma once

#include <exception>
#include <filesystem>
#include <memory>

#include "yaml-cpp/yaml.h"
#include "leadStrategy.hpp"
#include "strategy.hpp"
#include "carFactory.hpp"
#include "lane.hpp"
#include "simInputs.hpp"

/**
 * @brief Parses the yaml input file for a discrete model (no continuously flowing cars in)
 * @note Discrete model means all car x0, v0 and vdes is described in the config
 * (Lanes too)
 */
class DiscreteParser {
    YAML::Node cfg_;
    std::shared_ptr<CarFactory> factory_;
    std::shared_ptr<CarLogger> logger_;
    double totaltime_;
    double dt_;

    // Private helper methods to break up parsing

    /**
     * @brief General stuff for all simulations
     * 
     */
    void parseGeneral();
    
    /**
     * @brief Parses the lead strategy
     * 
     * @param leadStrat Node with the Lead Strategy data
     * @throw Throws an error if the strategy does not supply the right data. 
     * @return std::shared_ptr<LeadStrategy> 
     */
    std::shared_ptr<LeadStrategy> parseLeadStrategy();

    /**
     * @brief Parses the Driver Factory. Can be either Gipps or Intelligent
     * @throw Throws an error if factory is not "Gipps" or "IDM" (case sensitive) or 
     * if the driver parameters are incorrect. 
     */
    void parseCarFactory();


    /**
     * @brief Fills a lane with cars
     * 
     * @param lane Lane to populate with cars
     */
    void  fillLane(Lane& lane);

    public: 
    DiscreteParser(std::filesystem::path yamlpath);

    SimulatorInputs parse();
};

struct InvalidConfigError : public std::exception {

    std::string msg;

    InvalidConfigError(std::string message) throw():msg{message}{};
    virtual const char* what() throw() {return msg.c_str();};
};