#include "database/databaseInit.hpp"

#include <pqxx/pqxx>

std::expected<void, std::string> initDB::initDB(bool useTestDB){
    std::string connStr = "host=localhost port=5432 dbname=trafficDB";
    if (useTestDB){
        connStr = "host=localhost port=5432 dbname=trafficDBTest";
    }

    try {
        pqxx::connection connect(connStr);
        
        pqxx::work tx(connect);
        
        tx.exec("CREATE TABLE IF NOT EXISTS trafficJobs ( jobID int GENERATED ALWAYS AS IDENTITY PRIMARY KEY, configfile text, jobname text, status varchar(7), error text, followModel varchar(5), numCars int)");
        tx.exec("CREATE TABLE IF NOT EXISTS carData (carID int, jobID int, follow_a float, follow_b float, follow_c float, lead text, FOREIGN KEY (jobID) REFERENCES trafficjobs(jobID) , PRIMARY KEY (carID, jobID))");
        tx.exec("CREATE TABLE IF NOT EXISTS snapshotData (carID int, jobID int, x float, v float, t float, PRIMARY KEY (carID, jobID, t), FOREIGN KEY (carID, jobID) REFERENCES cardata (carID, jobID))");
        tx.commit();
        return {};
    }
    catch(const std::exception& e)
    {
        return std::unexpected(std::format("Error Initializing the Database: {}", e.what()));
    }
    

}