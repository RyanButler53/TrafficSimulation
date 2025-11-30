#include <iostream>
#include <pqxx/pqxx>
#include <format>

int main(int argc, char** argv){

    int jobid = 0;
    if (argc == 2){
        jobid = atoi(argv[1]);
    }

    std::string queryStr;
    pqxx::connection c("host=localhost port=5432 dbname=trafficDB");
    pqxx::work tx{c};

    queryStr = std::format("SELECT * FROM TrafficJobs");
    for (auto [jobid, name, cfg] : tx.query<int,std::string, std::string >(queryStr)){
        std::cout << jobid << " " << name << " " << cfg  << "\n";
    }

    queryStr = std::format("SELECT (carid, follow, lead) FROM CarData WHERE jobid = {}", jobid);
    for (auto [carid, follow, lead] : tx.query<int,std::string, std::string >(queryStr)){
        std::cout << carid << " " << follow << " " << lead  << "\n";
    }

    queryStr = std::format("SELECT (x, v, t) FROM SnapshotData WHERE jobid = {}", jobid);
    for (auto [x, v, t] : tx.query<float, float, float>(queryStr)){
        std::cout << x << " " << v << " " << t  << "\n";
    }


    return 0;
}