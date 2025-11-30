#pragma once

#include <atomic>
class TrafficApi {

    std::atomic_bool serverOn_ = true;
    bool useTestDB_;

    public: 

    TrafficApi(bool testDB = false):useTestDB_{testDB}{}
    
    void run();
    void closeServer();

};