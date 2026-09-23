#pragma once

#include <atomic>

/**
 * @brief Encapsulates running the Traffic API. 
 * 
 */
class TrafficApi {

    std::atomic_bool serverOn_ = true; ///< Is the server on? 
    bool useTestDB_; ///< Boolean to check if using the test DB

    public: 

    /**
     * @brief Construct a new Traffic Api object. 
     * @note The server is launched lazily via \ref run()
     * 
     * @param testDB True to use test database, false to use PROD. 
     */
    TrafficApi(bool testDB = false):useTestDB_{testDB}{}
    
    /**
     * @brief Run the API. This launches the server
     * 
     */
    void run();

    /**
     * @brief Closes the API server. Used to tear down the environment and server
     * 
     */
    void closeServer();

};