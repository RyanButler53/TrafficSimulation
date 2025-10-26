// Regression test for Traffic Simulator
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <ranges>
#include "sim/simulator.hpp"
#include "yaml-cpp/yaml.h"

#include "sim/parser.hpp"
#include "sim/parserFactory.hpp"
#include "sim/simulator.hpp"
#include "api/jobManager.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>

#include <pqxx/pqxx>

#ifdef WITH_OPEN_SSL
    #include <openssl/sha.h>
#endif

struct XVT{
    double x;
    double v;
    double t;
    
};

class RegressionTest : public ::testing::Test {
private:
    /* data */
    // Gets the config node except for the log dir/log type fields. 
    YAML::Node getConfigNode() {
        YAML::Node cfg;
        cfg["jobname"] = "test-file";
        cfg["type"] = "continuous";
        cfg["time"] = 200;
        cfg["timestep"] = 1;
        cfg["seed"] = 70;

        // Driver params (From traffic flow book example)
        cfg["driverType"] = "Gipps";
        cfg["driverParams"]["a"] = 1.981;
        cfg["driverParams"]["b"] = -2.8955;
        cfg["driverParams"]["bmax"] = -5.505;
        cfg["driverParams"]["a_stdev"] = 0.1;
        cfg["driverParams"]["a_stdev"] = 0.1;
        cfg["driverParams"]["bmax_stdev"] = 0.1;

        cfg["flow"]["rate"] = 600;
        cfg["flow"]["v0"] = 30;
        cfg["flow"]["vdes"] = 35;

        cfg["leadCar"]["leadType"] = "function";
        cfg["leadCar"]["function"]["sine"]["a"] = 5.0;
        cfg["leadCar"]["function"]["sine"]["b"] = 0.04;
        cfg["leadCar"]["function"]["sine"]["c"] = 40;
        cfg["leadCar"]["v0"] = 40;
        cfg["leadCar"]["vdes"] = 45;

        cfg["end"] = 1500;
        return cfg;
    }

protected:

    void SetUp() override {
        // Set up file case
        YAML::Node fileLog = getConfigNode();
        fileLog["logtype"] = "file";
        fileLog["logdir"] = "./file-test/logs";

        YAML::Node dbLog = getConfigNode();
        dbLog["logtype"] = "test";

        YAML::Emitter fileout;
        std::ofstream fileCfg("fileConfig.yaml");
        fileout << fileLog;
        fileCfg << fileout.c_str();

        YAML::Emitter dbout;
        std::ofstream dbCfg("dbConfig.yaml");
        dbout << dbLog;
        dbCfg << dbout.c_str();

        // Clear out the Test DB:
        pqxx::connection connect("host=localhost port=5432 dbname=trafficDBTest");
        pqxx::work tx(connect);

        tx.exec("DROP TABLE IF EXISTS trafficjobs CASCADE");
        tx.exec("DROP TABLE IF EXISTS cardata CASCADE");
        tx.exec("DROP TABLE IF EXISTS snapshotData");
        tx.commit();
    }

    // Note that these tests CANNOT be run in parallel 
    // since they are interacting with the SAME files. 
    void TearDown() override {
        if (std::filesystem::exists("fileConfig.yaml")) std::filesystem::remove("fileConfig.yaml");
        if (std::filesystem::exists("dbConfig.yaml")) std::filesystem::remove("dbConfig.yaml");

        if (std::filesystem::exists("file-test/logs")) std::filesystem::remove_all("file-test/logs");
    }

    void getXVTFromFIle(std::vector<XVT>& xvts, std::filesystem::path file){
        std::string line;
        std::ifstream in(file);
        std::getline(in, line); // eat the first header line
        std::vector<XVT> snapshots;
        while (std::getline(in, line)){
            size_t firstComma = std::find(line.begin(), line.end(), ',') - line.begin();
            size_t secondComma = std::find(line.begin() + firstComma + 1, line.end(), ',') - line.begin();
            double x = std::stod(line.substr(0, firstComma-1));
            double v = std::stod(line.substr(firstComma+1, secondComma - firstComma - 1));
            double t = std::stod(line.substr(secondComma+1));
            xvts.push_back({x,v,t});
        }
    }

    #ifdef WITH_OPEN_SSL
    std::string hashFile(std::filesystem::path filename){
        size_t fsize = std::filesystem::file_size(filename);
        std::vector<char> filedata(fsize);
        // Treat it like a binary file
        std::fstream in(filename, std::ios::binary);
        in.read(filedata.data(), fsize);
        return hashBytes(filedata.data(), fsize);
        
    }

    std::string hashBytes(char* data, size_t size){
        unsigned char hash[SHA256_DIGEST_LENGTH];

        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, data, size);
        SHA256_Final(hash, &sha256);
        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    #endif
};

TEST_F(RegressionTest, FileDBEquivalence){
    Traffic::Simulate("fileConfig.yaml");
    Traffic::Simulate("dbConfig.yaml");

    // Need to compare both of them, car by car at each timestamp. 

    size_t numCars  = std::distance(std::filesystem::directory_iterator("file-test/logs"), std::filesystem::directory_iterator{});
    // Read in each file, query the DB for each specific car id. Then check if they are ASSERT_EQ
    
    pqxx::connection connect("host=localhost port=5432 dbname=trafficDBTest");
    for (size_t carid = 0; carid < numCars; ++carid){
        pqxx::work transaction(connect);
        // Job Id is always 1 because the DB is cleared
        std::string queryStr = std::format("SELECT x, v, t FROM snapshotData WHERE (carid = {} AND jobid = 1)", carid);
        std::vector<XVT> dbValues, fileValues;
        for (auto [x,v,t] : transaction.stream<float, float, float>(queryStr)){
            dbValues.push_back({x,v,t});
        }
        std::filesystem::path carFile = std::format("file-test/logs/car{}.log", carid);
        
        // File logging:
        std::filesystem::path p = std::format("file=test/logs/car{}.csv", carid);
        getXVTFromFIle(fileValues, p);
        auto compareFunc = [](const XVT& t1, const XVT& t2){return t1.t < t2.t;};
        std::ranges::sort(dbValues, compareFunc);
        std::ranges::sort(fileValues, compareFunc);

        for (auto [db, file] : std::views::zip(dbValues, fileValues)){
            ASSERT_NEAR(db.x, file.x, 0.01);
            ASSERT_NEAR(db.x, file.x, 0.01);
            ASSERT_NEAR(db.t, file.t, 0.01);
        }
    }
}
#ifdef WITH_OPEN_SSL
TEST_F(RegressionTest, FileHashEquivalence){
    JobManager j;
    uint32_t id = j.submit("fileConfig.yaml");

    // Need to wait for the job to be done
    while (j.status(id) != JobStatus::DONE){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Get the hashes for each file and concatenate them
    size_t numCars  = std::distance(std::filesystem::directory_iterator("file-test/logs"), std::filesystem::directory_iterator{});
    std::string hashes;
    for (size_t carid = 0; carid < numCars; ++carid){
        std::filesystem::path p = std::format("file-test/logs/car{}.csv", carid);
        std::string hash = hashFile(p);
        hashes += hash;
    }
    std::string hash = hashBytes(hashes.data(),hashes.size());
    const std::string expectedHash("b908503a9209ad2bd547e68259e37664123c252316a6f2cf2c3f09aed7487df4");

    ASSERT_EQ(hash, expectedHash);
}
#endif
