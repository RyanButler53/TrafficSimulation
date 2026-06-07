/**
 * @file DBManager.hpp
 * @brief defines the DB Reader class
 * Depends on Api's DTO definitions
 */

#pragma once

#include <vector>
#include <memory>
#include <pqxx/pqxx>
#include <expected>

#include "structs.hpp"

using DBResponse = std::expected<void, std::string>;

/**
 * @class DB Manager
 * @brief Handles all API Database Queries and Updates
 * @details Implements many overloads of the query() function to handle
 * different endpoints
 * @note Can update the database through the DELETE job
 * 
 */

class DBManager {

    std::string connectionStr_;

    std::expected<pqxx::connection, std::string> getConnection();

public:

    // By default, constructs a reader for the PROD database. 
    DBManager(bool testDB = false);
    
    // Various overloads for each possible query to the DB. 
    std::expected<JobData, std::string> queryJobs(std::string jobname);
    std::expected<std::vector<JobData>, std::string> queryJobs();

    std::expected<CarMetadata, std::string> queryCars(std::string jobname, int carid);
    std::expected<std::vector<CarMetadata>, std::string> queryCars(std::string jobname);

    std::expected<RawData, std::string> queryData(std::string jobname, int carid);
    std::expected <std::vector<RawData>,std::string> queryData(std::string jobname);


    /**
     * @brief Gets the job id of a given job name. Can be used to check if a job already exists or not
     * 
     * @param jobname Job name to check
     * @return Integer values of jobids that match the provided name. Returns std::string if a DB error or no job present. 
     */
    std::expected<std::vector<int>, std::string> getJobId(std::string jobname);

    std::expected<void, std::string> deleteJob(std::string jobname);

};
