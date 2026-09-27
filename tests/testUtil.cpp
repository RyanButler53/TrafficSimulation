
#include "testUtil.hpp"
#include <fstream>
#include "database/databaseInit.hpp"

namespace TestUtil{

YAML::Node getConfigNode() {
    YAML::Node cfg;
    cfg["jobinfo"]["type"] = "continuous";
    cfg["jobinfo"]["time"] = 150;
    cfg["jobinfo"]["timestep"] = 1;

    // Driver params (From traffic flow book example)
    cfg["cars"]["driverType"] = "Gipps";
    cfg["cars"]["driverParams"]["a"] = 1.981;
    cfg["cars"]["driverParams"]["b"] = -2.8955;
    cfg["cars"]["driverParams"]["bmax"] = -5.505;
    cfg["cars"]["driverParams"]["p"] = 0.2;

    // ["cars"]Homogeneous traffic
    cfg["cars"]["driverParams"]["a_stdev"] = 0;
    cfg["cars"]["driverParams"]["a_stdev"] = 0;
    cfg["cars"]["driverParams"]["bmax_stdev"] = 0;
    cfg["cars"]["driverParams"]["p_stdev"] = 0;


    cfg["highway"]["lanes"];
    YAML::Node lane1, lane2;

    lane1["flow"]["rate"] = 800;
    lane1["flow"]["v0"] = 20;
    lane1["flow"]["vdes"] = 35;
    lane1["start"] = 0;
    lane1["end"] = 2000;
    lane1["position"]  = 0;

    lane2["flow"]["rate"] = 400;
    lane2["flow"]["v0"] = 0;
    lane2["flow"]["vdes"] = 38;
    lane2["start"] = 0;
    lane2["end"] = 2000;
    lane2["position"]  = 1;

    cfg["highway"]["lanes"].push_back(lane1);
    cfg["highway"]["lanes"].push_back(lane2);

    return cfg;
}

// Gets the config node except for the log dir/log type fields. 
YAML::Node getConfigNode_3Lane() {
    YAML::Node cfg;
    cfg["jobinfo"]["jobname"] = "test-file";
    cfg["jobinfo"]["type"] = "continuous";
    cfg["jobinfo"]["time"] = 900; // 15 minutes
    cfg["jobinfo"]["timestep"] = 1;
    cfg["jobinfo"]["seed"] = 70;

    // Driver params (From traffic flow book example)
    cfg["cars"]["driverType"] = "Gipps";
    cfg["cars"]["driverParams"]["a"] = 1.981;
    cfg["cars"]["driverParams"]["b"] = -2.8955;
    cfg["cars"]["driverParams"]["bmax"] = -5.505;
    cfg["cars"]["driverParams"]["p"] = 0.2;

    // Homogeneous traffic
    cfg["cars"]["driverParams"]["a_stdev"] = 0.0;
    cfg["cars"]["driverParams"]["b_stdev"] = 0.0;
    cfg["cars"]["driverParams"]["bmax_stdev"] = 0.0;
    cfg["cars"]["driverParams"]["p_stdev"] = 0.0;

    // 3 lanes of traffic
    cfg["highway"]["lanes"];
    YAML::Node lane1, lane2, lane3;

    lane1["flow"]["rate"] = 200;
    lane1["flow"]["v0"] = 0;
    lane1["flow"]["vdes"] = 30;
    lane1["start"] = 0;
    lane1["end"] = 2000;
    lane1["position"]  = 0;

    lane2["flow"]["rate"] = 400;
    lane2["flow"]["v0"] = 30;
    lane2["flow"]["vdes"] = 35;
    lane2["start"] = 0;
    lane2["end"] = 2000;
    lane2["position"]  = 1;

    lane3["flow"]["rate"] = 450;
    lane3["flow"]["v0"] = 30;
    lane3["flow"]["vdes"] = 38;
    lane3["start"] = 0;
    lane3["end"] = 2000;
    lane3["position"]  = 2;

    cfg["highway"]["lanes"].push_back(lane1);
    cfg["highway"]["lanes"].push_back(lane2);
    cfg["highway"]["lanes"].push_back(lane3);

    return cfg;
}

void configToFile(YAML::Node cfg, std::string fname){
    YAML::Emitter cfgyaml;
    std::ofstream fileout(fname);
    cfgyaml << cfg;
    fileout << cfgyaml.c_str();
    fileout.close();
}


void clearDB() {
    Database::clearDB(true);
}


void conditionalFileCleanup(std::string file){
    if (std::filesystem::exists(file)) std::filesystem::remove(file);
}
void conditionalFileCleanup(std::vector<std::string> files){
    for (const std::string& f : files){
        conditionalFileCleanup(f);
    }
}

void conditionalFolderCleanup(std::filesystem::path folder){
    if (std::filesystem::exists(folder)) std::filesystem::remove_all(folder);

}
void conditionalFolderCleanup(std::vector<std::filesystem::path> folders){
    for (const std::string& f : folders){
        conditionalFolderCleanup(f);
    }
}

}
