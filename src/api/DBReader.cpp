/**
 * @file DBReader.cpp
 * @author Ryan Butler
 * @brief Implements the DB Reader class
 * @version 0.1
 * @date 2025-10-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "api/DBReader.hpp"
#include <format>
#include <iostream>
#include <unordered_map>
#include <cstdlib> 

DBReader::DBReader(bool testDB){
    if (testDB){
        init("host=localhost port=5432 dbname=trafficDBTest");
    } else {
        init("host=localhost port=5432 dbname=trafficDB");
    }
}

void DBReader::init(std::string connectionStr){
    try {   
        connect_ = std::make_shared<pqxx::connection>(connectionStr);
        // Initialize tables if not initialized

        /// @warning This code is duplicated from the DBLoggerBase's setup code
        // Create 3 tables: Traffic Jobs Main Table, Car Data Table (including strategies), Main data table (with car x,v,t values)
        pqxx::work tx(*connect_);
        tx.exec("CREATE TABLE IF NOT EXISTS trafficJobs ( jobID int GENERATED ALWAYS AS IDENTITY PRIMARY KEY, configfile text, jobname text)");
        tx.exec("CREATE TABLE IF NOT EXISTS carData (carID int, jobID int, follow text, lead text, FOREIGN KEY (jobID) REFERENCES trafficjobs(jobID) , PRIMARY KEY (carID, jobID))");
        tx.exec("CREATE TABLE IF NOT EXISTS snapshotData (carID int, jobID int, x float, v float, t float, PRIMARY KEY (carID, jobID, t), FOREIGN KEY (carID, jobID) REFERENCES cardata (carID, jobID))");
        tx.commit();
    } catch(const std::exception& e) {
        std::cout << "Error connecting to the database" << std::endl;
        std::cerr << e.what() << '\n';
    }
    if (!connect_){
        throw DBReadError("Fatal Error: Could not connect to the database!");
    }
}

// JOB DATA

std::expected<JobData, std::string> DBReader::queryJobs(std::string jobname){
    std::string querystr = std::format("SELECT jobname, configfile FROM TrafficJobs WHERE jobname = '{}'", jobname);
    pqxx::work tx{*connect_};
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
    std::string name,cfgfile;
    try {
        name = r[0].as<std::string>();
        cfgfile = r[1].as<std::string>();
    } catch(const std::exception& e) {
        return std::unexpected("Error converting name or cfgfile to a string");
    }
    
    return JobData{name, cfgfile};
}

std::expected<std::vector<JobData>, std::string> DBReader::queryJobs(){
    std::string querystr = std::format("SELECT jobname, configfile FROM TrafficJobs");
    pqxx::work tx{*connect_};
    std::vector<JobData> data;
    try {
        for (auto [name, cfg] : tx.query<std::string, std::string>(querystr)){
            data.push_back({name, cfg});
        }    
    } catch(const std::exception& e) {
        return std::unexpected(std::format("Error in SELECT query from DB Reader: ", e.what()));
    }
    
    return data;
}

// CAR METADATA

std::expected<CarMetadata, std::string>  DBReader::queryCars(std::string jobname, int carid){
    pqxx::work tx{*connect_};

    std::string querystr = std::format("SELECT follow, lead FROM CarData INNER JOIN TrafficJobs ON TrafficJobs.JobID = CarData.JobID WHERE jobname = '{}' and carid = {}", jobname, carid);
    pqxx::result result = tx.exec(querystr);
    if (result.empty()) {
        return std::unexpected("No job named " + jobname);
    }
    pqxx::row r = result[0];
    return CarMetadata{r[0].as<std::string>(), r[1].as<std::string>(), {}, carid};
}

std::expected<std::vector<CarMetadata>, std::string> DBReader::queryCars(std::string jobname){
    pqxx::work tx{*connect_};
    std::string querystr = std::format("SELECT carid, follow, lead FROM CarData INNER JOIN TrafficJobs ON TrafficJobs.JobID = CarData.JobID WHERE jobname = '{}'", jobname);

    std::vector<CarMetadata> data;
    
    try {
        for (auto [carid, lead, follow] : tx.query<int, std::string, std::string>(querystr)){
            data.push_back({lead, follow, {}, carid});
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

std::expected<RawData, std::string>  DBReader::queryData(std::string jobname, int carid){
    std::string querystr = std::format("SELECT x, v, t FROM snapshotData INNER JOIN TrafficJobs ON TrafficJobs.JobID = snapshotData.jobid WHERE TrafficJobs.jobname = '{}' and snapshotData.carid = {} ORDER BY snapshotData.t ASC", jobname, carid);
    pqxx::work tx{*connect_};
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
        
    return raw;
}

std::expected<std::vector<RawData>, std::string> DBReader::queryData(std::string jobname){

    std::string querystr = std::format("SELECT carid, x, v, t FROM snapshotData INNER JOIN TrafficJobs ON TrafficJobs.JobID = snapshotData.jobid WHERE TrafficJobs.jobname = '{}' ORDER BY snapshotData.carid ASC, snapshotData.t ASC", jobname);

    pqxx::work tx{*connect_};

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

std::expected<std::vector<int>, std::string> DBReader::getJobId(std::string jobname) {
    pqxx::work tx{*connect_};
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

std::expected<void, std::string> DBReader::deleteJob(std::string jobname){

    auto deleteJobByID = [this, &jobname](const std::vector<int>&  jobids) -> std::expected<void, std::string>{
        try {
            for (const int& id : jobids){
                pqxx::work tx{*connect_};
                std::string querystr = std::format("DELETE FROM snapshotData  WHERE jobid = '{}'", id);
                tx.exec(querystr);
                querystr = std::format("DELETE FROM CarData WHERE jobid = '{}'", id);
    
                tx.exec(querystr);
                querystr = std::format("DELETE FROM TrafficJobs WHERE jobid = '{}'", id);
    
                tx.exec(querystr);
                tx.commit();
            }
            return std::expected<void, std::string>{};
        } catch(const std::exception& e) {
            std::string errMsg = std::format("Error deleting {} from database: {}", jobname, e.what());
            return std::unexpected(errMsg);
        }
    };
    return getJobId(jobname).and_then(deleteJobByID);
}