/**
 * @file DBManager.cpp
 * @author Ryan Butler
 * @brief Implements the DB Reader class
 * @version 0.1
 * @date 2025-10-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "api/DBManager.hpp"
#include <format>
#include <iostream>
#include <unordered_map>
#include <cstdlib> 

DBManager::DBManager(bool testDB){
    if (testDB){
        connectionStr_ = "host=localhost port=5432 dbname=trafficDBTest";
    } else {
        connectionStr_ = "host=localhost port=5432 dbname=trafficDB";
    }
}

std::expected<pqxx::connection, std::string> DBManager::getConnection(){
    try {   
        return pqxx::connection(connectionStr_);
    } catch(const std::exception& e) {
        std::cout << "Error connecting to the database" << std::endl;
        std::cerr << e.what() << '\n';
        return std::unexpected(std::format("Error Connecting to the database: {}", e.what()));
    }
}


// JOB DATA

std::expected<JobData, std::string> DBManager::queryJobs(std::string jobname){
    std::string querystr = std::format("SELECT jobname, configfile, status, error, followModel, numCars  FROM TrafficJobs WHERE jobname = '{}'", jobname);

    auto connect = getConnection();
    if (!connect){return std::unexpected(connect.error());}
    pqxx::work tx{*connect};
    pqxx::result result;
    try {
        result = tx.exec(querystr);
    } catch(const std::exception& e) {
        return std::unexpected(std::format("Error executing SELECT query: {}",  e.what()));
    }

    if (result.empty()) {
        return std::unexpected("No job named " + jobname);
    }
    pqxx::row r = result[0];
    std::string name, cfgfile, error, status, followModel;
    int numCars;
    try {
        name = r["jobname"].as<std::string>();
        cfgfile = r["configfile"].as<std::string>();
        error = r["error"].as<std::string>();
        status = r["status"].as<std::string>();
        followModel = r["followModel"].as<std::string>();
        numCars = r["numCars"].as<int>();

    } catch(const std::exception& e) {
        return std::unexpected("Error converting name, cfgfile, error or status to a string");
    }

    return JobData{name, cfgfile, error, status, followModel, numCars};
}

std::expected<std::vector<JobData>, std::string> DBManager::queryJobs(){
    std::string querystr = std::format("SELECT jobname, configfile, status, error, followModel, numCars FROM TrafficJobs");
    auto connect = getConnection();
    if (!connect){return std::unexpected(connect.error());}
    pqxx::work tx{*connect};
    std::vector<JobData> data;
    try {
        for (auto [name, cfg, status, error, followModel, numCars] : tx.query<std::string, std::string, std::string, std::string, std::string, int>(querystr)){
            data.push_back({name, cfg, error, status, followModel, numCars});
        }    
    } catch(const std::exception& e) {
        return std::unexpected(std::format("Error in SELECT query from DB Reader: ", e.what()));
    }
    
    return data;
}

// CAR METADATA

std::expected<CarMetadata, std::string>  DBManager::queryCars(std::string jobname, int carid){
    auto connect = getConnection();
    if (!connect){return std::unexpected(connect.error());}
    pqxx::work tx{*connect};

    std::string querystr = std::format("SELECT follow_a, follow_b, follow_c, lead FROM CarData INNER JOIN TrafficJobs ON TrafficJobs.JobID = CarData.JobID WHERE jobname = '{}' and carid = {}", jobname, carid);
    pqxx::result result = tx.exec(querystr);
    if (result.empty()) {
        return std::unexpected(std::format("No car with id {} in job named {}", carid, jobname));
    }
    try {
        pqxx::row r = result[0];
        FollowModelParams modelParams = {
            r["follow_a"].as<float>(),
            r["follow_b"].as<float>(),
            r["follow_c"].as<float>()
        };
        return CarMetadata{r["lead"].as<std::string>(), modelParams, carid};
    } catch(const std::exception& e) {
        return std::unexpected(std::format("Error getting Car Metadata: {}", e.what()));
    }
}

std::expected<std::vector<CarMetadata>, std::string> DBManager::queryCars(std::string jobname){
    auto connect = getConnection();
    if (!connect){return std::unexpected(connect.error());}
    pqxx::work tx{*connect};
    std::string querystr = std::format("SELECT carid, follow_a, follow_b, follow_c, lead FROM CarData INNER JOIN TrafficJobs ON TrafficJobs.JobID = CarData.JobID WHERE jobname = '{}'", jobname);

    std::vector<CarMetadata> data;
    
    try {
        for (auto [carid, a, b, c, lead] : tx.query<int, float, float, float, std::string>(querystr)){
            data.push_back({lead, {a,b,c}, carid});
        }    }
    catch(const std::exception& e){
        return std::unexpected(std::format("Error Querying all Cars from {}: {}", jobname, e.what()));
    }

    if (data.empty()){
        return std::unexpected("No Data found. Check to see if the job exists");
    }
    return data;
}

// RAW DATA 

std::expected<RawData, std::string>  DBManager::queryData(std::string jobname, int carid){
    std::string querystr = std::format("SELECT x, v, t FROM snapshotData INNER JOIN TrafficJobs ON TrafficJobs.JobID = snapshotData.jobid WHERE TrafficJobs.jobname = '{}' and snapshotData.carid = {} ORDER BY snapshotData.t ASC", jobname, carid);
    auto connect = getConnection();
    if (!connect){return std::unexpected(connect.error());}
    pqxx::work tx{*connect};
    
    RawData raw;
    raw.id_ = carid;
    try {
        for (auto [x, v, t] : tx.query<float, float, float>(querystr)){
            raw.x_.push_back(x);
            raw.v_.push_back(v);
            raw.t_.push_back(t);
        }
    } catch(const std::exception& e) {
        return std::unexpected(std::format("Error querying Raw Data from car {} in job {}: {}",jobname, carid, e.what()));
    }
        
    if (raw.t_.empty()){
        return std::unexpected(std::format("No car snapshot data from car {} found", carid));
    }
    return raw;
}

std::expected<std::vector<RawData>, std::string> DBManager::queryData(std::string jobname){

    std::string querystr = std::format("SELECT carid, x, v, t FROM snapshotData INNER JOIN TrafficJobs ON TrafficJobs.JobID = snapshotData.jobid WHERE TrafficJobs.jobname = '{}' ORDER BY snapshotData.carid ASC, snapshotData.t ASC", jobname);

    auto connect = getConnection();
    if (!connect){return std::unexpected(connect.error());}
    pqxx::work tx{*connect};

    // Holds a vector of snapshots for each car
    std::vector<RawData> alldata;
    alldata.push_back(RawData());
    alldata.back().id_ = 0;
    try {
        for (auto [id, x, v, t] : tx.query<int, float, float, float>(querystr)){
            if (id != alldata.back().id_){
                alldata.push_back(RawData());
                alldata.back().id_ = id;
            } // sorted by id, so new car data
            alldata.back().x_.push_back(x);
            alldata.back().v_.push_back(v);
            alldata.back().t_.push_back(t);
        }
    }
    catch(const std::exception& e) {
        return std::unexpected(std::format("Error reading raw data from all cars for job {}: {}", jobname, e.what()));
    }
    return alldata;
}

std::expected<std::vector<int>, std::string> DBManager::getJobId(std::string jobname) {
    
    auto connect = getConnection();
    if (!connect){return std::unexpected(connect.error());}
    pqxx::work tx{*connect};
    std::vector<int> ids;
    std::string querystr = std::format("SELECT jobID FROM TrafficJobs WHERE jobname = '{}'", jobname);
    try {
        for (auto [id] : tx.query<int>(querystr)){
            ids.push_back(id);
        }
        if (ids.empty()){
            return std::unexpected(std::format("No jobs with job name \"{}\" not found", jobname));
        } else {
            return ids;
        }
    } catch(const std::exception& e) {
        return std::unexpected(std::format("Error checking getting job id from job {}: {}", jobname, e.what()));
    }
}

DBResponse DBManager::deleteJob(std::string jobname){

    auto deleteJobByID = [this, &jobname](const std::vector<int>&  jobids) -> DBResponse {
        try {
            for (const int& id : jobids){
                auto connect = getConnection();
                if (!connect){return std::unexpected(connect.error());}
                pqxx::work tx{*connect};
                std::string querystr = std::format("DELETE FROM snapshotData  WHERE jobid = '{}'", id);
                tx.exec(querystr);
                querystr = std::format("DELETE FROM CarData WHERE jobid = '{}'", id);
    
                tx.exec(querystr);
                querystr = std::format("DELETE FROM TrafficJobs WHERE jobid = '{}'", id);
    
                tx.exec(querystr);
                tx.commit();
            }
            return DBResponse{};
        } catch(const std::exception& e) {
            std::string errMsg = std::format("Error deleting {} from database: {}", jobname, e.what());
            return std::unexpected(errMsg);
        }
    };
    return getJobId(jobname).and_then(deleteJobByID);
}
