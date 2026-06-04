#include <gtest/gtest.h>
#include <fstream>
#include <algorithm>
#include <fstream>

#include "api/DBManager.hpp"
#include "api/jobManager.hpp"
#include "api/structs.hpp"
#include "yaml-cpp/yaml.h"
#include "testUtil.hpp"

#include <pqxx/pqxx>

class DBManagerTest : public ::testing::Test {

    protected:

    void SetUp() override {

        for (size_t i = 0; i < 3; ++i){
            YAML::Node dbLog = TestUtil::getConfigNode();
            dbLog["logtype"] = "test";
            dbLog["jobname"] = std::format("test-dbreader{}", i);
            dbLog["seed"] = 70 + i;

            YAML::Emitter dbout;
            std::ofstream dbCfg(std::format("dbConfig{}.yaml", i));
            dbout << dbLog;
            dbCfg << dbout.c_str();
        }

        TestUtil::clearDB();

        // Run the tests with the job scheduler. 
        JobManager j;
        for (size_t i = 0; i< 3; ++i){
            j.submit(std::format("dbConfig{}.yaml", i));
        }
        for (size_t i = 0; i < 3; ++i){
            while(j.status(i) != JobStatus::DONE && j.status(i) != JobStatus::ERROR){
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

};

// Disabled until error condition found. 
class ErrorLogTest : public DBManagerTest {

    void SetUp() override {
        YAML::Node dbLog = TestUtil::getConfigNode();
        dbLog["jobname"] = "test-dbreader3";
        dbLog["seed"] = 70;
        dbLog["flow"]["rate"] = 900;
        dbLog["flow"]["v0"] = 40;

        YAML::Emitter dbout;
        std::ofstream dbCfg("dbConfig3.yaml");
        dbout << dbLog;
        dbCfg << dbout.c_str();

        TestUtil::clearDB();

    }

    void TearDown() override {
        std::filesystem::path fname = "dbConfig3.yaml";
        if (std::filesystem::exists(fname)) std::filesystem::remove(fname);
    }
};


TEST_F(DBManagerTest, evaulateDB){
    DBManager reader(true);

    // Job General Data
    std::vector<JobData> jobsSingle;
    for (size_t i = 0; i < 3; ++i){
        std::expected<JobData, std::string> data = reader.queryJobs(std::format("test-dbreader{}", i));
        ASSERT_TRUE(data.has_value()) << std::format("Error Querying Job {}: {}", i, data.error());
        jobsSingle.push_back(*data);
    }
    std::expected<std::vector<JobData>, std::string> jobsAll = reader.queryJobs();
    ASSERT_TRUE(jobsAll.has_value()) << std::format("Error Querying All Jobs: {}", jobsAll.error());
    EXPECT_EQ(jobsSingle.size(), jobsAll->size());
    for (auto [single, all] : std::views::zip(jobsSingle, *jobsAll)){
        EXPECT_EQ(single.cfgPath_, all.cfgPath_);
        EXPECT_EQ(single.jobName_, all.jobName_);
        EXPECT_EQ(single.errorMsg_, all.errorMsg_);
        EXPECT_EQ(single.driverModel_, all.driverModel_);
        EXPECT_EQ(single.numCars_, all.numCars_);

        // Check individual values that should be the same throughout all the jobs
        EXPECT_EQ(all.driverModel_, "Gipps");
        EXPECT_EQ(all.errorMsg_, "");
        EXPECT_EQ(all.status_, "DONE");
        EXPECT_GT(all.numCars_, 0);
    }

    std::vector<CarMetadata> singleMetadata;
    std::expected<std::vector<CarMetadata>, std::string> allCarMetadata = reader.queryCars("test-dbreader1");
    EXPECT_TRUE(allCarMetadata.has_value()) << std::format("Error querying all cars metadata: {}", allCarMetadata.error());
    size_t ncars = allCarMetadata->size();
    EXPECT_EQ(ncars, 22) << "Case is known to have 22 cars";
    for (size_t i = 0; i < ncars; ++i){
        auto data = reader.queryCars("test-dbreader1", i);
        EXPECT_TRUE(data.has_value()) << std::format("Error Querying Car {}: {}",i,  data.error());
        singleMetadata.push_back(*data);
    }

    for (auto [single, all] : std::views::zip(singleMetadata, *allCarMetadata)){
        EXPECT_EQ(single.id_, all.id_);
        EXPECT_EQ(single.politeness_, all.politeness_);
        EXPECT_EQ(single.model_.a_, all.model_.a_);
        EXPECT_EQ(single.model_.b_, all.model_.b_);
        EXPECT_EQ(single.model_.c_, all.model_.c_);

        EXPECT_EQ(single.politeness_, all.politeness_);
        EXPECT_EQ(single.model_.a_, all.model_.a_);
        EXPECT_EQ(single.model_.b_, all.model_.b_);

        // Confirm the data is being read from the DB correctly. 
        EXPECT_FLOAT_EQ(all.model_.a_,  1.981);
        EXPECT_FLOAT_EQ(all.model_.b_, -2.8955);
        EXPECT_FLOAT_EQ(all.model_.c_, -5.505);
        EXPECT_FLOAT_EQ(all.politeness_, 0.2);
    }

    // Due to the size of the raw data, just check if X is increasing 
    std::vector<RawData> singleRawData;
    std::expected<std::vector<RawData>, std::string> allRawData = reader.queryData("test-dbreader1");
    EXPECT_TRUE(allRawData.has_value()) << std::format("Error querying all cars snapshots: {}", allRawData.error());
    for (size_t i = 0; i < ncars; ++i){
        auto cardata = reader.queryData("test-dbreader1", i);
        EXPECT_TRUE(cardata.has_value()) << std::format("Error querying car {}: {}", i, cardata.error());
        singleRawData.push_back(*cardata);
        std::vector<float>& xs = singleRawData.back().x_;
        ASSERT_TRUE(std::ranges::is_sorted(xs));
    }

    for (auto [single, all] : std::views::zip(singleRawData, *allRawData)){
        EXPECT_EQ(single.id_, all.id_);
        EXPECT_EQ(single.x_, all.x_);
        EXPECT_EQ(single.v_, all.v_);
        EXPECT_EQ(single.t_, all.t_);
        EXPECT_EQ(single.l_, all.l_);
    }

    // Clean up jobs: 
    for (size_t i = 0; i < 3; ++i){
        std::expected<void, std::string> result = reader.deleteJob(std::format("test-dbreader{}", i));
        EXPECT_TRUE(result.has_value()) << std::format("Error deleting job {}: {}", i, result.error());
    }
    jobsAll = reader.queryJobs();
    ASSERT_TRUE(jobsAll.has_value()) << std::format("Error Querying All Jobs: {}", jobsAll.error());
    EXPECT_TRUE(jobsAll.value().empty());
}

TEST_F(ErrorLogTest, DISABLED_errorLogging){
    // Need to see if the error exists. 

    std::expected<void, std::string> result = Traffic::Simulate("dbConfig3.yaml");
    ASSERT_TRUE(result.has_value()) << std::format("Error Querying Job test-dbreader3: {}", result.error());

    DBManager reader(true);

    std::expected<JobData, std::string> data = reader.queryJobs("test-dbreader3");
    ASSERT_TRUE(data.has_value()) << std::format("Error Querying Job test-dbreader3: {}", data.error());

    // ASSERT_EQ(data->errorMsg_, "Accident at t = 10: Car 6: x = 39.24 Leader: x = 16.13");
    EXPECT_EQ(data->status_, "ERROR");
}