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
            statuses_[id] = j->operator()();
        } else {
            std::unique_lock lk(queueMutex_);
            cv_.wait(lk, [this](){return !workQueue_.empty() || isDone_.load();});
        }
    }

}

std::expected<uint32_t, std::string> JobManager::submit(std::string path){

    // Do the Parsing here since its so fast it can be redone later. 
    
    std::expected<SimulatorInputs, std::string> inputs = ParserFactory(path).makeParser()
                                                                            .and_then([](std::unique_ptr<Parser> p){return p->parse();});
    if (!inputs.has_value()){
        std::cout << "Input checking error" << std::endl;
        return std::unexpected("Input Checking Error: " + inputs.error());
    } 
    statuses_.push_back(JobStatus::QUEUED);
    std::unique_lock lk(queueMutex_);
    auto j = std::make_shared<Job>(inputs.value(), jobid_);
    workQueue_.push(j);
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
JobStatus Job::operator()(){
    // Loses the error message: Where should it go?
    return Simulator(inputs_).run().transform([](){return JobStatus::DONE;}).value_or(JobStatus::FAILED);
}

