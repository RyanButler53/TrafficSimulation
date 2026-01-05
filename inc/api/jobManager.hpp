/**
 * @file jobManager.hpp
 * @author Ryan Butler
 * @brief Implements the Job Manager
 * @version 0.2
 * @date 2025-10-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

 #pragma once

#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <expected>
#include "structs.hpp"
#include "sim/simulator.hpp"



class Job {

    private:
    SimulatorInputs inputs_;
    uint32_t id_;
    
    public:
    Job(const SimulatorInputs& inputs, uint32_t id):inputs_{inputs}, id_{id}{}

    JobStatus operator()();
    uint32_t id() const {return id_;}
};



/**
 * @brief Job Manager accepts from submit endpoint. 
 * @note Job manager is independent from the API and can be 
 * constructed on its own
 * 
 */
class JobManager
{
private:

    std::vector<JobStatus> statuses_;
    std::queue<std::shared_ptr<Job>> workQueue_;
    uint32_t jobid_; // Job ID from the Job manager. NOT releated to the Databse Job ID. 
    std::atomic_bool isDone_;
    std::mutex queueMutex_;
    std::thread workerThread_;
    std::condition_variable cv_;


    /**
     * @brief Worker thread routine. 
     * 
     */
    void threadRoutine();

public:
    JobManager();
    ~JobManager();

    /**
     * @brief Submits a job to the queue. 
     * 
     * @param path Config file to use. 
     * @return Job ID to query status
     */
    std::expected<uint32_t, std::string> submit(std::string path);

    /**
     * @brief Check the status of a given job id. 
     * 
     * @param id 
     * @return JobStatus 
     */
    JobStatus status(uint32_t id);
};

