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
    DBReader(bool testDB = false);

    // Various overloads for each possible query to the DB. 
    std::expected<JobData, std::string> queryJobs(std::string jobname);
    std::expected<std::vector<JobData>, std::string> queryJobs();

    std::expected<CarMetadata, std::string> queryCars(std::string jobname, int carid);
    std::expected<std::vector<CarMetadata>, std::string> queryCars(std::string jobname);

    std::expected<RawData, std::string> queryData(std::string jobname, int carid);
    std::expected <std::vector<RawData>,std::string> queryData(std::string jobname);

    std::expected<void, std::string> deleteJob(std::string jobname);

    /**
     * @brief Gets the job id of a given job name. Can be used to check if a job already exists or not
     * 
     * @param jobname Job name to check
     * @return Integer values of jobids that match the provided name. Returns std::string if a DB error or no job present. 
     */
    std::expected<std::vector<int>, std::string> getJobId(std::string jobname);
};

class DBReadError : public std::exception {
    std::string msg_;
    public:
    DBReadError(std::string msg):msg_{msg}{}
    
    const char* what() const noexcept {return msg_.c_str();}
};