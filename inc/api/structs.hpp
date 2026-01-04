// Defines the structs populated by the DB Reader and translated by the 
// api controller. 
#pragma once
#include <vector>
#include <string>

struct JobData {
    std::string jobName_;
    std::string cfgPath_;
    std::string errorMsg_; // Is empty when no error is present
    std::string status_;
};

struct FollowModelParams {
    double a_;
    double b_;
};

struct CarMetadata {
    std::string lead_;
    std::string follow_;
    FollowModelParams model_;
    int id_;
};

struct RawData {
    std::vector<float> x_;
    std::vector<float> v_;
    std::vector<float> t_;
    int id_;
};

enum class JobStatus : uint8_t {
    INVALID = 0, // Jobs that can't parse
    QUEUED = 1,
    RUNNING = 2,
    DONE = 3,
    ERROR = 4 // Jobs that throw runtime errors
};
