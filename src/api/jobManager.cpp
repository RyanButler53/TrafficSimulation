#include "api/jobManager.hpp"

// Simulation Headers
#include "sim/parser.hpp"
#include "sim/parserFactory.hpp"
#include "sim/simulator.hpp"
#include <iostream>

JobManager::JobManager():jobid_{0},workerThread_{[this](){threadRoutine();}}{}

JobManager::~JobManager(){
    isDone_.store(true);
    cv_.notify_one(); // notify the worker thread to exit out
    if (workerThread_.joinable()){workerThread_.join();}
}

void JobManager::threadRoutine(){
    // Forever loop
    while (!isDone_.load()){
        std::shared_ptr<Job> j;
        {
        std::scoped_lock lk(queueMutex_);
        if (!workQueue_.empty()){
            j = workQueue_.front();
            workQueue_.pop();
        } 
        }
        
        if (j){
            uint32_t id = j->id();
            statuses_[id] = JobStatus::RUNNING;
            j->operator()();
            statuses_[id] = JobStatus::DONE;
        } else {
            std::unique_lock lk(queueMutex_);
            cv_.wait(lk, [this](){return !workQueue_.empty() || isDone_.load();});
        }
    }

}

uint32_t JobManager::submit(std::string path){
    statuses_.push_back(JobStatus::QUEUED);
    std::unique_lock lk(queueMutex_);
    workQueue_.push(std::make_shared<Job>(path, jobid_));
    lk.unlock();
    cv_.notify_one();
    return jobid_++;
}

JobStatus JobManager::status(uint32_t id){
    if (id < statuses_.size()){
        return statuses_[id];
    } else {
        throw std::invalid_argument(std::format("{} is not a valid job id. Maximum job id is {}", id, jobid_));
    }
}

// Runs the job
void Job::operator()(){
    ParserFactory parserFac(inputPath_);
    std::shared_ptr<Parser> parser;
    try {
       parser = parserFac.makeParser();
    } catch(const std::exception& e) {
        return;
    }
    
    SimulatorInputs inputs = parser->parse();
    Simulator s(inputs);
    s.run();
    return;
}

