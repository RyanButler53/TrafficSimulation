/**
 * @file DBReader.hpp
 * @brief defines the DB Reader class
 * Depends on Api's DTO definitions
 */

#pragma once

#include <vector>
#include <memory>
#include <pqxx/pqxx>
#include <expected>

#include "structs.hpp"

/**
 * @class DB Reader
 * @brief Returns the filled out DTO for the given request. 
 * @details Implements many overloads of the query() function to handle
 * different endpoints
 * @note Can update the database through the DELETE job
 * 
 */

class DBReader {

    std::shared_ptr<pqxx::connection> connect_;

    void init(std::string connectStr);

public:

    // By default, constructs a reader for the PROD database. 
    DBReader(std::string connectionStr = "host=localhost port=5432 dbname=trafficDB");

    // Various overloads for each possible query to the DB. 
    std::expected<JobData, std::string> queryJobs(std::string jobname);
    std::expected<std::vector<JobData>, std::string> queryJobs();

    std::expected<CarMetadata, std::string> queryCars(std::string jobname, int carid);
    std::expected<std::vector<CarMetadata>, std::string> queryCars(std::string jobname);

    std::expected<RawData, std::string> queryData(std::string jobname, int carid);
    std::expected <std::vector<RawData>,std::string> queryData(std::string jobname);

    std::expected<bool, std::string> deleteJob(std::string jobname);

    /**
     * @brief Checks if the job name exists
     * 
     * @param jobname Job name to check
     * @return true if the job name is availane
     * @return false if the job name is NOT available 
     */
    std::expected<bool, std::string> checkJobName(std::string jobname);
};

class DBReadError : public std::exception {
    std::string msg_;
    public:
    DBReadError(std::string msg):msg_{msg}{}
    
    const char* what() const noexcept {return msg_.c_str();}
};