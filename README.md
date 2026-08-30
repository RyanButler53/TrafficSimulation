# Traffic Simulator


### Dependencies: 

- PostgreSQL: Main Database system
- libpqxx: C++ libpq wrapper
- yaml-cpp: C++ Yaml parsing library
- oatpp: C++ Api library
- Googletest: C++ Testing library
- openssl (optional): Hashing for regression testing. 
- Curl: For making api requests for Api Testing

Compiler must be compatible with C++23 with `std::views::zip` and `std::expected`

### Database Schema:
There are two databases: TrafficDB and TrafficDBTest
Both have 3 tables: trafficJobs, carData, snapshotData

#### Job Data:

Table name: trafficJobs 

| jobID (int) | configfile (text) | jobname (text) | status (varchar 7) | error (text) | followModel (varchar 5) | numCars (int) |
| --- | --- | --- | --- | --- | --- | --- |
| Unique Job identifier | Configuration file for the simulation | User provided jobname (in input file) | Job Status (Queued, Running Error, Done) | Error message for erroring jobs | Follow model type: Gipps/IDM | Number of total cars in the simulation. 

#### Car Metadata:

Table name: carData

| carID (int) | jobID (int) | Follow Strategy A (float)  | Follow Strategy B (float) | Follow Strategy C (float) | Politeness (float) | Desired Velocity (float) |
| --- | --- | --- | --- | --- | --- | --- |
| Unique Car Id to its simulation | Job ID that the car belongs to | Acceleration parameter to Car Following Strategy | Braking coefficent for car following strategy | C coefficent. C is either max braking for Gipps Car Following model or Minimum allowable gap for Intelligent driver model | Lane Changing Politeness (MOBIL Model) | Desired velocity (v_des)

JobID is a foreign key to the TrafficJobs table. The combination CarID and JobID are gauranteed to be unique. 

#### Car Snapshot Data:
Table name: snapshotData

| carID (int) | jobID (int) | x (float) | v (float) | t (float) | Lane (int) |
| --- | --- | --- | --- | --- | --- |
| Car Id for simulation | Job the car belongs to | position | velocity | timestamp | current lane |


CarID and JobID is a foreign key to CarData's CarID and JobID values. The triplet CarID, JobID and timestamp (t) are gauranteed to be unique. 

#### Lane Segments

Table name: laneSegments

| jobId (int) | segmentStart (float) | segmentEnd (float) | rate (float) | position (int)
| --- | --- | --- | --- | --- | --- |
| Job ID that the segment belongs to | Start of the road segment (m) | End of the road segment (m) | Flow rate (veh/hr). NULL if it is an unpopulated segment | Lane position of the segment (0 represents right lane)

JobID is a foreigh key to the Job ID in the Traffic Jobs table. 

#### Environment Data:

| jobId (int) | x0 (float) | xf (float) | numLanes (int) |
| --- | --- | --- | --- |
| Job ID of the environment | Beginning of the road (m) | End of the road (m). The maximum of all segmentEnds with the same Job ID | Total number of lanes. Represents the maximum value of the "position" parameter in input files |

JobID is a foreigh key to the Job ID in the Traffic Jobs table. There should be only one environment with a given JobID in this table, thus it is a primary key (although it is explicitly marked as one)

# List of Api Endpoints

The API is hosted locally on port 8000 so the base url is `http://localhost:8000`

`GET "/jobs/{jobname}"` -> Returns Jobname and Config file for job name

`GET "/jobs"` -> Returns a list of all the jobs that have been submitted. 

`GET "/data/{job-name}/cars/{id}"` -> Returns data about a specific car for a specific job

`GET "/data/{job-name}/cars/"` -> Returns data about all the cars for a specific job

`GET "/data/{job-name}/raw/"` -> Returns raw snapshot data about all cars for a specific job. This is a _very_ large amount of data!

`GET "/data/{job-name}/raw/{id}"` -> Returns raw snapshot data for a single car in the specified job.

`GET "/data/{job-name}/spatial/"` -> Runs a "spatial" query that returns snapshots occuring between `t0` and `t1` and bewteen `x0` and `x1` 
Requires a query parameter to specify the boundaries:

```
{"t0": 10,
 "t1": 50,
 "x0": 100, 
 "x1": 250}
```

`POST "/submit/{job-name}"` -> Submits a job with the specified job name. The Job name must be unique. Requires a query parameter `{"cfg" : "configfile.yaml"}` to specify the config file. 

`DELETE "/jobs/{jobname}"` -> Deletes the specified job if it exists. 