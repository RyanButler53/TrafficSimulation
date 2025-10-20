/**
 * @file jobManager.hpp
 * @author Ryan Butler
 * @brief Implements the Job Manager
 * @version 0.1
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

class Job {
    private:
    std::string inputPath_;

    public:
    Job(std::string path):inputPath_{path}{}
    void operator()();
};



class JobManager
{
private:
    /* data */
    // Job Manager accepts jobs from the submit endpoint
    std::atomic_bool isDone_;
    std::queue<std::shared_ptr<Job>> workQueue_;
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
     */
    void submit(std::string path);
};

