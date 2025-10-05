/**
 * @file logger.hpp
 * @author Ryan Butler
 * @brief Header for logger and other logging related classes. 
 * @version 0.2
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
#include <exception>

/**
 * @brief Struct containing the minimum data about each car at a given timestep
 * 
 */
struct CarLog {
    size_t id;
    double x;
    double v;
    double t;
};

class CarLogger 
{

    /// @brief Uncommited logs. Unwritten to file or unwritten to database
    std::vector<CarLog> logs_;

    /// @brief Uncommitted logs split up by car id. 
    std::vector<std::vector<CarLog>> partitions_;

    protected:

    /// @brief Keeps track if partitions is current
    bool cached = false;

    void clearLogs();

    public:
    CarLogger() = default;
    virtual ~CarLogger(){};

    /**
     * @brief Fast Operation for logging a car object. Done at each timestep
     * 
     * @param id Car ID
     * @param x position
     * @param v Velocity
     * @param t timestep
     */
    void log(size_t id, double x, double v, double t); 

    /**
     * @brief Much slower operation for logging. Called to "commit" the 
     * logged results to a file or database. Can be left as a default 
     * for logging that doesn't need to write to files at all
     * 
     */
    virtual void write(){}; 

    /**
     * @brief Utility for accessing the logs of a specific car. Is expensive unless partition() has been called
     * 
     * @param id Car ID to get logs from
     * @return std::vector<CarLog> vector of logs
     */
    std::vector<CarLog> getCar(size_t id);

    
    /**
     * @brief Splits the logs up by which car id. To speed up getCar queries. 
     * 
     * @param n Number of cars. If not present, then logger needs to compute this 
     */
    void partition(size_t n = 0);

    std::vector<std::vector<CarLog>>& getPartition();


};

class FileLogger : public CarLogger {

    std::filesystem::path basepath_;
    public: 
    FileLogger(std::string basepath);
    ~FileLogger() = default;

    void write() override;

};

class DBLogger : public CarLogger {

    std::string tablePrefix_; // jobname + timestamp
    std::string configFile_; // important for traceability
    std::string jobname_;
    int ncars_; // number of corresponding tables

    public: 
    DBLogger(std::string jobname, std::string config);
    ~DBLogger() = default;

    // Commits to the database
    void write() override;
};

struct DatabaseError : public std::exception{
    std::string msg;

    DatabaseError(std::string message) throw():msg{message}{};
    virtual const char* what() throw() {return msg.c_str();};
};
