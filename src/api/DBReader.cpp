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

DBReader::DBReader(std::string connectionStr){
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

JobData DBReader::queryJobs(std::string jobname){
    std::string querystr = std::format("SELECT jobname, configfile FROM TrafficJobs WHERE jobname = '{}'", jobname);
    pqxx::work tx{*connect_};
    pqxx::result result = tx.exec(querystr);
    if (result.empty()) {
        throw DBReadError("No job named " + jobname);
    }
    pqxx::row r = result[0];
    return JobData{r[0].as<std::string>(), r[1].as<std::string>()};
}

std::vector<JobData> DBReader::queryJobs(){
    std::string querystr = std::format("SELECT jobname, configfile FROM TrafficJobs");
    pqxx::work tx{*connect_};
    std::vector<JobData> data;
    for (auto [name, cfg] : tx.query<std::string, std::string>(querystr)){
        data.push_back({name, cfg});
    }
    return data;
}

// CAR METADATA

CarMetadata DBReader::queryCars(std::string jobname, int carid){
    pqxx::work tx{*connect_};

    std::string querystr = std::format("SELECT follow, lead FROM CarData INNER JOIN TrafficJobs ON TrafficJobs.JobID = CarData.JobID WHERE jobname = '{}' and carid = {}", jobname, carid);
    pqxx::result result = tx.exec(querystr);
    if (result.empty()) {
        throw DBReadError("No job named " + jobname);
    }
    pqxx::row r = result[0];
    return {r[0].as<std::string>(), r[1].as<std::string>(), {}, carid};
}

std::vector<CarMetadata> DBReader::queryCars(std::string jobname){
    pqxx::work tx{*connect_};
    std::string querystr = std::format("SELECT carid, follow, lead FROM CarData INNER JOIN TrafficJobs ON TrafficJobs.JobID = CarData.JobID WHERE jobname = '{}'", jobname);

    std::vector<CarMetadata> data;
    for (auto [carid, lead, follow] : tx.query<int, std::string, std::string>(querystr)){
        data.push_back({lead, follow, {}, carid});
    }
    return data;
}

// RAW DATA 

RawData DBReader::queryData(std::string jobname, int carid){
    std::string querystr = std::format("SELECT x, v, t FROM snapshotData INNER JOIN TrafficJobs ON TrafficJobs.JobID = snapshotData.jobid WHERE TrafficJobs.jobname = '{}' and snapshotData.carid = {} ORDER BY snapshotData.t ASC", jobname, carid);
    pqxx::work tx{*connect_};
    RawData raw;
    raw.id_ = carid;
    for (auto [x, v, t] : tx.query<float, float, float>(querystr)){
        raw.x_.push_back(x);
        raw.v_.push_back(v);
        raw.t_.push_back(t);
    }
    return raw;
}

std::vector<RawData> DBReader::queryData(std::string jobname){

    std::string querystr = std::format("SELECT carid, x, v, t FROM snapshotData INNER JOIN TrafficJobs ON TrafficJobs.JobID = snapshotData.jobid WHERE TrafficJobs.jobname = '{}' ORDER BY snapshotData.carid ASC, snapshotData.t ASC", jobname);

    pqxx::work tx{*connect_};

    // Holds a vector of snapshots for each car
    std::vector<RawData> alldata;
    alldata.push_back(RawData());
    alldata.back().id_ = 0;
    for (auto [id, x, v, t] : tx.query<int, float, float, float>(querystr)){
        if (id != alldata.back().id_){
            alldata.push_back(RawData());
            alldata.back().id_ = id;
        } // sorted by id, so new car data
        alldata.back().x_.push_back(x);
        alldata.back().v_.push_back(v);
        alldata.back().t_.push_back(t);
    }
    return alldata;
}

bool DBReader::checkJobName(std::string jobname) {
    pqxx::work tx{*connect_};
    std::string querystr = std::format("SELECT ('jobname', 'configfile')  FROM TrafficJobs WHERE jobname = '{}'", jobname);
    pqxx::result result = tx.exec(querystr);
    return result.empty();
}