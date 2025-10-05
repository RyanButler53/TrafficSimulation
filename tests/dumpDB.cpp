#include <iostream>
#include <pqxx/pqxx>
#include <format>

int main(){
    pqxx::connection c("host=localhost port=5432 dbname=trafficDB");
    pqxx::work tx{c};

    std::string queryStr = std::format("SELECT * FROM TrafficJobs");
    // jobID int GENERATED ALWAYS AS IDENTITY, configfile text, jobname text, carTablePrefix text
    for (auto [jobid, name, cfg, tablePrefix] : tx.query<int,std::string,std::string, std::string >(queryStr)){
        std::cout << jobid << " " << name << " " << cfg << " " << tablePrefix << "\n";
    }

    return 0;
}