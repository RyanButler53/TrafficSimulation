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
#include <ranges>

// Database
#include <pqxx/pqxx>
#include "database/databaseInit.hpp"

namespace fs = std::filesystem;


void CarLogger::partition(std::vector<CarSnapshot>&& snapshots, std::unordered_map<size_t, std::vector<CarSnapshot>>& partitions){
    for (CarSnapshot& snapshot : snapshots){
        size_t id = snapshot.id;
        if (!partitions.contains(id)){
            partitions[id] = {};
        }
        partitions[id].push_back(std::move(snapshot));
    }
    for (auto& [_, logs] : partitions){
        std::ranges::sort(logs, [](const CarSnapshot& c1, const CarSnapshot& c2){return c1.t < c2.t;});
    }
}

// Main loop
void CarLogger::run(CommunicationsManager& comms){
    DataType messageType = DataType::NO_DATA;
    while (messageType != DataType::END_OF_DATA){
        DataPacket::ptr packet = comms.getPacket();
        if (auto pkt = std::dynamic_pointer_cast<CarMetadataPacket>(packet)){
            messageType = DataType::CAR_DATA;
            writeCars(pkt->moveData());
        } else if (auto pkt = std::dynamic_pointer_cast<CarSnapshotPacket>(packet)){
            messageType = DataType::SNAPSHOT_DATA;
            writeSnapshots(pkt->moveData());
        } else if (auto pkt = std::dynamic_pointer_cast<EndOfData>(packet)){
            messageType = DataType::END_OF_DATA;
        }
    }
}

// FILE LOGGER

FileLogger::FileLogger(std::string basepath):basepath_{basepath}{
    // create the directory if it doesn't exist and clear it out if it does
    if (fs::exists(basepath_)){
        fs::remove_all(basepath);
    }
    fs::create_directories(basepath_);
}

std::expected<void, std::string> FileLogger::writeSnapshots(std::vector<CarSnapshot> snapshots){

    std::unordered_map<size_t, std::vector<CarSnapshot>> byCar;
    partition(std::move(snapshots), byCar);
    size_t n = byCar.size();
    for (const auto& [i, cars] : byCar){
        // If file doesn't exist, make it
        fs::path fname = basepath_ / fs::path("car" + std::to_string(i) + ".csv");
        if (!fs::exists(fname)){
            std::ofstream out(fname);
            out << "x,v,t,l\n";
        }
        
        std::ofstream logfile(fname, std::ios::app);
        for (const CarSnapshot& c : cars){
            logfile << c.x << "," << c.v << ","<< c.t << "," << c.l<<"\n";
        }
        logfile.close();
    }
    return {};
}

std::expected<void, std::string> FileLogger::writeCars(std::vector<CarData> data){
    fs::path fname = basepath_ / fs::path("car_stats.csv");
    std::ranges::sort(data, [](const CarData& c1, const CarData& c2){return c1.id < c2.id;});
    if(!fs::exists(fname)){
        std::ofstream out(fname);
        out << "id,a,b,c,p\n";
    }

    std::ofstream logfile(fname, std::ios::app);
    for (const CarData& c : data){
        logfile << c.id << "," << c.a << ","<< c.b << "," << c.c<< "," << c.p <<"\n";
    }
    return {};
}

std::expected<void, std::string> FileLogger::logFailure(std::string message) {
    std::ofstream errorOut(basepath_ / fs::path("error.txt"));
    errorOut << "Job failed: " << message << std::endl;
    errorOut.close();
    return {};
}

std::expected<void, std::string> FileLogger::writeStats(SimulationStats s) {
    std::ofstream statsOut(basepath_ / fs::path("stats.txt"));
    statsOut << "Runtime: " << s.runtime_ << std::endl;
    statsOut.close();
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
    auto init = initDB::initDB(test);
    if (!init){return std::unexpected("Error initializing database: " + init.error());}
    try {

        pqxx::connection connect(logger->connectionStr_);
        pqxx::work tx(connect);
       
        // Read in entire config file (1KB) into memory and store in database
        std::ifstream cfgin(config);
        size_t n = std::filesystem::file_size(config);
        std::string inputfile;
        inputfile.resize(n);
        cfgin.read(inputfile.data(), n);

        std::string row = std::format("INSERT INTO trafficJobs (configfile, jobname, status, error, followModel, numCars)\nVALUES ('{}', '{}', 'QUEUED', '', '{}', 0) RETURNING jobID", inputfile, jobname, followType);
        pqxx::result result = tx.exec(row);
        logger->jobid_ = result.one_field().as<int>();
        tx.commit();
    } catch(const std::exception& e) {
        std::unexpected(std::format("Error setting up database: {}",e.what()));
    }
    return std::shared_ptr<DBLogger>(logger);
}

 std::expected<void, std::string> DBLogger::writeSnapshots(std::vector<CarSnapshot> snapshots) {

    if (snapshots.empty()){
        return std::unexpected("No cars!");
    }

    std::unordered_map<size_t, std::vector<CarSnapshot>> byCar;
    partition(std::move(snapshots), byCar);

    pqxx::connection connect(connectionStr_);

    // Update the big data table
    for (auto& [id, car] : byCar){
        if (car.empty()) continue;
        if (id != car[0].id){
            return std::unexpected("Partition Mismatch: Car id does not match partition id");
        }
        try {
            pqxx::work car_transaction(connect);

            std::string logstr;
            for (CarSnapshot& log : car){
                logstr = std::format("INSERT INTO snapshotData (jobid, carid, x, v, t, lane)\nVALUES ({}, {}, {}, {}, {}, {})", jobid_, log.id, log.x, log.v, log.t, log.l);
                car_transaction.exec(logstr);
            }
            car_transaction.commit();
        } catch(const std::exception& e) {
            return std::unexpected(std::format("Error inserting car raw snapshot data into database: {}", e.what()));
        }
    }


    return {};
}

std::expected<void, std::string> DBLogger::writeCars(std::vector<CarData> cars) {
    pqxx::connection connect(connectionStr_);
    // Add rows for all the new cars seen. This breaks when splitting up writing into 2 or more steps
    try {
        pqxx::work tx(connect);
        for (CarData& cdata : cars){
            tx.exec(std::format("INSERT INTO carData (carid, jobid, follow_a, follow_b, follow_c, politeness)\nVALUES ({}, {}, {}, {}, {}, {})", cdata.id, jobid_, cdata.a, cdata.b, cdata.c, cdata.p));
        }
        tx.commit();
    } catch(const std::exception& e) {
        return std::unexpected(std::format("Error inserting car info data into database: {}", e.what()));
    }

    // Update the number of cars.  This will break if writing to the DB is done in chunks. Number of Unique cars is the number of rows found in car metadata
    nCars_ += cars.size();
    return updateField("numCars", nCars_, "Number of Cars");
}

template <typename T>
std::expected<void, std::string> DBLogger::updateField(std::string key, T value, std::string errMsg){
    try {
        pqxx::connection connect(connectionStr_);
        pqxx::work finish_tx(connect);

        std::string updateStatus = std::format("UPDATE ONLY trafficJobs SET {} = '{}' WHERE jobid = '{}'", key, value, jobid_);
        finish_tx.exec(updateStatus); 
        finish_tx.commit();
        return {};
    } catch(const std::exception& e) {
        return std::unexpected(std::format("Error updating the {}: {} ", errMsg, e.what()));
    }
}

std::expected<void, std::string> DBLogger::updateStatus(std::string newStatus) {
    return updateField<std::string>("status", newStatus, "Job Status");
}

std::expected<void, std::string> DBLogger::writeStats(SimulationStats s) {
    return updateField("runtime", s.runtime_, "simulator stats");
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
                return std::unexpected(std::format("Error updating the error message {} ", e.what()));
            }
        });
}

// Template Instantiations
template std::expected<void, std::string> DBLogger::updateField(std::string, size_t, std::string);
template std::expected<void, std::string> DBLogger::updateField(std::string, std::string, std::string);
template std::expected<void, std::string> DBLogger::updateField(std::string, double, std::string);

