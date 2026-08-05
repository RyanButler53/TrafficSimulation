#include "yaml-cpp/yaml.h"
#include "testUtil.hpp"
#include <fstream>
#include <chrono>
#include <ranges>
#include <filesystem>
#include <string>
#include <print>
#include <charconv>

#include "api/jobManager.hpp"
#include "api/DBManager.hpp"

class Benchmark{
    std::vector<std::filesystem::path> paths_;
    public: 

    Benchmark(int n){
        TestUtil::clearDB();

        for (int i : std::views::iota(0, n)){
            YAML::Node cfg = TestUtil::getConfigNode_3Lane();
            std::string file_name = std::format("timing{}_file", i);
            std::string db_name = std::format("timing{}_db", i);
            cfg["logtype"] = "file";
            cfg["jobname"] = file_name;
            cfg["timestep"] = 0.1; // dt = 0.1 tests streaming
        
            // Heterogeneous traffic
            cfg["driverParams"]["a_stdev"] = 0.1;
            cfg["driverParams"]["b_stdev"] = 0.2;
            cfg["driverParams"]["bmax_stdev"] = 0.2;
            cfg["driverParams"]["p_stdev"] = 0.02;
        
            TestUtil::configToFile(cfg, std::format("{}.yaml", file_name));
    
            cfg["logtype"] = "test";
            cfg["jobname"] = std::format("timing{}_db", i);
            TestUtil::configToFile(cfg, std::format("{}.yaml", db_name));

            // Save Paths
            paths_.push_back(std::format("{}.yaml", file_name));
            paths_.push_back(std::format("{}.yaml", db_name));
        }
    }

    void operator()(){
        JobManager jobScheduler;
        DBManager database(true);
        for (auto p : paths_){
            jobScheduler.submit(p);
        }

        // Need to wait until the jobs are finished. Need to query them for the timing. 

        std::vector<float> dbtimes;
        std::vector<float> filetimes;
        int n = paths_.size();
        for (int i : std::views::iota(0,n)){
            std::filesystem::path path = paths_[i];
            std::string fname = path.string();
            std::string jobname = fname.erase(fname.size() - 5);
            
            if (i % 2 == 0){
                std::filesystem::path fpath = fname / std::filesystem::path("stats.txt");

                while (!std::filesystem::exists(fpath)){
                    std::this_thread::sleep_for (std::chrono::seconds(1));
                }
                std::ifstream in(fpath);
                std::string textline;
                in >> textline;
                in >> textline;
                float f;
                std::from_chars(textline.data(), textline.data() + textline.size(), f);
                filetimes.push_back(f);


            } else {
                JobData data;
                data.status_ = "";
                while (data.status_ != "DONE"){
                    auto expdata = database.queryJobs(jobname);
                    if (expdata){
                        std::this_thread::sleep_for (std::chrono::seconds(1));
                        data = expdata.value();
                    } else if (data.status_ == "ERROR"){
                        break;
                    } else {
                        break;
                    }
                }
                dbtimes.push_back(data.runtime_);
            }
        }

        float fileAvg = std::accumulate(filetimes.begin(), filetimes.end(), 0.0)/static_cast<float>(filetimes.size());
        float dbavg = std::accumulate(dbtimes.begin(), dbtimes.end(), 0.0)/static_cast<float>(dbtimes.size());
        std::println("File Average Time: {}", fileAvg);
        std::println("Database Average Time: {}", dbavg);
    }

    ~Benchmark(){
        for (auto& p : paths_){
            std::string fname = p.string();
            std::string jobname = fname.erase(fname.size() - 5);
            if (std::filesystem::exists(p)){
                std::filesystem::remove(p);
            }
            if (std::filesystem::is_directory(jobname)){
                std::filesystem::remove_all(jobname);
            }
        }
        TestUtil::clearDB();

    }
};


int main(int argc, char** argv){
    int n = 5;
    if (argc > 2){
        std::string s(argv[1]);
        std::from_chars(s.data(), s.data() + s.size(), n);
    }
    Benchmark b(n);
    b();
}
