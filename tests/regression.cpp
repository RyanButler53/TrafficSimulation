// Regression test for Traffic Simulator
#include <gtest/gtest.h>
#include <expected>
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
    #include <openssl/evp.h>
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
        cfg["driverParams"]["a_stdev"] = 0;
        cfg["driverParams"]["b_stdev"] = 0;
        cfg["driverParams"]["bmax_stdev"] = 0;

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
        // if (std::filesystem::exists("fileConfig.yaml")) std::filesystem::remove("fileConfig.yaml");
        // if (std::filesystem::exists("dbConfig.yaml")) std::filesystem::remove("dbConfig.yaml");

        // if (std::filesystem::exists("file-test/logs")) std::filesystem::remove_all("file-test/logs");
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
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int numHashedBytes;
        EVP_MD_CTX* mdctx = EVP_MD_CTX_new();

        if (!EVP_DigestInit_ex2(mdctx, EVP_sha256(), NULL)){
            EVP_MD_CTX_free(mdctx);
            throw std::logic_error("Error initializing Hash function");
        }

        if(!EVP_DigestUpdate(mdctx, data, size)) {
            EVP_MD_CTX_free(mdctx);
            throw std::logic_error("Error updating Hash");
        }
        if (!EVP_DigestFinal_ex(mdctx, hash, &numHashedBytes)) {
            printf("Message digest finalization failed.\n");
            EVP_MD_CTX_free(mdctx);
            throw std::logic_error("Hashing Error");
        }

        std::stringstream ss;
        for(int i = 0; i < numHashedBytes; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }

        EVP_MD_CTX_free(mdctx);

        return ss.str();
    }
    #endif
};

TEST_F(RegressionTest, FileDBEquivalence){
    ASSERT_TRUE(Traffic::Simulate("fileConfig.yaml").has_value());
    ASSERT_TRUE(Traffic::Simulate("dbConfig.yaml").has_value());

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
    std::expected<uint32_t, std::string> id = j.submit("fileConfig.yaml");
    ASSERT_TRUE(id.has_value()) << id.error();
    // Need to wait for the job to be done
    while (j.status(*id) != JobStatus::DONE){
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
    const std::string expectedHash("d3e4767ae4085484ecb21d35a1aa915e1e0ba8c5dce2f03cf74d607f2dbe41fc");

    ASSERT_EQ(hash, expectedHash);
}
#endif
