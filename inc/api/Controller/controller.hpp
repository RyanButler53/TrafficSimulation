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

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/core/data/mapping/type/Object.hpp"


using namespace oatpp;

#define DATABASE_ERROR  auto err = ErrorDTO::createShared(); \
                err->errmsg = e.what(); \
                return createDtoResponse(Status::CODE_404, err);

#include OATPP_CODEGEN_BEGIN(ApiController) ///< Begin Codegen

class Controller : public oatpp::web::server::api::ApiController {
    
    DBReader reader_;
    JobManager manager_;
    
    public: 

    Controller(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, mapper))
        // Call base class constructor. Pass the object mapper component
        :oatpp::web::server::api::ApiController(mapper){}

    // Get simple information about a job
    ENDPOINT("GET", "/jobs/{job-name}", queryJobs, PATH(String, jobname, "job-name")){
        auto response = JobDataDTO::createShared();
        // Query Database for job
        JobData data;
        try  {
            data = reader_.queryJobs(jobname);
        } catch(const DBReadError& e) {
            DATABASE_ERROR
        }
        
        response->present = true;
        response->jobname = data.jobName_;
        response->cfgfile = data.cfgPath_;
        return createDtoResponse(Status::CODE_200, response);
    }

    // Querying for whatever jobs are available
    ENDPOINT("GET", "/jobs", allJobs){
        auto response = JobDataListDTO::createShared();
        std::vector<JobData> jobs = reader_.queryJobs();
        for (JobData& j : jobs){
            auto job = JobDataDTO::createShared();
            job->jobname = j.jobName_;
            job->cfgfile = j.cfgPath_;
            response->jobs->push_back(job);
        }
        return createDtoResponse(Status::CODE_200, response);
    }
   
    //  Getting all information about a car
    ENDPOINT("GET", "/data/{job-name}/cars/{id}", getCarData, 
            PATH(String, job, "job-name"), PATH(Int32, id, "id")) {
        auto response = CarMetadataDTO::createShared();
        auto followModel = FollowModelDTO::createShared();
        
        CarMetadata cm;
        try  {
            cm = reader_.queryCars(job, id);
        } catch(const DBReadError& e) {
            DATABASE_ERROR
        } 
        response->leadStrategy = cm.lead_;
        response->followStrategy = cm.follow_;

        followModel->a = cm.model_.a_;
        followModel->b = cm.model_.b_;
        response->followModel = followModel;
        response->carid = id;

        return createDtoResponse(Status::CODE_200, response);
    }

    // Get all information about ALL cars
    ENDPOINT("GET", "/data/{job-name}/cars/", getAllCarData, 
            PATH(String, job, "job-name")){
        auto response = CarMetadataListDTO::createShared();
        std::vector<CarMetadata> cars; 

        try  {
            cars = reader_.queryCars(job);
        } catch(const DBReadError& e) {
            DATABASE_ERROR
        }

        response->numCars = cars.size();
        for (CarMetadata& cm : cars){
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

        return createDtoResponse(Status::CODE_200, response);
    }

    // Getting raw data about one car
    ENDPOINT("GET", "/data/{job-name}/cars/raw/{id}", carRawData, 
            PATH(String, job, "job-name"), PATH(Int32, car, "id")){
        
        auto response = CarSnapshotDTO::createShared();
        RawData raw;

        try {
            raw = reader_.queryData(job, car);
        } catch(const DBReadError& e) {
            DATABASE_ERROR
        }
        response->x->resize(raw.x_.size());
        response->v->resize(raw.v_.size());
        response->t->resize(raw.t_.size());
        std::ranges::transform(raw.x_, response->x->begin(), [](float x){return Float32(x);});
        std::ranges::transform(raw.v_, response->v->begin(), [](float v){return Float32(v);});
        std::ranges::transform(raw.t_, response->t->begin(), [](float t){return Float32(t);});

        return createDtoResponse(Status::CODE_200, response);
    }

    // Getting raw data about ALL cars (this is a big call)
    ENDPOINT("GET", "/data/{job-name}/cars/raw", carRawDataAll, 
        PATH(String, job, "job-name")){
    auto response = RawDataDTO::createShared();

    std::vector<RawData> raw;
    try {
        raw = reader_.queryData(job);
    } catch(const DBReadError& e) {
        DATABASE_ERROR
    }
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

    return createDtoResponse(Status::CODE_200, response);
    }

    // Submitting a job. Must check if jobname is unique. 
    ENDPOINT("POST", "/submit/{job-name}", submitJob, 
        PATH(String, jobname, "job-name"),
        QUERY(String, cfg, "config") ){
    auto response = JobSubmitDTO::createShared();
    try {
        if (reader_.checkJobName(jobname)){
            response->jobname = jobname;
            response->configpath = cfg;
            manager_.submit(cfg);
            return createDtoResponse(Status::CODE_200, response);
        } else {
            auto err = ErrorDTO::createShared();
            err->errmsg = "Job with the same name already exists!";
            return createDtoResponse(Status::CODE_409, err);
        }
    }
    catch(const DBReadError& e) {
        DATABASE_ERROR
    }
    }
};

#include OATPP_CODEGEN_END(ApiController) ///< End Codegen
