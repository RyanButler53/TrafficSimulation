/**
 * @file logger.cpp
 * @author Ryan Butler (rmbutler@outlook.com)
 * @brief  Implements the Car Logger Base class, File Logger and Database Logger
 * @note Relies on libpqxx and libpostgres
 * @version 0.2
 * @date 2025-07-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "sim/logger.hpp"
#include <numeric>
#include <fstream>
#include <chrono>
#include <format>
#include <iostream>

// Database
#include <pqxx/pqxx>
#include "database/databaseInit.hpp"

namespace fs = std::filesystem;

void CarLogger::log(size_t id, double x, double v, double t) {
    logs_.push_back({id, x, v, t});
    cached = false;
}; 
void CarLogger::addCar(size_t id, const std::tuple<double, double, double>& follow){
    cars_.push_back({std::get<0>(follow), std::get<1>(follow),std::get<2>(follow), id});
}

std::vector<CarSnapshot> CarLogger::getCar(size_t id){
    if (cached){
        return partitions_[id];
    } else {
        std::vector<CarSnapshot> carSpecific;    
        auto logs =  std::copy_if(logs_.begin(), logs_.end(), std::back_inserter(carSpecific), [id](CarSnapshot& c){return c.id == id;});
        return carSpecific;
    }
}

void CarLogger::clearLogs(){
    partitions_.clear();
    logs_.clear();
    cached = false;
}

std::vector<std::vector<CarSnapshot>>& CarLogger::getPartition(){
    if (!cached){
        partition();
    }
    return partitions_;
}

void CarLogger::partition(size_t n) {
    if (n == 0){
        auto max = std::ranges::max_element(logs_, [](const CarSnapshot& c1, const CarSnapshot& c2){return c1.id < c2.id;});
        n = max->id;
    }
    // Resize and clear partitions
    partitions_.resize(n+1);
    for (auto& v : partitions_){
        v.clear();
    }

    // Split all logs by the car.  
    for (CarSnapshot& c : logs_){
        partitions_[c.id].push_back(c);
    }

    // Sort by timestamp
    for (std::vector<CarSnapshot>& log : partitions_){
        std::ranges::sort(log, [](const CarSnapshot& c1, const CarSnapshot& c2){return c1.t < c2.t;});
    }
    cached = true;
}


// FILE LOGGER

FileLogger::FileLogger(std::string basepath):basepath_{basepath}{
    // create the directory if it doesn't exist and clear it out if it does
    if (fs::exists(basepath_)){
        fs::remove_all(basepath);
    }
    fs::create_directories(basepath_);
}

std::expected<void, std::string> FileLogger::writeData(){

    std::vector<std::vector<CarSnapshot>> byCar = getPartition();
    size_t n = byCar.size();

    for (size_t i = 0; i < n; ++i){
        // If file doesn't exist, make it
        fs::path fname = basepath_ / fs::path("car" + std::to_string(i) + ".csv");
        if (!fs::exists(fname)){
            std::ofstream out(fname);
            out << "x,v,t\n";
        }
        
        std::ofstream logfile(fname, std::ios::app);
        for (CarSnapshot& c : byCar[i]){
            logfile << c.x << "," << c.v << ","<< c.t<<"\n";
        }
    }
    clearLogs();
    return {};
}

std::expected<void, std::string> FileLogger::logFailure(std::string message) {
    std::ofstream errorOut(basepath_ / fs::path("error.txt"));
    errorOut << "Job failed: " << message << std::endl;
    errorOut.close();
    return {};
}

// DATABASE LOGGER

DBLogger::DBLogger(std::string jobname, std::string config, bool test):
    jobname_{jobname}, configFile_{std::filesystem::absolute(config)}{
    if (test){
        connectionStr_ = "host=localhost port=5432 dbname=trafficDBTest";
    } else {
        connectionStr_ = "host=localhost port=5432 dbname=trafficDB";
    }
}

std::expected<std::shared_ptr<DBLogger>, std::string> DBLogger::make(std::string jobname, std::string config, std::string followType, bool test){
    DBLogger* logger = new DBLogger(jobname, config, test);
    try {

        initDB::initDB(test);

        pqxx::connection connect(logger->connectionStr_);
        pqxx::work tx(connect);
       

        std::string row = std::format("INSERT INTO trafficJobs (configfile, jobname, status, error, followModel, numCars)\nVALUES ('{}', '{}', 'QUEUED', '', '{}', 0) RETURNING jobID", config, jobname, followType);
        pqxx::result result = tx.exec(row);
        logger->jobid_ = result.one_field().as<int>();
        tx.commit();
    } catch(const std::exception& e) {
        std::unexpected(std::format("Error setting up database: {}",e.what()));
    }
    return std::shared_ptr<DBLogger>(logger);
}

 std::expected<void, std::string> DBLogger::writeData() {

    std::vector<std::vector<CarSnapshot>> byCar = getPartition();

    pqxx::connection connect(connectionStr_);

    // Add rows for all the new cars seen. This breaks when splitting up writing into 2 or more steps
    try {
        pqxx::work tx(connect);
        for (CarData& cdata : cars_){
            tx.exec(std::format("INSERT INTO carData (carid, jobid, follow_a, follow_b, follow_c)\nVALUES ({}, {}, {}, {}, {})", cdata.id, jobid_, cdata.a, cdata.b, cdata.c));
        }
        tx.commit();
    } catch(const std::exception& e) {
        return std::unexpected(std::format("Error inserting car info data into database", e.what()));
    }
    
    // Update the big data table
    for (std::vector<CarSnapshot>& car : byCar){
        if (car.empty()) continue;
        try {
            pqxx::work car_transaction(connect);

            std::string logstr;
            for (CarSnapshot& log : car){
                logstr = std::format("INSERT INTO snapshotData (jobid, carid, x, v, t)\nVALUES ({}, {}, {}, {}, {})", jobid_, log.id, log.x, log.v, log.t);
                car_transaction.exec(logstr);
            }
            car_transaction.commit();
        } catch(const std::exception& e) {
            return std::unexpected(std::format("Error inserting car raw snapshot data into database", e.what()));
        }
    }

    // Update the number of cars.  This will break if writing to the DB is done in chunks. Number of Unique cars is the number of rows found in car metadata
    try {
        pqxx::connection connect(connectionStr_);
        pqxx::work finish_tx(connect);

        std::string updateStatus = std::format("UPDATE ONLY trafficJobs SET numCars = {} WHERE jobid = '{}'", byCar.size(), jobid_);
        finish_tx.exec(updateStatus); 
        finish_tx.commit();
        return {};
    } catch(const std::exception& e) {
        return std::unexpected(std::format("Error updating the Number of Cars: {} ", e.what()));
    }

    clearLogs();
    return {};
}

std::expected<void, std::string> DBLogger::updateStatus(std::string newStatus) {
    try {
        pqxx::connection connect(connectionStr_);
        pqxx::work finish_tx(connect);

        std::string updateStatus = std::format("UPDATE ONLY trafficJobs SET status = '{}' WHERE jobid = '{}'", newStatus, jobid_);
        finish_tx.exec(updateStatus); 
        finish_tx.commit();
        return {};
    } catch(const std::exception& e) {
        return std::unexpected(std::format("Error updating the Job Status: {} ", e.what()));
    }
}

std::expected<void, std::string> DBLogger::logFailure(std::string message) {

    return updateStatus("ERROR").and_then([this, &message]() -> std::expected<void, std::string>{
            try {
                pqxx::connection connect(connectionStr_);
                pqxx::work finish_tx(connect);
        
                std::string updateMsg = std::format("UPDATE ONLY trafficJobs SET error = '{}' WHERE jobid = '{}'", message, jobid_);
                finish_tx.exec(updateMsg);
                finish_tx.commit();
                return {};
            } catch(const std::exception& e) {
                return std::unexpected(std::format("Error updating the erroe message {} ", e.what()));
            }
        });
   
}