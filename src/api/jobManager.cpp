#include "api/jobManager.hpp"

// Simulation Headers
#include "sim/parser.hpp"
#include "sim/parserFactory.hpp"
#include "sim/simulator.hpp"
#include <iostream>

JobManager::JobManager():workerThread_{[this](){threadRoutine();}}{}

JobManager::~JobManager(){
    isDone_.store(true);
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
            j->operator()();
        } else {
            std::unique_lock lk(queueMutex_);
            cv_.wait(lk, [this](){return !workQueue_.empty();});
        }
    }

}

void JobManager::submit(std::string path){
    std::unique_lock lk(queueMutex_);
    workQueue_.push(std::make_shared<Job>(path));
    lk.unlock();
    cv_.notify_one();
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

