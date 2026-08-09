// Regression test for Traffic Simulator
#include <gtest/gtest.h>
#include <expected>
#include <filesystem>
#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <ranges>
#include <charconv>
#include "sim/simulator.hpp"
#include "yaml-cpp/yaml.h"

#include "sim/parser.hpp"
#include "sim/parserFactory.hpp"
#include "sim/simulator.hpp"
#include "api/jobManager.hpp"
#include <pqxx/pqxx>
#include "testUtil.hpp"
#include <cstring>

#ifdef WITH_OPEN_SSL
    #include <openssl/evp.h>
#endif

struct XVTL{
    double x;
    double v;
    double t;
    int l;
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
        dbLog["jobname"] = "DB Test";
        dbLog["logtype"] = "test";

        YAML::Node tsLog = TestUtil::getConfigNode_3Lane();
        tsLog["jobname"] = "test-time-series";
        tsLog["logtype"] = "time-series";
        tsLog["logdir"] = "./file-test/time-series";

        TestUtil::configToFile(fileLog, "fileConfig.yaml");
        TestUtil::configToFile(dbLog, "dbConfig.yaml");
        TestUtil::configToFile(tsLog, "timeSeriesConfig.yaml");

        // Clear out the Test DB:
        TestUtil::clearDB();
    }

    // Note that these tests CANNOT be run in parallel 
    // since they are interacting with the SAME files. 
    void TearDown() override {
        if (std::filesystem::exists("fileConfig.yaml")) std::filesystem::remove("fileConfig.yaml");
        if (std::filesystem::exists("dbConfig.yaml")) std::filesystem::remove("dbConfig.yaml");
        if (std::filesystem::exists("timeSeriesConfig.yaml")) std::filesystem::remove("timeSeriesConfig.yaml");

        if (std::filesystem::exists("file-test/logs")) std::filesystem::remove_all("file-test/logs");
        if (std::filesystem::exists("file-test/time-series")) std::filesystem::remove_all("file-test/time-series");
    }

    void getXVTFromFIle(std::vector<XVTL>& xvts, std::filesystem::path file){
        std::string line;
        std::ifstream in(file);
        if (!in.good()){
            FAIL() << "Error opening file: " << strerror(errno);
        }
        std::getline(in, line); // eat the first header line
        while (std::getline(in, line)){
            std::vector<double> values;
            for (auto word : std::views::split(line, ',')) {
                std::string_view token{word};
                double r = -1;
                std::from_chars(token.begin(), token.begin() + token.size(), r);
                values.push_back(r);
            }
            if (values.size() < 4){
                FAIL() << "Not enough values found in file: " << file << " " << values.size() << " Line: " << line;
            } else {
                xvts.push_back({values[0], values[1], values[2], int(values[3])});
            }
        }
    }

    /**
     * @brief Converts a directory of time series data into a mapping of id -> XVTL. Stoers
     * 
     * @param directory 
     * @return std::map<size_t, std::vector<XVTL>>  id -> snapshots
     */
    void fromTimeSeries(std::filesystem::path directory, std::map<size_t, std::vector<XVTL>>& snapshots){
        snapshots.clear();
        for (std::filesystem::directory_entry file : std::filesystem::directory_iterator(directory)){
            std::string fname = file.path().filename().string();
            double t;
            try {
                t = std::stod(fname.substr(5, fname.size() - 3));
            }
            catch(const std::exception& e)
            {
                // Exclude the stats and car metadata csvs
                continue;
            }
            
            std::ifstream in(file.path());
            if (!in.good()){
                FAIL() << "Error opening file: " << strerror(errno);
            }

            std::string line;
            std::getline(in, line); // eat the first header line
            while (std::getline(in, line)){
                std::vector<double> values;
                for (auto word : std::views::split(line, ',')) {
                    std::string_view token{word};
                    double r = -1;
                    std::from_chars(token.begin(), token.begin() + token.size(), r);
                    values.push_back(r);
                }
                if (values.size() < 4){
                    FAIL() << "Not enough values found in file: " << file << " " << values.size() << " Line: " << line;
                } else {
                    size_t id(values[0]);
                    if (!snapshots.contains(id)){
                        snapshots.insert({id, {}});
                    }
                    snapshots[id].push_back({values[1], values[2], t, int(values[3])});
                }
            }
        }
    }

    static bool sortByTime(const XVTL& t1, const XVTL& t2){
        return t1.t < t2.t;
    };

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

    // Compare both file and DB, car by car at each timestamp. 

    size_t numCars  = std::distance(std::filesystem::directory_iterator("file-test/logs"), std::filesystem::directory_iterator{});
    numCars -= 2; // Car Stats and Simulation stats files aren't counted. 
    // Read in each file, query the DB for each specific car id. Then check if they are ASSERT_EQ
    pqxx::connection connect("host=localhost port=5432 dbname=trafficDBTest");
    for (size_t carid = 0; carid < numCars; ++carid){
        pqxx::work transaction(connect);
        // Job Id is always 1 because the DB is cleared
        std::string queryStr = std::format("SELECT x, v, t, lane FROM snapshotData WHERE (carid = {} AND jobid = 1)", carid);
        std::vector<XVTL> dbValues, fileValues;
        for (auto [x,v,t, l] : transaction.stream<float, float, float, int>(queryStr)){
            dbValues.push_back({x,v,t, l});
        }
        
        // File logging:
        std::filesystem::path p = std::format("file-test/logs/car{}.csv", carid);
        getXVTFromFIle(fileValues, p);
        // auto compareFunc = [](const XVTL& t1, const XVTL& t2){return t1.t < t2.t;};
        std::ranges::sort(dbValues, sortByTime);
        std::ranges::sort(fileValues, sortByTime);

        // Files are truncated to 4 places. 
        for (auto [db, file] : std::views::zip(dbValues, fileValues)){
            ASSERT_NEAR(db.x, file.x, 0.01);
            ASSERT_NEAR(db.x, file.x, 0.01);
            EXPECT_FLOAT_EQ(db.t, file.t);
            ASSERT_EQ(db.l, file.l);
        }
    }
}

TEST_F(RegressionTest, FileTimeSeriesEquivalence){
    ASSERT_TRUE(Traffic::Simulate("fileConfig.yaml").has_value());
    ASSERT_TRUE(Traffic::Simulate("timeSeriesConfig.yaml").has_value());

    // GetXVT from file gets all timestamps of data from all cars. 
    std::map<size_t, std::vector<XVTL>> timeSeries;
    fromTimeSeries("file-test/time-series", timeSeries);

    size_t numCars  = std::distance(std::filesystem::directory_iterator("file-test/logs"), std::filesystem::directory_iterator{});
    numCars -= 2; // Car Stats and Simulation stats files aren't counted. 
    ASSERT_EQ(timeSeries.size(), numCars);

    for (size_t carid = 0; carid < numCars; ++carid){
        std::filesystem::path p = std::format("file-test/logs/car{}.csv", carid);
        std::vector<XVTL> fileValues;
        getXVTFromFIle(fileValues, p);
        std::vector<XVTL>& timeSeriesValues = timeSeries[carid];

        std::ranges::sort(timeSeriesValues, sortByTime);
        std::ranges::sort(fileValues, sortByTime);

        for (auto [ts, file] : std::views::zip(timeSeriesValues, fileValues)){
            EXPECT_FLOAT_EQ(ts.x, file.x);
            EXPECT_FLOAT_EQ(ts.x, file.x);
            EXPECT_FLOAT_EQ(ts.t, file.t);
            ASSERT_EQ(ts.l, file.l);
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
    size_t numCars  = std::distance(std::filesystem::directory_iterator("file-test/logs"), std::filesystem::directory_iterator{}) - 2;
    std::string hashes;
    for (size_t carid = 0; carid < numCars; ++carid){
        std::filesystem::path p = std::format("file-test/logs/car{}.csv", carid);
        std::string hash = hashFile(p);
        hashes += hash;
    }
    std::string hash = hashBytes(hashes.data(),hashes.size());
   
    // The expected hash of the simulation is in "hash.txt"
    std::fstream in(std::string(HASH_FILE) + "/hash.txt");
    std::string expectedHash;
    std::getline(in, expectedHash);

    ASSERT_EQ(hash, expectedHash);
}
#endif
