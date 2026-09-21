/**
 * @file DBManager.hpp
 * @brief defines the DB Reader class
 * Depends on Api's DTO definitions
 */

#pragma once

#include <vector>
#include <memory>
#include <expected>
#include <optional>
#include "structs.hpp"
#include "shared/environment.hpp"

using DBResponse = std::expected<void, std::string>;

/**
 * @brief Handles all API Database Queries and Updates
 * @details Implements many overloads of the query() function to handle
 * different endpoints
 * @note Can update the database through the DELETE job
 * 
 */

class DBManager {

    /**
     * @brief Connection string to connect to the database.
     * @note There is not a persistent connection to the database. 
     */
    std::string connectionStr_;

public:

    // 
    /**
     * @brief Construct a new DBManager object.
     * @note By default, constructs a reader for the PROD database. 
     * @param testDB True to use the testDB, false to use prod DB
     */
    DBManager(bool testDB = false);
    
    /**
     * @brief Queries for top level job information for a single job by name.
     * @param jobname Name of the job to look up.
     * @return The job's data on success, or an error message on failure.
     */
    std::expected<JobData, std::string> queryJobs(std::string jobname);

    /**
     * @brief Queries for job information on all jobs.
     * @return Data for every job on success, or an error message on failure.
     */
    std::expected<std::vector<JobData>, std::string> queryJobs();

    /**
     * @brief Queries the metadata of a single car in the specified job.
     * @param jobname Name of the job containing the car.
     * @param carid Id of the car to look up.
     * @return The car's metadata on success, or an error message on failure.
     */
    std::expected<CarMetadata, std::string> queryCars(std::string jobname, int carid);

    /**
     * @brief Queries the metadata of all cars in the specified job.
     * @param jobname Name of the job to query.
     * @return Metadata for every car in the job on success, or an error
     *         message on failure.
     */
    std::expected<std::vector<CarMetadata>, std::string> queryCars(std::string jobname);

    /**
     * @brief Queries the raw snapshot data recorded for a single car in the specified job.
     * @param jobname Name of the job containing the car.
     * @param carid Id of the car to look up.
     * @return The car's snapshot data on success, or an error message on failure.
     */
    std::expected<RawData, std::string> queryData(std::string jobname, int carid);

    /**
     * @brief Queries the snapshot data recorded for all cars in a job.
     * @param jobname Name of the job to query.
     * @return Raw snapshot data for every car in the job on success, or an error
     *         message on failure.
     */
    std::expected<std::vector<RawData>, std::string> queryData(std::string jobname);

    /**
     * @brief Queries a job's data as a time series, optionally restricted
     *        to a position and time window. 
     * @param jobname Name of the job to query.
     * @param x0 Lower position bound. If empty, the lower bound is unrestricted.
     * @param x1 Upper position bound. If empty, the upper bound is unrestricted.
     * @param t0 Start time. If empty, the start is unrestricted.
     * @param t1 End time. If empty, the end is unrestricted.
     * @return The matching time series on success, or an error message on
     *         failure.
     */
    std::expected<TimeSeries, std::string> queryData(std::string jobname,
                                                     std::optional<double> x0, std::optional<double> x1,
                                                     std::optional<double> t0, std::optional<double> t1);

    /**
     * @brief Queries the environment settings of a job.
     * @param jobname Name of the job to query.
     * @return The job's environment on success, or an error message on failure.
     */
    std::expected<Environment, std::string> queryEnvironment(std::string jobname);

    /**
     * @brief Gets the job id of a given job name. Can be used to check if a job already exists or not
     * 
     * @param jobname Job name to check
     * @return Integer values of jobids that match the provided name. Returns std::string if a DB error or no job present. 
     */
    std::expected<std::vector<int>, std::string> getJobId(std::string jobname);

    /**
     * @brief Deletes all jobs from the database matching the specified job name
     * 
     * @param jobname Jobname to delete. 
     * @return std::expected<void, std::string> Nthing on success, error string on failure
     */
    std::expected<void, std::string> deleteJob(std::string jobname);

};
