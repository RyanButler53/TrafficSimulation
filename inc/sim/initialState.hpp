/**
 * @file initialState.hpp
 * @author Ryan Butler
 * @brief Namespace/class to build cars from a time series input. Needs a path of previous run data. 
 * @version 0.1
 * @date 2026-09-19
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once

#include "shared/expected.hpp"
#include <filesystem>
#include <yaml-cpp/yaml.h>
#include "highway.hpp"
#include "carFactory.hpp"
/**
 * @brief Abstract class that encapsulates all the data needed to construct the initial state of the
 * highway. 
 * @details The current simulation running will be at time = 0, but the highway will be at time t. 
 * The pure virtual method in this class is @ref populateHighway() from some (derived class) defined
 * "sink" source where a previous run or discrete input. 
 * 
 */
class InitialStateGenerator {


    public: 

    virtual ~InitialStateGenerator() {}

    /**
     * @brief Populates the provided highway with the state of the system at time t
     * 
     * @param highway empty highway object to populate
     * @param t Time t. Derived classes determine how to interpret their data source to build the cars
     * at this timestep. 
     * @param factory Car factory to build the cars. Ensures that the correct driver model is used
     * @return std::expected<void, std::string> Nothing on success, error string on error. Error string would be
     * if a car is out of bounds or if two cars overlap. 
     */
    virtual std::expected<void, std::string> populateHighway(std::shared_ptr<Highway> highway, double t, std::shared_ptr<CarFactory> factory) = 0;
};

/**
 * @brief Abstract class representing the File State. Provides utility functions for the IndividualFile
 * and Time Series File generators including CSV parsing utilities
 * 
 */
namespace FileState {

    /**
     * @brief Struct to hold car metadata parsed from the Car Metadata csv
     * 
     */
    struct Metadata {
        double a;
        double b; 
        double c;
        double p;
        double vdes;
    };

    /**
     * @brief Struct to hold snapshot data parsed from the specific 
     * 
     */
    struct SnapshotData {
        double x;
        double v;
        int lane;
    };

    // Utility to parse car metadata.csv
    std::expected<std::unordered_map<size_t, Metadata>, std::string> getCarMetadata(std::filesystem::path csv, std::unordered_set<size_t>&& ids);

    void populateHighway(const std::unordered_map<size_t, Metadata>& metadata, 
                         const std::unordered_map<size_t, SnapshotData>& snapshots,
                         std::shared_ptr<CarFactory> factory,
                         std::shared_ptr<Highway> hwy, 
                         double t);

};

/**
 * @brief Class representing the initial state from an individual car logging (logtype = file)
 * 
 */
class IndividualFileState : public InitialStateGenerator {
    std::filesystem::path folder_;

    /**
     * @brief Struct holding enough data to build a car from the correct factory
     * 
     */


    /// @brief Map holding cars and data of cars found to be within
    std::unordered_map<size_t, FileState::SnapshotData> m_cars;


    public:
    IndividualFileState(std::filesystem::path directory);

    std::expected<void, std::string> populateHighway(std::shared_ptr<Highway> highway, double t, std::shared_ptr<CarFactory> factory) override;
};

/**
 * @brief Initial state generator for time series file logging (logtype = time-series)
 * 
 */
class TimeSeriesFileState : public InitialStateGenerator {
    std::filesystem::path folder_;

    public:
    TimeSeriesFileState(std::filesystem::path directory);

    std::expected<void, std::string> populateHighway(std::shared_ptr<Highway> highway, double t, std::shared_ptr<CarFactory> factory) override;
};

/**
 * @brief Initial state generator for any flavor of database logging (logtype = db or test)
 * 
 */
class DatabaseFileState : public InitialStateGenerator {
    std::string jobname_;
    bool useTestDB_;

    public:
    DatabaseFileState(std::string jobname, bool useTestDB);

    std::expected<void, std::string> populateHighway(std::shared_ptr<Highway> highway, double t, std::shared_ptr<CarFactory> factory) override;
};

// /**
//  * @brief Initial State generator for a discrete initial state spelled out in the input file. 
//  * @note only available for non-checkpointed simulations since the checkpoint sets initial state. 
//  * 
//  */
// class DiscreteState : public InitialStateGenerator {
//     YAML::Node discreteNode_; 

//     public:
//     DiscreteState(YAML::Node cfgnode);

//     std::expected<void, std::string> populateHighway(std::shared_ptr<Highway> highway, double t, std::shared_ptr<CarFactory> factory) override;
// };
