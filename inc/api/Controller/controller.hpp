#pragma once

#include "api/DTOs/DTOs.hpp"
#include "api/structs.hpp"
#include "api/DBReader.hpp"
#include "api/jobManager.hpp"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <fstream>
#include <optional>
#include <ranges>
#include <functional>

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/core/data/mapping/type/Object.hpp"


using namespace oatpp;

#include OATPP_CODEGEN_BEGIN(ApiController) ///< Begin Codegen

class Controller : public oatpp::web::server::api::ApiController {
    
    std::shared_ptr<DBReader> reader_;
    JobManager manager_;
    // Error DTO Prototype in the class
    using ErrorResponse = ErrorDTO::Wrapper;
    ErrorResponse error_ = ErrorDTO::createShared();

    static ErrorDTO::Wrapper translateError(const std::string& errorMsg){
        ErrorDTO::Wrapper error = ErrorDTO::createShared();
        error->errmsg = errorMsg;
        return error;
    }


    template <typename ReturnDTO>
    auto getReturnDto(std::expected<ReturnDTO, ErrorResponse> apiResponse){
        if (apiResponse.has_value()){
            return createDtoResponse(Status::CODE_200, apiResponse.value());
        } else {
            return createDtoResponse(Status::CODE_400, apiResponse.error());
        }
    }
    
    static std::string decodeURL(std::string url){
        std::string decoded = "";
        for (size_t i = 0; i < url.size(); ++i){
            if (url[i] == '%' and i < url.size() - 2){
                char hex1 = url[i+1];
                char hex2 = url[i+2];
                std::stringstream s;
                s << std::hex << hex1 << hex2;
                int hexVal = stoi(s.str(), 0, 16);
                decoded.push_back(char(hexVal));
                i += 2;
            } else {
                decoded.push_back(url[i]);
            }
        }
        return decoded;
    }

    public:

    Controller(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, mapper))
        // Call base class constructor. Pass the object mapper component
        :oatpp::web::server::api::ApiController(mapper){}

    void setDBReader(bool testDB){
        reader_ = std::make_shared<DBReader>(testDB);
    }
    
    // Get simple information about a job
    ENDPOINT("GET", "/jobs/{job-name}", queryJobs, PATH(String, jobname, "job-name")){

        // Query Database for job
        auto data = reader_->queryJobs(jobname);
        // Define translation function between JobData and DTO and translate
        std::function<JobDataDTO::Wrapper(JobData)> translateJobData = [](const JobData& data){
            auto response = JobDataDTO::createShared();
            response->present = true;
            response->jobname = data.jobName_;
            response->cfgfile = data.cfgPath_;
            return response;
        };

        return getReturnDto(data.transform(translateJobData).transform_error(Controller::translateError));
    }

    // Querying for whatever jobs are available
    ENDPOINT("GET", "/jobs", allJobs){
        OATPP_LOGI("Controller", "Getting job information for all jobs");

        std::expected<std::vector<JobData>, std::string> jobs = reader_->queryJobs();
        auto translateJobList = [](const std::vector<JobData>& jobs){ // std::function<JobDataListDTO::Wrapper(std::vector<JobData>)>
            auto response = JobDataListDTO::createShared();
            response->jobs = {};
            for (const JobData& j : jobs){
                auto job = JobDataDTO::createShared();
                job->jobname = j.jobName_;
                job->cfgfile = j.cfgPath_;
                response->jobs->push_back(job);
            }
            return response;
        };
        return getReturnDto(jobs.transform(translateJobList).transform_error(Controller::translateError));
    }
   
    //  Getting all information about a car
    ENDPOINT("GET", "/data/{job-name}/cars/{id}", getCarData, 
            PATH(String, job, "job-name"), PATH(Int32, id, "id")) {

        OATPP_LOGI("Controller", "Getting job information for car %d in %s", int(id), std::string(job).c_str());

        std::expected<CarMetadata, std::string> cm = reader_->queryCars(job, id);

        std::function<CarMetadataDTO::Wrapper(CarMetadata)> translateCarMetadata = [&id](const CarMetadata& cm){
            auto response = CarMetadataDTO::createShared();
            auto followModel = FollowModelDTO::createShared();
            response->leadStrategy = cm.lead_;
            response->followStrategy = cm.follow_;
    
            followModel->a = cm.model_.a_;
            followModel->b = cm.model_.b_;
            response->followModel = followModel;
            response->carid = id; // id
            return response;
        };
        return getReturnDto(cm.transform(translateCarMetadata).transform_error(Controller::translateError));

    }

    // Get all information about ALL cars
    ENDPOINT("GET", "/data/{job-name}/cars/", getAllCarData, 
            PATH(String, job, "job-name")){
        OATPP_LOGI("Controller", "Getting job information for all cars in %s", std::string(job).c_str());

        auto cars = reader_->queryCars(job);
        auto translate = [](std::vector<CarMetadata> carlist){
            auto response = CarMetadataListDTO::createShared();
            response->numCars = carlist.size();
            for (CarMetadata& cm : carlist){
                auto carMeta = CarMetadataDTO::createShared();
                auto followModel = FollowModelDTO::createShared();
                carMeta->carid = cm.id_;
                carMeta->leadStrategy = cm.lead_;
                carMeta->followStrategy = cm.follow_;
        
                followModel->a = cm.model_.a_;
                followModel->b = cm.model_.b_;
                carMeta->followModel = followModel;
                response->cars->push_back(carMeta);
            }
            return response;
        };
        return getReturnDto(cars.transform(translate).transform_error(Controller::translateError));
    }

    // Getting raw data about one car
    ENDPOINT("GET", "/data/{job-name}/cars/raw/{id}", carRawData, 
            PATH(String, job, "job-name"), PATH(Int32, car, "id")){
        OATPP_LOGI("Controller", "Getting raw data for car %d in %s", int(car), std::string(job).c_str());

        auto raw = reader_->queryData(job, car);
        auto translate = [](const RawData& raw){
            auto response = CarSnapshotDTO::createShared();
            response->x->resize(raw.x_.size());
            response->v->resize(raw.v_.size());
            response->t->resize(raw.t_.size());
            std::ranges::transform(raw.x_, response->x->begin(), [](float x){return Float32(x);});
            std::ranges::transform(raw.v_, response->v->begin(), [](float v){return Float32(v);});
            std::ranges::transform(raw.t_, response->t->begin(), [](float t){return Float32(t);});
            return response;
        };
        return getReturnDto(raw.transform(translate).transform_error(Controller::translateError));
    }

    // Getting raw data about ALL cars (this is a big call)
    ENDPOINT("GET", "/data/{job-name}/cars/raw", carRawDataAll, 
        PATH(String, job, "job-name")){

    auto raw = reader_->queryData(job);
    auto translate = [](const std::vector<RawData>& raw){
        auto response = RawDataDTO::createShared();
        response->data = {};
        for (const RawData& cardata : raw){
            auto individualCar = CarSnapshotDTO::createShared();
            individualCar->x->resize(cardata.x_.size());
            individualCar->v->resize(cardata.v_.size());
            individualCar->t->resize(cardata.t_.size());
            std::ranges::transform(cardata.x_, individualCar->x->begin(), [](float x){return Float32(x);});
            std::ranges::transform(cardata.v_, individualCar->v->begin(), [](float v){return Float32(v);});
            std::ranges::transform(cardata.t_, individualCar->t->begin(), [](float t){return Float32(t);});
            response->data->push_back(individualCar);
        }
        return response;
    };

    return getReturnDto(raw.transform(translate).transform_error(Controller::translateError));
}

    // Submitting a job. Must check if jobname is unique. 
    ENDPOINT("POST", "/submit/{job-name}", submitJob, 
        PATH(String, jobname, "job-name"),
        QUERY(String, cfg, "config") ){ // path must be an ABSOLUTE PATH 

    std::string path = std::string(Controller::decodeURL(cfg));
    if (std::filesystem::path(path).is_relative()){
        auto error = ErrorDTO::createShared();
        error->errmsg = path + " is not an absolute path. Must pass in an absolute path!";
        return createDtoResponse(Status::CODE_400, error);
    }

    std::string msg = std::format("Submitting Job {}", std::string(jobname));
    OATPP_LOGI("Controller", "%s", msg.c_str());
    std::expected<int, std::string> exists = reader_->getJobId(jobname);

    // Exists has value -> jobid has been found and jobname is taken. 
    if (exists.has_value()){
        auto error = ErrorDTO::createShared();
        error->errmsg = "Job with this name already exists";
        return createDtoResponse(Status::CODE_409, error);
        // Job is not found . Submit one. 
    } else if (exists.error() == std::format("Job {} not found", std::string(jobname))) {
        std::expected<uint32_t, std::string> submit = manager_.submit(path);
        if (!submit){
            auto error = ErrorDTO::createShared();
            error->errmsg = submit.error();
            return createDtoResponse(Status::CODE_400, error);
        }
        auto response = JobSubmitDTO::createShared();
        response->jobname = jobname;
        response->configpath = path;
        response->jobID = submit.value();
        return createDtoResponse(Status::CODE_400, response);
    } else {
        auto error = ErrorDTO::createShared();
        error->errmsg = exists.error();
        return createDtoResponse(Status::CODE_404, error); 
    }
    }

    ENDPOINT("DELETE", "/delete/{job-name}", deleteJob, 
        PATH(String, jobname, "job-name")){
            // Delete job handles checking if it exists. 
            std::expected<void, std::string> result = reader_->deleteJob(jobname);
            if (result.has_value()){
                // Return an error code
                auto response = DeleteDTO::createShared();
                response->msg = "Successfully deleted " + jobname;
                return createDtoResponse(Status::CODE_200, response);
            } else {
                auto response = ErrorDTO::createShared();
                response->errmsg = result.error();
                return createDtoResponse(Status::CODE_400, response); 
            }
        }
};

#include OATPP_CODEGEN_END(ApiController) ///< End Codegen
