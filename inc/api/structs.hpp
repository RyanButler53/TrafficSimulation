// Defines the structs populated by the DB Reader and translated by the 
// api controller. 
#pragma once
#include <vector>
#include <string>

/**
 * @brief Struct to hold top level job data. Unique to each job. 
 * 
 */
struct JobData {
    std::string jobName_;
    std::string cfgPath_; // Text of the entire yaml file. 
    std::string errorMsg_; // Is empty when no error is present
    std::string status_;
    std::string driverModel_;
    int numCars_;
    float runtime_;
};

/**
 * @brief Follow model parameters.
 * 
 */
struct FollowModelParams {
    float a_; ///< acceleration
    float b_; ///< braking corefficient
    float c_; ///< Either max braking or safety gap depending on follow model
};

/**
 * @brief Struct to hold Car Metadat coming from the database 
 * 
 */
struct CarMetadata {
    FollowModelParams model_; ///< Follow Model
    float politeness_;
    float vDes_; ///< desired velocity
    int id_; ///< Car ID
};

/**
 * @brief Struct to hold all the raw snapshot data for a given car. 
 * 
 */
struct RawData {
    std::vector<float> x_;
    std::vector<float> v_;
    std::vector<float> t_;
    std::vector<float> l_;
    int id_;
};

/**
 * @brief Struct holding an individual snapshot data for the API. 
 * 
 */
struct Snapshot {
    int id_;
    float x_;
    float v_;
    int l_;
};

/**
 * @brief Struct to hold Databae's data as a time series
 * 
 */
struct TimeSeries {
    std::vector<float> timestamps_; ///< All timestamps found (sorted)
    std::vector<std::vector<Snapshot>> snapshots_; ///< All car snapshots found by timestamp
};

/**
 * @brief Status of the job. 
 * 
 */
enum class JobStatus : uint8_t {
    INVALID = 0, // Jobs that can't parse
    QUEUED = 1,
    RUNNING = 2,
    DONE = 3,
    ERROR = 4 // Jobs that throw runtime errors
};
