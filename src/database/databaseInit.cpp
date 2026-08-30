#include "database/databaseInit.hpp"

#include <pqxx/pqxx>

std::expected<void, std::string> Database::initDB(bool useTestDB){
    std::string connStr = "host=localhost port=5432 dbname=trafficDB";
    if (useTestDB){
        connStr = "host=localhost port=5432 dbname=trafficDBTest";
    }

    try {
        pqxx::connection connect(connStr);
        
        pqxx::work tx(connect);
        
        tx.exec("CREATE TABLE IF NOT EXISTS trafficJobs ( jobID int GENERATED ALWAYS AS IDENTITY PRIMARY KEY, configfile text, jobname text, status varchar(7), error text, followModel varchar(5), numCars int, runtime float)");
        tx.exec("CREATE TABLE IF NOT EXISTS carData (carID int, jobID int, follow_a float, follow_b float, follow_c float, politeness float, desired_vel float, FOREIGN KEY (jobID) REFERENCES trafficjobs(jobID) , PRIMARY KEY (carID, jobID))");
        tx.exec("CREATE TABLE IF NOT EXISTS snapshotData (carID int, jobID int, x float, v float, t float, lane int, PRIMARY KEY (carID, jobID, t), FOREIGN KEY (carID, jobID) REFERENCES cardata (carID, jobID))");
        tx.exec("CREATE TABLE IF NOT EXISTS laneSegments (jobID int, segmentStart float, segmentEnd float, rate float, position int, FOREIGN KEY (jobID) REFERENCES trafficJobs(jobID))");
        tx.exec("CREATE TABLE IF NOT EXISTS environment (jobID int, x0 float, xf float, numLanes int, FOREIGN KEY (jobID) REFERENCES trafficJobs(jobID))");
        tx.commit();
        return {};
    }
    catch(const std::exception& e)
    {
        return std::unexpected(std::format("Error Initializing the Database: {}", e.what()));
    }
}


std::expected<void, std::string> Database::clearDB(bool useTestDB){
    std::string connStr = "host=localhost port=5432 dbname=trafficDB";
    if (useTestDB){
        connStr = "host=localhost port=5432 dbname=trafficDBTest";
    }
    try  {
        pqxx::connection connect(connStr);
        pqxx::work tx(connect);

        tx.exec("DROP TABLE IF EXISTS trafficjobs CASCADE");
        tx.exec("DROP TABLE IF EXISTS cardata CASCADE");
        tx.exec("DROP TABLE IF EXISTS snapshotData");
        tx.exec("DROP TABLE IF EXISTS environment");
        tx.exec("DROP TABLE IF EXISTS laneSegments");
        tx.commit();
        return {};
    } catch(const std::exception& e) {
        return std::unexpected(std::format("Error clearing the database: {}", e.what()));
    }    
}
