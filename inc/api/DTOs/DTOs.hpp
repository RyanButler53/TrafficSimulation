#pragma once
#include "oatpp/core/data/mapping/type/Object.hpp"

#include "oatpp/core/macro/codegen.hpp"


// Follow Model Params
#include OATPP_CODEGEN_BEGIN(DTO)
class FollowModelDTO : public oatpp::DTO {

  DTO_INIT(FollowModelDTO, DTO);

  DTO_FIELD(String, model);
  DTO_FIELD(Float64,  a);
  DTO_FIELD(Float64, b);
};
#include OATPP_CODEGEN_END(DTO)

// CAR METADATA

// Car Metadata
#include OATPP_CODEGEN_BEGIN(DTO)

class CarMetadataDTO : public oatpp::DTO {

    DTO_INIT(CarMetadataDTO, DTO);
  
    DTO_FIELD(Int64, carid);
    DTO_FIELD(String, leadStrategy);
    DTO_FIELD(String, followStrategy);
    DTO_FIELD(Object<FollowModelDTO>, followModel);

};

#include OATPP_CODEGEN_END(DTO)

// List of car metadata
#include OATPP_CODEGEN_BEGIN(DTO)

class CarMetadataListDTO : public oatpp::DTO {

  DTO_INIT(CarMetadataListDTO, DTO);

  DTO_FIELD(Int32, numCars);
  DTO_FIELD(Vector<Object<CarMetadataDTO>>, cars);
};

#include OATPP_CODEGEN_END(DTO)

/// RAW DATA

// Car Snapshot Data
#include OATPP_CODEGEN_BEGIN(DTO)
class CarSnapshotDTO : public oatpp::DTO {

    DTO_INIT(CarSnapshotDTO, DTO);
  
    DTO_FIELD(Vector<Float32>, x);   // Status code field
    DTO_FIELD(Vector<Float32>, v);     // Message field
    DTO_FIELD(Vector<Float32>, t);
};

#include OATPP_CODEGEN_END(DTO)

// x,v and t raw data for all cars
#include OATPP_CODEGEN_BEGIN(DTO)
class RawDataDTO : public oatpp::DTO {
   DTO_INIT(RawDataDTO, DTO);

   DTO_FIELD(Vector<Object<CarSnapshotDTO>>, data);
};

#include OATPP_CODEGEN_END(DTO)

// Response to submitting a job. Returns the job name and config path
#include OATPP_CODEGEN_BEGIN(DTO)
class JobSubmitDTO : public oatpp::DTO {
    DTO_INIT(JobSubmitDTO, DTO);
  
    DTO_FIELD(String, jobname);
    DTO_FIELD(String, configpath);
};

#include OATPP_CODEGEN_END(DTO)

// Job Data DTO
#include OATPP_CODEGEN_BEGIN(DTO)
class JobDataDTO : public oatpp::DTO {
  DTO_INIT(JobDataDTO, DTO);

  DTO_FIELD(Boolean, present);
  DTO_FIELD(String, jobname);
  DTO_FIELD(String, cfgfile);

};
#include OATPP_CODEGEN_END(DTO)

// querying for ALL jobs
#include OATPP_CODEGEN_BEGIN(DTO)
class JobDataListDTO : public oatpp::DTO {
  DTO_INIT(JobDataListDTO, DTO);

  DTO_FIELD(Vector<Object<JobDataDTO>>, jobs);

};
#include OATPP_CODEGEN_END(DTO)

// Error message
#include OATPP_CODEGEN_BEGIN(DTO)
class ErrorDTO : public oatpp::DTO {
  DTO_INIT(ErrorDTO, DTO);
  DTO_FIELD(String, errmsg);

};
#include OATPP_CODEGEN_END(DTO)
