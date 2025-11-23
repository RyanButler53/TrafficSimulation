/**
 * @file DBReader.cpp
 * @author Ryan Butler (you@domain.com)
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

DBReader::DBReader(std::string connectionStr){
    const char* test = std::getenv("TRAFFIC_DEBUG");
    if (test){
        std::cout << "initializing the test DB" << std::endl;
        init("host=localhost port=5432 dbname=trafficDBTest");
    } else {
        std::cout << "Initializing caller provided DB" << std::endl;
        init(connectionStr);
    }
}

void DBReader::init(std::string connectionStr){
    try {   
        connect_ = std::make_shared<pqxx::connection>(connectionStr);
    } catch(const std::exception& e) {
        std::cout << "Error connecting to the database" << std::endl;
        std::cerr << e.what() << '\n';
    }
    if (!connect_){
        throw DBReadError("Could not connect to the database!");
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
        return std::unexpected(e.what());
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
        return std::unexpected(e.what());
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
        return std::unexpected(e.what());
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
        return std::unexpected(e.what());
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
        }    }
    catch(const std::exception& e) {
        return std::unexpected(e.what());
    }
    return alldata;
}

std::expected<bool, std::string> DBReader::checkJobName(std::string jobname) {
    pqxx::work tx{*connect_};
    std::string querystr = std::format("SELECT ('jobname', 'configfile')  FROM TrafficJobs WHERE jobname = '{}'", jobname);
    try {
        pqxx::result result = tx.exec(querystr);
        return result.empty();
    } catch(const std::exception& e) {
        return std::unexpected(e.what());
    }
}

std::expected<bool, std::string> DBReader::deleteJob(std::string jobname){

    auto deleteJob = [this, &jobname](bool) -> std::expected<bool, std::string>{
        try {
            pqxx::work tx{*connect_};
            std::string querystr = std::format("DELETE  FROM TrafficJobs WHERE jobname = '{}'", jobname);
            tx.exec(querystr);
            return true;
        } catch(const std::exception& e) {
            return std::unexpected(e.what());
        }
    };
    auto result = checkJobName(jobname).and_then(deleteJob);
    return result;
    // if(checkJobName(jobname).has_value()){
        
    // } else {
    //     return std::unexpected("Job not found.");
    // }
    
}