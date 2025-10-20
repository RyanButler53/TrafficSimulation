/**
 * @file DBReader.hpp
 * @brief defines the DB Reader class
 * Depends on Api's DTO definitions
 */

#pragma once

#include <vector>
#include <memory>
#include <pqxx/pqxx>

#include "structs.hpp"

/**
 * @class DB Reader
 * @brief Returns the filled out DTO for the given request. 
 * @details Implements many overloads of the query() function to handle
 * different endpoints
 * 
 */
class DBReader {

    std::shared_ptr<pqxx::connection> connect_;

public:

    DBReader();

    // Various overloads for each possible query to the DB. 
    JobData queryJobs(std::string jobname);
    std::vector<JobData> queryJobs();

    CarMetadata queryCars(std::string jobname, int carid);
    std::vector<CarMetadata> queryCars(std::string jobname);

    RawData queryData(std::string jobname, int carid);
    std::vector<RawData> queryData(std::string jobname);

    bool checkJobName(std::string jobname);
};

class DBReadError : public std::exception {
    std::string msg_;
    public:
    DBReadError(std::string msg):msg_{msg}{}
    
    const char* what() const noexcept {return msg_.c_str();}
};