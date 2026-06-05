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
#include "testUtil.hpp"

#ifdef WITH_OPEN_SSL
    #include <openssl/evp.h>
#endif

struct XVT{
    double x;
    double v;
    double t;
    
};

class RegressionTest : public ::testing::Test {

protected:

    void SetUp() override {
        // Set up file case
        YAML::Node fileLog = TestUtil::getConfigNode_3Lane();
        fileLog["jobname"] = "test-file";
        fileLog["logtype"] = "file";
        fileLog["logdir"] = "./file-test/logs";

        YAML::Node dbLog = TestUtil::getConfigNode_3Lane();
        dbLog["logtype"] = "test";
        dbLog["jobname"] = "DB Test";


        YAML::Emitter fileout;
        std::ofstream fileCfg("fileConfig.yaml");
        fileout << fileLog;
        fileCfg << fileout.c_str();

        YAML::Emitter dbout;
        std::ofstream dbCfg("dbConfig.yaml");
        dbout << dbLog;
        dbCfg << dbout.c_str();

        // Clear out the Test DB:
        TestUtil::clearDB();
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
    const std::string expectedHash("4174313eb6b4a8b59d1163c9b2e32e6cf2e525bb97a45ac8074d7a63977641cd");
    ASSERT_EQ(hash, expectedHash);
}
#endif
