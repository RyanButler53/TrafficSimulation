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

DBReader::DBReader(){
    try {   
        connect_ = std::make_shared<pqxx::connection>("host=localhost port=5432 dbname=trafficDB");
    } catch(const std::exception& e) {
        std::cout << "Error connecting to the database" << std::endl;
        std::cerr << e.what() << '\n';
    }
    if (!connect_){
        throw DBReadError("Could not connect to the database!");
    }
    
}

JobData DBReader::queryJobs(std::string jobname){
    std::string querystr = std::format("SELECT ('jobname', 'configfile')  FROM TrafficJobs WHERE jobname  = {}", jobname);
    pqxx::work tx{*connect_};
    pqxx::result result = tx.exec(querystr);
    if (result.empty()) {
        throw DBReadError("No job named " + jobname);
    }
    pqxx::row r = result[0];
    return JobData{r[0].as<std::string>(), r[1].as<std::string>()};
}

std::vector<JobData> DBReader::queryJobs(){
    std::string querystr = std::format("SELECT (jobname, configfile) FROM TrafficJobs");
    pqxx::work tx{*connect_};
    std::vector<JobData> data;
    for (auto [name, cfg] : tx.stream<std::string, std::string>(querystr)){
        data.push_back({name, cfg});
    }
    return data;
}

CarMetadata DBReader::queryCars(std::string jobname, int carid){
    std::string querystr = std::format("SELECT follow, lead FROM CarData WHERE (jobname = {} AND carid = {})", jobname, carid);
    pqxx::work tx{*connect_};
    pqxx::result result = tx.exec(querystr);
    if (result.empty()) {
        throw DBReadError("No job named " + jobname);
    }
    pqxx::row r = result[0];
    return {r[0].as<std::string>(), r[1].as<std::string>(), {}, carid};
}

std::vector<CarMetadata> DBReader::queryCars(std::string jobname){
    std::string querystr = std::format("SELECT follow, lead FROM CarData WHERE jobname = {}", jobname);
    pqxx::work tx{*connect_};

    std::vector<CarMetadata> data;
    for (auto [lead, follow] : tx.stream<std::string, std::string>(querystr)){
        data.push_back({lead, follow, {}});
    }
    return data;
}

RawData DBReader::queryData(std::string jobname, int carid){
    std::string querystr = std::format("SELECT x, v, t FROM snapshotData WHERE (jobname = {} AND carid = {})", jobname, carid);
    pqxx::work tx{*connect_};
    pqxx::result result = tx.exec(querystr);
    if (result.empty()) {
        throw DBReadError(std::format("No car in job {} with carid {}", jobname, carid));
    }
    // Expecting MANY rows. 
    RawData raw;
    raw.id_ = carid;
    for (auto [x, v, t] : tx.stream<float, float, float>(querystr)){
        raw.x_.push_back(x);
        raw.v_.push_back(v);
        raw.t_.push_back(t);
    }
    return raw;
}

std::vector<RawData> DBReader::queryData(std::string jobname){
    std::string querystr = std::format("SELECT id, x, v, t FROM snapshotData WHERE jobname = {} ORDER BY id ASC, t ASC", jobname);
    pqxx::work tx{*connect_};

    // Holds a vector of snapshots for each car
    std::vector<RawData> alldata;
    alldata.push_back(RawData());
    alldata.back().id_ = 0;
    for (auto [id, x, v, t] : tx.stream<int, float, float, float>(querystr)){
        if (id != alldata.back().id_){alldata.push_back(RawData());} // sorted by id, so new car data
        alldata.back().x_.push_back(x);
        alldata.back().v_.push_back(v);
        alldata.back().t_.push_back(t);
    }
    return alldata;
}

bool DBReader::checkJobName(std::string jobname) {
    pqxx::work tx{*connect_};
    std::string querystr = std::format("SELECT ('jobname', 'configfile')  FROM TrafficJobs WHERE jobname = {}", jobname);
    pqxx::result result = tx.exec(querystr);
    return result.empty();
}