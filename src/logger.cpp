/**
 * @file logger.cpp
 * @author Ryan Butler (rmbutler@outlook.com)
 * @brief  Implements the Car Logger and File Logger classes
 * @note Will be expanded to provide interface for a database
 * @version 0.1
 * @date 2025-07-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "logger.hpp"
#include <numeric>
#include <fstream>

namespace fs = std::filesystem;

void CarLogger::log(size_t id, double x, double v, double t) {
    logs_.push_back({id, x, v, t});
    cached = false;
}; 

std::vector<CarLog> CarLogger::getCar(size_t id){
    if (cached){
        return partitions_[id];
    } else {
        std::vector<CarLog> carSpecific;    
        auto logs =  std::copy_if(logs_.begin(), logs_.end(), std::back_inserter(carSpecific), [id](CarLog& c){return c.id == id;});
        return carSpecific;
    }
}

void CarLogger::clearLogs(){
    partitions_.clear();
    logs_.clear();
    cached = false;
}

std::vector<std::vector<CarLog>>& CarLogger::getPartition(){
    if (!cached){
        partition();
    }
    return partitions_;
}

void CarLogger::partition(size_t n) {
    if (n == 0){
        auto max = std::ranges::max_element(logs_, [](const CarLog& c1, const CarLog& c2){return c1.id < c2.id;});
        n = max->id;
    }
    // Resize and clear partitions
    partitions_.resize(n+1);
    for (auto& v : partitions_){
        v.clear();
    }

    // Split all logs by the car.  
    for (CarLog& c : logs_){
        partitions_[c.id].push_back(c);
    }

    // Sort by timestamp
    for (std::vector<CarLog>& log : partitions_){
        std::ranges::sort(log, [](const CarLog& c1, const CarLog& c2){return c1.t < c2.t;});
    }
    cached = true;
}

FileLogger::FileLogger(std::string basepath):basepath_{basepath}{
    // create the directory if it doesn't exist and clear it out if it does
    if (fs::exists(basepath_)){
        fs::remove_all(basepath);
    }
    fs::create_directory(basepath_);
}

void FileLogger::write(){

    std::vector<std::vector<CarLog>> byCar = getPartition();
    size_t n = byCar.size();

    for (size_t i = 0; i < n; ++i){
        // If file doesn't exist, make it
        fs::path fname = basepath_ / fs::path("car" + std::to_string(i) + ".csv");
        if (!fs::exists(fname)){
            std::ofstream out(fname);
            out << "x,v,t\n";
        }
        
        std::ofstream logfile(fname, std::ios::app);
        for (CarLog& c : byCar[i]){
            logfile << c.x << "," << c.v << ","<< c.t<<"\n";
        }
    }
    clearLogs();
}

