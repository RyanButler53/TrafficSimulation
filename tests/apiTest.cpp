#include <array>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>
#include <format>
#include <expected>
#include <sstream>
#include <print>
#include <optional>

#include <gtest/gtest.h>
#include "api/api.hpp"  // Include API Running
#include "oatpp/network/Server.hpp"     
#include "yaml-cpp/yaml.h"
#include <nlohmann/json.hpp>
#include "curl/curl.h"
#include "testUtil.hpp"


using json = nlohmann::json;

struct CurlData{
    json jsonData;
    long code;

    // Return the error string. Only valid if the code is not 200. 
    std::string error() {
        return jsonData["errmsg"];
    }
};

using CurlResponse = std::expected<CurlData, std::string>;

// Class to wrap all the Curl Calls. 
class CurlWrapper {

    static size_t write_data(void *ptr, size_t size, size_t nmemb, std::string* data) {
        data->append((char*) ptr, size * nmemb);
        return size * nmemb;
    }
    
    /**
     * @brief Run a simple get query for the provided URL. 
     * @details Adds the https://localhost:8000
     * 
     * @return CurlResponse with the Json Data or an error string 
     */
    CurlResponse getQuery(std::string url);

    public: 

    CurlResponse queryJobs(){return getQuery("/jobs");}; // GET /jobs
    CurlResponse queryJob(std::string jobname) {return getQuery(std::format("/jobs/{}", jobname));} // get /jobs/{jobname}
    CurlResponse queryCarData(std::string jobname, size_t id){return getQuery(std::format("/data/{}/cars/{}", jobname, id));} // GET /data/{jobname}/cars/id
    CurlResponse queryCarDatas(std::string jobname) {return getQuery(std::format("/data/{}/cars", jobname));} // GET /data/{jobname}
    CurlResponse queryRawData(std::string jobname, size_t id) {return getQuery(std::format("/data/{}/raw/{}", jobname, id));} // GET /data/{jobname}/cars/raw/id
    CurlResponse queryRawDatas(std::string jobname) {return getQuery(std::format("/data/{}/raw", jobname));}; // GET /data/{jobname}/cars/raw
    CurlResponse queryTimeSeries(std::string jobname, std::optional<double> t0 = std::nullopt,
                                                      std::optional<double> t1 = std::nullopt,
                                                      std::optional<double> x0 = std::nullopt, 
                                                      std::optional<double> x1 = std::nullopt);
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

CurlResponse CurlWrapper::queryTimeSeries(std::string jobname, std::optional<double> t0, std::optional<double> t1,
                                          std::optional<double> x0, std::optional<double> x1){

    CURL* handle = curl_easy_init();
    if (!handle){
        return std::unexpected("Could not initalize curl handle");
    }
    std::string url = std::format("/data/{}/spatial?", jobname);
    if (t0) {url += std::format("t0={}&", *t0);}
    if (t1) {url += std::format("t1={}&", *t1);}
    if (x0) {url += std::format("x0={}&", *x0);}
    if (x1) {url += std::format("x1={}&", *x1);}

    if (url.back() == '&'){
        url.pop_back();
    }
    return getQuery(url);
}

CurlResponse CurlWrapper::postJob(std::string jobname, std::filesystem::path cfgpath) {
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
        YAML::Node cfg = TestUtil::getConfigNode();

        cfg["jobname"] = "apiTest";
        cfg["seed"] = 105;
        cfg["logtype"]= "test";
        TestUtil::configToFile(cfg, "apiConfig.yml");

        YAML::Node cfg2 = TestUtil::getConfigNode_3Lane();
        cfg2["jobname"] = "apiTestTimeSeries";
        cfg2["seed"] = 140;
        cfg2["logtype"]= "test";
        TestUtil::configToFile(cfg2, "apiTestTimeSeries.yml");
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
        if (std::filesystem::exists("apiTestTimeSeries.yml")){
            std::filesystem::remove("apiTestTimeSeries.yml");
        }
        
        /* Destroy oatpp Environment */
        apiRunner_.closeServer();
        apiThread_.join();
        oatpp::base::Environment::destroy();
    }
};

std::thread ApiTest::apiThread_;
TrafficApi ApiTest::apiRunner_(true);


TEST_F(ApiTest, ValidRequests){

    CurlWrapper requester;
    CurlResponse response = requester.queryJobs();
    ASSERT_TRUE(response.has_value()) << std::format("Error Querying All Jobs: {}", response.error());
    ASSERT_EQ(response->code, 200) << std::format("Error Querying All Jobs: {}", response->error());
    size_t initialNumJobs = response.value().jsonData["jobs"].size();

    // No check for return code here, 200 and 400 are both valid. Just clearing out the DB. 
    response = requester.deleteJob("apiTest");
    ASSERT_TRUE(response.has_value()) << std::format("Error Deleting Job: {}", response.error());

    response = requester.postJob("apiTest", std::filesystem::absolute("./apiConfig.yml"));
    ASSERT_TRUE(response.has_value()) << std::format("Error Submitting Simulation: {}", response.error());

    EXPECT_EQ(response->code, 200);
    json data = response->jsonData;

    ASSERT_EQ(data["jobname"], "apiTest");
    ASSERT_EQ(data["configpath"], std::filesystem::absolute("./apiConfig.yml").string()); // Should return the config path back
    

    // Wait 1 second to let the job finish running. Would be cool to be able to check the status of the job from the api...
    response = requester.queryJob("apiTest");
    while(response->jsonData["status"] != "DONE"){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        response = requester.queryJob("apiTest");
    }

    ASSERT_TRUE(response.has_value()) << std::format("Error Querying for Job: {}", response.error());
    EXPECT_EQ(response->code, 200);
    EXPECT_EQ(response->jsonData["jobname"], "apiTest");
    EXPECT_EQ(response->jsonData["driverModel"], "Gipps");
    EXPECT_EQ(response->jsonData["status"], "DONE");

    std::fstream in(std::string(DATA_DIR) + "/api_case_ncars.txt");
    std::string ncars;
    std::getline(in, ncars);
    size_t ncars_expected = std::stoull(ncars);
    EXPECT_EQ(response->jsonData["numCars"], ncars_expected) << "Case is known to have " << ncars_expected << " cars"; // Expect roughly 50 cars. 


    response = requester.queryJobs();
    ASSERT_TRUE(response.has_value()) << std::format("Error Querying For Jobs: {}", response.error());
    ASSERT_EQ(response->jsonData["jobs"].size(), initialNumJobs + 1);

    response = requester.queryCarDatas("apiTest");
    ASSERT_TRUE(response.has_value()) << std::format("Error Querying Car Data {}", response.error());
    EXPECT_EQ(response->code, 200);

    // Homogeneous traffic
    json carMetadata = response->jsonData["cars"];
    for (size_t i = 0; i < carMetadata.size();++i){
        EXPECT_EQ(carMetadata[i]["carid"], i);
        EXPECT_FLOAT_EQ(carMetadata[i]["followModel"]["a"], 1.981 );
        EXPECT_FLOAT_EQ(carMetadata[i]["followModel"]["b"], -2.8955);
        EXPECT_FLOAT_EQ(carMetadata[i]["followModel"]["c"], -5.505);
        EXPECT_FLOAT_EQ(carMetadata[i]["politeness"], 0.2);
    }

    std::array<float, 2> desiredVelocities {35.0, 38.0};
    response = requester.queryRawDatas("apiTest");
    ASSERT_TRUE(response.has_value()) << std::format("Error Querying Raw Data: {}", response.error());
    EXPECT_EQ(response->code, 200);
    size_t id = 0;
    for (const json& carXVT : response->jsonData["data"]){
        std::vector<float> xs = carXVT["x"];
        int initialLane = carXVT["l"][0];
        // Check if x values are always increasing and the desired velocities are correct
        ASSERT_TRUE(std::is_sorted(xs.begin(), xs.end()));
        EXPECT_EQ(carMetadata[id]["desired_vel"], desiredVelocities[initialLane]);
        ++id;
    }

    // Query for car not present in the job
    response = requester.queryCarData("apiTest", 1500);
    ASSERT_TRUE(response.has_value()) << std::format("Error Querying for apiTest job: {}", response.error());

    EXPECT_EQ(response->code, 400) << "Found data for a car that shouldn't exist!";
    EXPECT_EQ(response->error(), "No car with id 1500 in job named apiTest");

    response = requester.deleteJob("apiTest");
    ASSERT_TRUE(response.has_value()) << std::format("Error Deleting Job: {}", response.error());
    
    EXPECT_EQ(response->jsonData["msg"], "Successfully deleted apiTest");
}

TEST_F(ApiTest, TimeSeriesRequests){

    CurlWrapper requester;

    // Don't check for return code since this is not gauranteed to be in the DB. 
    CurlResponse response = requester.deleteJob("apiTestTimeSeries");
    ASSERT_TRUE(response.has_value()) << std::format("Error Deleting Job: {}", response.error());

    response = requester.postJob("apiTestTimeSeries", std::filesystem::absolute("./apiTestTimeSeries.yml"));
    ASSERT_EQ(response->code, 200) << std::format("Error posting job: {}", response->error());

    response = requester.queryJob("apiTestTimeSeries");
    while(response->jsonData["status"] != "DONE"){
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        response = requester.queryJob("apiTestTimeSeries");
        EXPECT_EQ(response->code, 200) << std::format("Error querying job: {}", response->error());
    }
    ASSERT_EQ(response->code, 200) << std::format("Error posting job: {}", response->error());
    
    response = requester.queryTimeSeries("apiTestTimeSeries", 50, 100);
    // The cars must be within 50 and 100 timestep
    ASSERT_EQ(response->code, 200);
    json data = response->jsonData;
    std::vector<float> timestamps = data["timestamps"];
    EXPECT_EQ(timestamps.size(), 51);
    EXPECT_EQ(data["snapshots"].size(), 51);


    response = requester.deleteJob("apiTestTimeSeries");
    ASSERT_EQ(response->code, 200) << std::format("Error deleting job: {}", response->error());

    response = requester.queryTimeSeries("apiTestTimeSeries", std::nullopt, std::nullopt, 750, 1000);
    ASSERT_EQ(response->code, 200) << "Error in spatial query" << std::endl;

    data = response->jsonData;
    for (const json& perCar  : response->jsonData["snapshots"]){
        EXPECT_GT(perCar["x"], 600.0f);
        EXPECT_LT(perCar["x"], 750.0f);
    }

    // Clean up
    response = requester.deleteJob("apiTestTimeSeries");
    ASSERT_TRUE(response.has_value()) << std::format("Error Deleting Job: {}", response.error());

}

TEST_F(ApiTest, ErrorRequests){

    CurlWrapper requester;
    CurlResponse response = requester.queryJobs();
    ASSERT_TRUE(response.has_value()) << std::format("Error Querying All Jobs: {}", response.error());
    ASSERT_EQ(response.value().code, 200);
    size_t initialNumJobs = response.value().jsonData["jobs"].size();

    response = requester.postJob("ApiConfig", "./apiConfig.yml");
    ASSERT_TRUE(response.has_value()) << std::format("Error posting for invalid Job: {}", response.error());
    ASSERT_EQ(response->code, 400) << "Submitted a job without an absolute path?";
    EXPECT_EQ(response->error(), "./apiConfig.yml is not an absolute path. Must pass in an absolute path!");

    // No check for return code here, 200 and 400 are both valid. Just clearing out the DB. 
    response = requester.deleteJob("apiTest");
    ASSERT_TRUE(response.has_value()) << std::format("Error Deleting Job: {}", response.error());

    // Delete job that doesn't exist
    response = requester.deleteJob("apiTest");
    ASSERT_TRUE(response.has_value()) << std::format("Error Deleting Job: {}", response.error());
    ASSERT_EQ(response->code, 400) << "Found ApiTest, which is supposed to be deleted!";

    // Job data with incorrect job name
    response = requester.queryJob("wrongJobName");
    ASSERT_TRUE(response.has_value()) << std::format("Error Querying for fake Job: {}", response.error());

    ASSERT_EQ(response.value().code, 400) << "Found a job that shouldn't exist!";
    EXPECT_EQ(response->error(), "No job named wrongJobName");

    // Car Metadata with incorrect job name
    response = requester.queryCarDatas("wrongJobName");
    ASSERT_TRUE(response.has_value()) << std::format("Error Querying for fake Job: {}", response.error());

    ASSERT_EQ(response->code, 400) << "Found a job that shouldn't exist!";
    EXPECT_EQ(response->error(), "No Data found. Check to see if the job exists");


}