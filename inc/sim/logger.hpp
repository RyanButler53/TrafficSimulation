/**
 * @file logger.hpp
 * @author Ryan Butler
 * @brief Header for logger and other logging related classes. 
 * @version 0.3
 * @date 2025-06-22
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include <vector>
#include <algorithm>
#include <iterator>
#include <ranges>
#include <filesystem>
#include <unordered_map>
#include <expected>
#include "logStructs.hpp"

// Comms for streaming based logging
#include "comms.hpp"


/**
 * @brief Car Logger base class. Derived classes handle writing data to a sink (CSV file or sink)
 * @details contains methods for partitioning cars by the car id to improve disk write speeds. 
 * Contains many pure virtaul methods
 * @pure writeSnapshots(snapshots): Write the car snapshots
 * @pure writeCars(cars): Write Car metadata
 * @pure logFailure(error): Writes a failure
 * @pure writeStats(stats): Writes simulation stats
 * 
 */
class CarLogger 
{

    /**
     * @brief Writes the Car Snapshots to the file or database. Much slower as this hits 
     * the file system. This is called by the call operator to support streaming. 
     * 
     */
    virtual std::expected<void, std::string> writeSnapshots(std::vector<CarSnapshot> snapshots) = 0; 

    /**
     * @brief Writes Car Metadata to the file/database. 
     * 
     * @return std::expected<void, std::string> 
     */
    virtual std::expected<void, std::string> writeCars(std::vector<CarData> cardata) = 0;

    protected:
    
    /**
     * @brief Splits the logs up by which car id and sorted by timestamp. Used to arrange before calling WriteData;
     * 
     * @param [in] snapshots Car Snapshots sent to the logger 
     * @param [out] partitions Mapping of car ids to all their timestampe
     */


    void partition(std::vector<CarSnapshot>&& snapshots, std::unordered_map<size_t, std::vector<CarSnapshot>>& partitions);

    public:

    CarLogger() = default;
    virtual ~CarLogger(){};

    /**
     * @brief Write the simulation stats to the database. Called by the simulation
     * 
     * @param s 
     * @return std::expected<void, std::string> 
     */
    virtual std::expected<void, std::string> writeStats(SimulationStats s) = 0;

    /**
     * @brief Updates the simulation's status to a new status 
     * 
     * @return std::expected<void, std::string> 
     */
    virtual std::expected<void, std::string> updateStatus(std::string newStatus) {return {};}
    
    /**
     * @brief Writes and commits the job status as "ERROR" and populates the error message field
     * 
     */
    virtual std::expected<void, std::string> logFailure(std::string message) = 0;

    /**
     * @brief Streaming run function.  Reads data from the comms manager 
     * and gets it to the appropriate file sink
     * 
     * @param comms Communications manager that has Data Packets coming off the queue. 
     */
    void run(CommunicationsManager& comms);
};

class FileLogger : public CarLogger {

    std::filesystem::path basepath_;
    public: 
    FileLogger(std::string basepath);
    ~FileLogger() = default;

    std::expected<void, std::string> writeSnapshots(std::vector<CarSnapshot> snapshots) override;

    std::expected<void, std::string> writeCars(std::vector<CarData> data) override;

    std::expected<void, std::string> writeStats(SimulationStats s) override;

    std::expected<void, std::string> logFailure(std::string message) override;
};


/**
 * @class DBLogger
 * @brief Database Logger. Used for both Test and Prod DBs. 
 * 
 */
class DBLogger : public CarLogger {

    std::string configFile_; // important for traceability
    std::string jobname_;
    std::string connectionStr_;
    int jobid_;
    /// @brief Number of unique cars in the simulation
    size_t nCars_{0};

    DBLogger(std::string jobname, std::string config, bool test);
    
    /**
     * @brief Updates the specific field in the jobs table. 
     * 
     * @tparam T 
     * @return std::expected<void, std::string> 
     */
    template <typename T>
    std::expected<void, std::string> updateField(std::string field, T value, std::string errmsg);

    public: 

    /**
     * @brief Makes a Database Logger
     * 
     * @param jobname Job Name the logger represents
     * @param config Config file to use
     * @param Follow Model Type (Gipps/IDM/...)
     * @param test True to use test db, false for prod DB
     * @return std::expected<DBLogger*, std::string> 
     */
    static std::expected<std::shared_ptr<DBLogger>, std::string> make(std::string jobname, std::string config, std::string followType, bool test);
    ~DBLogger(){};


    // Overrides
    std::expected<void, std::string> writeSnapshots(std::vector<CarSnapshot> snapshots) override;

    std::expected<void, std::string> writeCars(std::vector<CarData> data) override;

    std::expected<void, std::string> writeStats(SimulationStats s) override;

    std::expected<void, std::string> updateStatus(std::string newStatus) override;

    std::expected<void, std::string> logFailure(std::string message) override;
};
