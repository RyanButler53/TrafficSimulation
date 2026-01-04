#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>
#include <format>
#include <expected>
#include <sstream>
#include <source_location>

#include <gtest/gtest.h>
#include "api/api.hpp"  // Include API Running
#include "oatpp/network/Server.hpp"     
#include "yaml-cpp/yaml.h"
#include <nlohmann/json.hpp>
#include "curl/curl.h"


using json = nlohmann::json;

struct CurlData{
    json jsonData;
    long code;
};

using CurlResponse = std::expected<CurlData, std::string>;

// Class to wrap all the Curl Calls. 
class CurlWrapper {

    static size_t write_data(void *ptr, size_t size, size_t nmemb, std::string* data) {
        data->append((char*) ptr, size * nmemb);
        return size * nmemb;
    }
    CurlResponse getQuery(std::string url);

    public: 


    CurlResponse queryJobs(){return getQuery("/jobs");}; // GET /jobs
    CurlResponse queryJob(std::string jobname) {return getQuery(std::format("/jobs/{}", jobname));} // get /jobs/{jobname}
    CurlResponse queryCarData(std::string jobname, size_t id){return getQuery(std::format("/data/{}/cars/{}", jobname, id));} // GET /data/{jobname}/cars/id
    CurlResponse queryCarDatas(std::string jobname) {return getQuery(std::format("/data/{}/cars", jobname));} // GET /data/{jobname}
    CurlResponse queryRawData(std::string jobname, size_t id) {return getQuery(std::format("/data/{}/raw/{}", jobname, id));} // GET /data/{jobname}/cars/raw/id
    CurlResponse queryRawDatas(std::string jobname) {return getQuery(std::format("/data/{}/raw", jobname));}; // GET /data/{jobname}/cars/raw
    
    // Post and delete
    CurlResponse postJob(std::string jobname, std::filesystem::path cfg);
    CurlResponse deleteJob(std::string jobname);

};

CurlResponse CurlWrapper::getQuery(std::string url){

    CURL* handle = curl_easy_init();
    if (!handle){
        return std::unexpected("Could not initalize curl handle");
    }
    curl_easy_setopt(handle, CURLOPT_URL, ("http://localhost:8000" + url).c_str());
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, CurlWrapper::write_data);

    curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 50L);

    std::string response_string;
    std::string header_string;
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(handle, CURLOPT_HEADERDATA, &header_string);
    

    CURLcode errorCode = curl_easy_perform(handle);
    long code;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &code);

    curl_easy_cleanup(handle);

    if (errorCode){
        return std::unexpected("Curl Error: " + std::to_string(errorCode));
    }
    return CurlData{json::parse(response_string), code};
}


CurlResponse CurlWrapper::postJob(std::string jobname, std::filesystem::path cfgpath){
    CURL* handle = curl_easy_init();
    if (!handle){
        return std::unexpected("Could not initalize curl handle");
    }

    std::string url = std::format("http://localhost:8000/submit/{}?config={}", jobname, cfgpath.string());
    std::string response_string;

    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, CurlWrapper::write_data);
    curl_easy_setopt(handle, CURLOPT_POST, 1L);
    curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response_string);

    CURLcode errorCode = curl_easy_perform(handle);
    
    long code;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &code);


    curl_easy_cleanup(handle);
    if (errorCode){
        return std::unexpected("Curl Error: " + std::to_string(errorCode));
    }
    return CurlData{json::parse(response_string), code};


}

CurlResponse CurlWrapper::deleteJob(std::string jobname){
    CURL* handle = curl_easy_init();
    if (!handle){
        return std::unexpected("Could not initalize curl handle");
    }

    std::string url = std::format("http://localhost:8000/delete/{}", jobname);
    std::string response_string;

    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, CurlWrapper::write_data);
    curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response_string);

    CURLcode errorCode = curl_easy_perform(handle);

    long code;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &code);

    curl_easy_cleanup(handle);
    if (errorCode){
        return std::unexpected("Curl Error: " + std::to_string(errorCode));
    }
    return CurlData{json::parse(response_string), code};
}

class ApiTest : public ::testing::Test {

    static std::thread apiThread_;
    static TrafficApi apiRunner_;

    static void createInputFile() {
        YAML::Node cfg;
        cfg["type"] = "continuous";
        cfg["time"] = 40;
        cfg["timestep"] = 1;

        // Driver params (From traffic flow book example)
        cfg["driverType"] = "Gipps";
        cfg["driverParams"]["a"] = 1.981;
        cfg["driverParams"]["b"] = -2.8955;
        cfg["driverParams"]["bmax"] = -5.505;
        cfg["driverParams"]["a_stdev"] = 0.1;
        cfg["driverParams"]["a_stdev"] = 0.1;
        cfg["driverParams"]["bmax_stdev"] = 0.1;

        cfg["flow"]["rate"] = 600;
        cfg["flow"]["v0"] = 30;
        cfg["flow"]["vdes"] = 35;

        cfg["leadCar"]["leadType"] = "function";
        cfg["leadCar"]["function"]["sine"]["a"] = 5.0;
        cfg["leadCar"]["function"]["sine"]["b"] = 0.04;
        cfg["leadCar"]["function"]["sine"]["c"] = 40;
        cfg["leadCar"]["v0"] = 40;
        cfg["leadCar"]["vdes"] = 45;

        cfg["end"] = 1500;
        cfg["logtype"] = "test";

        cfg["jobname"] = "apiTest";
        cfg["seed"] = 105;
        YAML::Emitter cfgyaml;
        std::ofstream fileout("apiConfig.yml");
        cfgyaml << cfg;
        fileout << cfgyaml.c_str();
    }

    protected:

    static void SetUpTestSuite(){    
        curl_global_init(CURL_GLOBAL_DEFAULT);  
        // Create Input File
        createInputFile();
        
        // Init oatpp Environment 
        oatpp::base::Environment::init();
        apiThread_ = std::thread([](){apiRunner_.run();});
        
        // Sleep for 1 second to let the API finish starting up
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    static void TearDownTestSuite(){

        curl_global_cleanup();
        // Cleanup Input File. 
        if (std::filesystem::exists("apiConfig.yml")){
            std::filesystem::remove("apiConfig.yml");
        }
        
        /* Destroy oatpp Environment */
        apiRunner_.closeServer();
        apiThread_.join();
        oatpp::base::Environment::destroy();
    }
};

std::thread ApiTest::apiThread_;
TrafficApi ApiTest::apiRunner_(true);


TEST_F(ApiTest, Waiting){

    CurlWrapper requester;
    CurlResponse response = requester.queryJobs();
    ASSERT_TRUE(response.has_value()) << std::format("Error Querying Jobs: {}", response.error());
    ASSERT_EQ(response.value().code, 200);

    // No check for return code here, 200 and 400 are both valid. Just clearing out the DB. 
    response = requester.deleteJob("apiTest");
    ASSERT_TRUE(response.has_value()) << std::format("Error Deleting Job: {}", response.error());
    

    size_t initialNumJobs = response.value().jsonData["jobs"].size();
    response = requester.postJob("apiTest", std::filesystem::absolute("./apiConfig.yml"));
    ASSERT_TRUE(response.has_value()) << std::format("Error Submitting Simulation: {}", response.error());

    ASSERT_EQ(response.value().code, 200);
    json data = response.value().jsonData;

    ASSERT_EQ(data["jobname"], "apiTest");
    ASSERT_EQ(data["configpath"], std::filesystem::absolute("./apiConfig.yml").string()); // Should return the config path back
    

    // Wait 1 second to let the job finish running. Would be cool to be able to check the status of the job from the api...
    std::this_thread::sleep_for(std::chrono::seconds(1));
    response = requester.queryJob("apiTest");
    ASSERT_TRUE(response.has_value()) << std::format("Error Querying for Job: {}", response.error());
    EXPECT_EQ(response.value().code, 200);
    ASSERT_EQ(response.value().jsonData["jobname"], "apiTest");

    response = requester.queryJobs();
    ASSERT_TRUE(response.has_value()) << std::format("Error Querying For Jobs: {}", response.error());
    ASSERT_EQ(response->jsonData["jobs"].size(), initialNumJobs + 1);

    response = requester.queryCarDatas("apiTest");
    ASSERT_TRUE(response.has_value()) << std::format("Error Querying Car Data {}", response.error());
    ASSERT_EQ(response.value().code, 200);
    ASSERT_EQ(response->jsonData["cars"].size(), 6); // number of cars in the job, not number of jobs


    response = requester.queryRawDatas("apiTest");
    ASSERT_TRUE(response.has_value()) << std::format("Error Querying Raw Data: {}", response.error());
    ASSERT_EQ(response.value().code, 200);
    for (const json& carXVT : response->jsonData["data"]){
        std::vector<float> xs = carXVT["x"];
        ASSERT_TRUE(std::ranges::is_sorted(xs));
    }

    response = requester.deleteJob("apiTest");
    ASSERT_TRUE(response.has_value()) << std::format("Error Deleting Job: {}", response.error());
    
    EXPECT_EQ(response.value().jsonData["msg"], "Successfully deleted apiTest");

}