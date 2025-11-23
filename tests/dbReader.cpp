#include <gtest/gtest.h>
#include <fstream>
#include <algorithm>

#include "api/DBReader.hpp"
#include "api/jobManager.hpp"
#include "api/structs.hpp"
#include "yaml-cpp/yaml.h"

#include <pqxx/pqxx>

class DBReaderTest : public ::testing::Test {

    YAML::Node getConfigNode() {
        YAML::Node cfg;
        cfg["type"] = "continuous";
        cfg["time"] = 15;
        cfg["timestep"] = 1;

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

    void SetUp() override {

        for (size_t i = 0; i < 3; ++i){
            YAML::Node dbLog = getConfigNode();
            dbLog["logtype"] = "test";
            dbLog["jobname"] = std::format("test-dbreader{}", i);
            dbLog["seed"] = 70 + i;

            YAML::Emitter dbout;
            std::ofstream dbCfg(std::format("dbConfig{}.yaml", i));
            dbout << dbLog;
            dbCfg << dbout.c_str();
        }

        // Clear out the Test DB:
        pqxx::connection connect("host=localhost port=5432 dbname=trafficDBTest");
        pqxx::work tx(connect);

        tx.exec("DROP TABLE IF EXISTS trafficjobs CASCADE");
        tx.exec("DROP TABLE IF EXISTS cardata CASCADE");
        tx.exec("DROP TABLE IF EXISTS snapshotData");
        tx.commit();

        // Run the tests with the job scheduler. 
        JobManager j;
        for (size_t i = 0; i< 3; ++i){
            j.submit(std::format("dbConfig{}.yaml", i));
        }
        for (size_t i = 0; i < 3; ++i){
            while(j.status(i) != JobStatus::DONE){
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }

    void TearDown() override {
        for (size_t i = 0; i < 3; ++i){
            std::filesystem::path fname = std::format("dbConfig{}.yaml", i);
            if (std::filesystem::exists(fname)) std::filesystem::remove(fname);
        }
    }

    protected:
    template <typename T>
    void checkError(std::expected<T, std::string>& result){
        if (!result){  FAIL() << result.error(); }
    }
};

TEST_F(DBReaderTest, evaulateDB){
    DBReader reader("host=localhost port=5432 dbname=trafficDBTest");

    // Job General Data
    std::vector<JobData> jobsSingle;
    for (size_t i = 0; i < 3; ++i){
        std::expected<JobData, std::string> data = reader.queryJobs(std::format("test-dbreader{}", i));
        checkError(data);
        jobsSingle.push_back(*data);
    }
    std::expected<std::vector<JobData>, std::string> jobsAll = reader.queryJobs();
    checkError(jobsAll);
    for (auto [single, all] : std::views::zip(jobsSingle, *jobsAll)){
        ASSERT_EQ(single.cfgPath_, all.cfgPath_);
        ASSERT_EQ(single.jobName_, all.jobName_);
    }

    std::vector<CarMetadata> singleMetadata;
    std::expected<std::vector<CarMetadata>, std::string> allCarMetadata = reader.queryCars("test-dbreader1");
    checkError(allCarMetadata);
    size_t ncars = allCarMetadata->size();
    for (size_t i = 0; i < ncars; ++i){
        auto data = reader.queryCars("test-dbreader1", i);
        checkError(data);
        singleMetadata.push_back(*data);
    }

    for (auto [single, all] : std::views::zip(singleMetadata, *allCarMetadata)){
        ASSERT_EQ(single.id_, all.id_);
        ASSERT_EQ(single.follow_, all.follow_);
        ASSERT_EQ(single.lead_, all.lead_);
        ASSERT_EQ(single.model_.a_, all.model_.a_);
        ASSERT_EQ(single.model_.b_, all.model_.b_);
    }

    // Due to the size of the raw data, just check if X is increasing 
    std::vector<RawData> singleRawData;
    std::expected<std::vector<RawData>, std::string> allRawData = reader.queryData("test-dbreader1");
    checkError(allCarMetadata);
    for (size_t i = 0; i < ncars; ++i){
        auto cardata = reader.queryData("test-dbreader1", i);
        checkError(cardata);
        singleRawData.push_back(*cardata);
        std::vector<float>& xs = singleRawData.back().x_;
        ASSERT_TRUE(std::ranges::is_sorted(xs));
    }

    for (auto [single, all] : std::views::zip(singleRawData, *allRawData)){
        ASSERT_EQ(single.id_, all.id_);
        ASSERT_EQ(single.x_, all.x_);
        ASSERT_EQ(single.v_, all.v_);
        ASSERT_EQ(single.t_, all.t_);
    }
}

