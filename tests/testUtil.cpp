
#include "testUtil.hpp"
#include <pqxx/pqxx>

namespace TestUtil{

YAML::Node getConfigNode() {
    YAML::Node cfg;
    cfg["type"] = "continuous";
    cfg["time"] = 150;
    cfg["timestep"] = 1;

    // Driver params (From traffic flow book example)
    cfg["driverType"] = "Gipps";
    cfg["driverParams"]["a"] = 1.981;
    cfg["driverParams"]["b"] = -2.8955;
    cfg["driverParams"]["bmax"] = -5.505;
    cfg["driverParams"]["p"] = 0.2;

    // Homogeneous traffic
    cfg["driverParams"]["a_stdev"] = 0;
    cfg["driverParams"]["a_stdev"] = 0;
    cfg["driverParams"]["bmax_stdev"] = 0;
    cfg["driverParams"]["p_stdev"] = 0;


    cfg["lanes"];
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
    lane2["posiiton"]  = 1;

    cfg["lanes"].push_back(lane1);
    cfg["lanes"].push_back(lane2);

    return cfg;
}

// Gets the config node except for the log dir/log type fields. 
YAML::Node getConfigNode_3Lane() {
    YAML::Node cfg;
    cfg["jobname"] = "test-file";
    cfg["type"] = "continuous";
    cfg["time"] = 900; // 15 minutes
    cfg["timestep"] = 1;
    cfg["seed"] = 70;

    // Driver params (From traffic flow book example)
    cfg["driverType"] = "Gipps";
    cfg["driverParams"]["a"] = 1.981;
    cfg["driverParams"]["b"] = -2.8955;
    cfg["driverParams"]["bmax"] = -5.505;
    cfg["driverParams"]["p"] = 0.2;

    // Homogeneous traffic
    cfg["driverParams"]["a_stdev"] = 0.0;
    cfg["driverParams"]["b_stdev"] = 0.0;
    cfg["driverParams"]["bmax_stdev"] = 0.0;
    cfg["driverParams"]["p_stdev"] = 0.0;

    // 3 lanes of traffic
    cfg["lanes"];
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

    cfg["lanes"].push_back(lane1);
    cfg["lanes"].push_back(lane2);
    cfg["lanes"].push_back(lane3);

    return cfg;
}


void clearDB() {
    // Clear out the Test DB:
    pqxx::connection connect("host=localhost port=5432 dbname=trafficDBTest");
    pqxx::work tx(connect);

    tx.exec("DROP TABLE IF EXISTS trafficjobs CASCADE");
    tx.exec("DROP TABLE IF EXISTS cardata CASCADE");
    tx.exec("DROP TABLE IF EXISTS snapshotData");
    tx.commit();
}

}
