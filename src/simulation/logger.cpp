/**
 * @file logger.cpp
 * @author Ryan Butler (rmbutler@outlook.com)
 * @brief  Implements the Car Logger Base class, File Logger and Database Logger
 * @note Relies on libpqxx and libpostgres
 * @version 0.1
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

namespace fs = std::filesystem;

void CarLogger::log(size_t id, double x, double v, double t) {
    logs_.push_back({id, x, v, t});
    cached = false;
}; 
void CarLogger::addCar(size_t id, std::string lead, std::string follow){
    cars_.push_back({lead, follow, id});
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
    fs::create_directory(basepath_);
}

void FileLogger::writeData(){

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
}

// DATABASE LOGGER

DBLogger::DBLogger(std::string jobname, std::string config):
    jobname_{jobname}, configFile_{std::filesystem::absolute(config)}{

    // Initialize Database connection: 
    pqxx::connection connect("host=localhost port=5432 dbname=trafficDB");

    // Create 3 tables: Traffic Jobs Main Table, Car Data Table (including strategies), Main data table (with car x,v,t values)
    pqxx::work tx(connect);
    tx.exec("CREATE TABLE IF NOT EXISTS trafficJobs ( jobID int GENERATED ALWAYS AS IDENTITY PRIMARY KEY, configfile text, jobname text)");
    tx.exec("CREATE TABLE IF NOT EXISTS carData (carID int, jobID int, follow text, lead text, FOREIGN KEY (jobID) REFERENCES trafficjobs(jobID) , PRIMARY KEY (carID, jobID))");
    tx.exec("CREATE TABLE IF NOT EXISTS snapshotData (carID int, jobID int, x float, v float, t float, PRIMARY KEY (carID, jobID, t), FOREIGN KEY (carID, jobID) REFERENCES cardata (carID, jobID))");



    // Add row for the job

    std::string row = std::format("INSERT INTO trafficJobs (configfile, jobname)\nVALUES ('{}', '{}') RETURNING jobID", configFile_, jobname_);
    pqxx::result result = tx.exec(row);
    jobid_ = result.one_field().as<int>();
    std::cout << "Creted logger for job " << jobid_ << std::endl;
    tx.commit();

    std::cout << "commited to the DB" << std::endl;

}

// CREATE TABLE trafficJobs ( jobID int GENERATED ALWAYS AS IDENTITY PRIMARY KEY, configfile text, jobname text );
// CREATE TABLE carData (carID int, jobID int, follow text, lead text, FOREIGN KEY (jobID) REFERENCES trafficjobs(jobID) , PRIMARY KEY (carID, jobID));
// CREATE TABLE snapshotData (carID int, jobID int, x float, v float, t float, PRIMARY KEY (carID, jobID, t), FOREIGN KEY (carID, jobID) REFERENCES cardata (carID, jobID));
 void DBLogger::writeData() {

    std::vector<std::vector<CarSnapshot>> byCar = getPartition();
    size_t n = byCar.size();
    std::cout << "Writing to separate database " << std::endl;
    pqxx::connection connect("host=localhost port=5432 dbname=trafficDB");

    // Add rows for all the new cars seen. 
    pqxx::work tx(connect);
    for (CarData& cdata : cars_){
        tx.exec(std::format("INSERT INTO cardata (carid, jobid, follow, lead)\nVALUES ({}, {}, '{}', '{}')", cdata.id, jobid_, cdata.followStrategy, cdata.leadStrategy));
    }
    tx.commit();

    // Update the big data table
    for (std::vector<CarSnapshot>& car : byCar){
        if (car.empty()) continue;
        pqxx::work car_transaction(connect);

        std::string logstr;
        for (CarSnapshot& log : car){
            logstr = std::format("INSERT INTO snapshotData (jobid, carid, x, v, t)\nVALUES ({}, {}, {}, {}, {})", jobid_, log.id, log.x, log.v, log.t);
            car_transaction.exec(logstr);
        }
        car_transaction.commit();
    }
}