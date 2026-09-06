#include "api/jobManager.hpp"

// Simulation Headers
#include "sim/parser.hpp"
#include "sim/parserFactory.hpp"
#include "sim/simulator.hpp"
#include <iostream>
#include <functional>

JobManager::JobManager(bool delayStart):jobid_{0}{
    if (!delayStart){
        workerThread_ = std::thread([this](){threadRoutine();});
    }
}

JobManager::~JobManager(){
    isDone_.store(true);
    if (workerThread_.joinable()){workerThread_.join();}
}

void JobManager::start(){
    workerThread_ = std::thread([this](){threadRoutine();});
}

void JobManager::threadRoutine(){
    // Forever loop
    while (!isDone_.load()){

        // Wait for a job off the queue. 250 ms timeout
        std::shared_ptr<Job> j = workQueue_.wait_and_pop(250);

        if (j){
            uint32_t id = j->id();
            statuses_[id] = JobStatus::RUNNING;
            statuses_[id] = (*j)();
        }
    }

}

// Do this with monads better
std::expected<std::pair<uint32_t, std::string>, std::string> JobManager::submit(std::string path){
    
    std::expected<SimulatorInputs, std::string> inputs = ParserFactory(path).makeParser()
                                                                            .and_then(std::mem_fn(&Parser::parse));
    if (!inputs.has_value()){
        return std::unexpected("Input Checking Error: " + inputs.error());
    } 
    statuses_.push_back(JobStatus::QUEUED);
    // Return is always true since this work queue has no limit
    workQueue_.try_push({inputs.value(), jobid_});
    ++jobid_;
    return std::make_pair(jobid_ - 1, inputs->jobname_);
}

JobStatus JobManager::status(uint32_t id){
    if (id < statuses_.size()){
        return statuses_[id];
    } else {
        throw std::invalid_argument(std::format("{} is not a valid job id. Maximum job id is {}", id, jobid_));
    }
}

// Runs the job
JobStatus Job::operator()(){
    // Error message will go to the database/file depending on logging type
    return Simulator(inputs_).run().transform([](){return JobStatus::DONE;}).value_or(JobStatus::ERROR);
}

