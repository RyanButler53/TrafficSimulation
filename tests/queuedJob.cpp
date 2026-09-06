#include <gtest/gtest.h>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <string>
#include <format>

#include "api/DBManager.hpp"
#include "api/jobManager.hpp"
#include "api/structs.hpp"
#include "testUtil.hpp"
#include "yaml-cpp/yaml.h"


class QueuedJobTest : public ::testing::Test {

    DBManager dbManager_{true};
    protected:
    void SetUp() override {
        YAML::Node dbLog = TestUtil::getConfigNode();
        dbLog["logtype"] = "test";
        dbLog["jobname"] = "test-queued";
        dbLog["seed"] = 133;

        TestUtil::configToFile(dbLog, "test-queued.yml");
        dbManager_.deleteJob("test-queued");
        TestUtil::clearDB();
    }

    void TearDown() override {
        EXPECT_TRUE(dbManager_.deleteJob("test-queued").has_value());
        TestUtil::conditionalFileCleanup("test-queued.yml");
    }
};

TEST_F(QueuedJobTest, QueryQueuedJob){
    JobManager j(true);

    auto job = j.submit("test-queued.yml");
    ASSERT_TRUE(job.has_value()) << "Error submitting job: " << job.error();
    uint32_t jobid = job->first;
    EXPECT_EQ(job->second, "test-queued");
    EXPECT_EQ(j.status(jobid), JobStatus::QUEUED);

    DBManager db(true);

    std::expected<JobData, std::string> data = db.queryJobs("test-queued");
    ASSERT_TRUE(data.has_value()) << "Error querying the database: " << data.error();

    EXPECT_EQ(data->status_, "QUEUED");
    EXPECT_FLOAT_EQ(data->runtime_, -1.0);
    EXPECT_EQ(data->numCars_, 0);

    // Start the job
    j.start();

    while(j.status(jobid) != JobStatus::DONE && j.status(jobid) != JobStatus::ERROR){
        if (j.status(jobid) == JobStatus::RUNNING){
            data = db.queryJobs("test-queued");
            // Ensure that querying during running never fails
            EXPECT_TRUE(data.has_value());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    EXPECT_EQ(j.status(jobid), JobStatus::DONE);

    data = db.queryJobs("test-queued");
    EXPECT_GT(data->runtime_, 0.0);
}