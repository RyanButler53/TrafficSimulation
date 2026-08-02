/**
 * @file migrateSchema.cpp
 * @author Ryan Butler
 * @brief Small script to clear the production database and 
 * @version 0.1
 * @date 2026-07-27
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <pqxx/pqxx>

int main(){

    pqxx::connection connect("host=localhost port=5432 dbname=trafficDB");
    pqxx::work tx(connect);

    tx.exec("DROP TABLE IF EXISTS trafficjobs CASCADE");
    tx.exec("DROP TABLE IF EXISTS cardata CASCADE");
    tx.exec("DROP TABLE IF EXISTS snapshotData");
    tx.commit();

}