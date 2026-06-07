# Traffic Simulator

Car factory: Creates cars with different distributions of aggressiveness, braking and reaction times. 

Roadmap:
Piecewise Functions
Valuable Metrics (probably traffic flow at a specific X at a given time)
Car Factory can be randomized


Add actual test for postgres integration
Add integration test for API access to DB

Dependencies: 

- PostgreSQL: Main Database system
- libpqxx: C++ libpq wrapper
- yaml-cpp: C++ Yaml parsing library
- oatpp: C++ Api library
- Googletest: C++ Testing library
- openssl (optional): Hashing for regression testing. 

Compiler must be compatible with C++23 with `std::views::zip` and `std::expected`

### Database Schema:
There are two databases: TrafficDB and TrafficDBTest
Both have 3 tables: trafficJobs, carData, snapshotData

TrafficJobs: 

| jobID (int) | configfile (text) | jobname (text) | status (varchar 7) | error (text) | followModel (varchar 5) | numCars (int) |
| --- | --- | --- | --- | --- | --- | --- |
| Unique Job identifier | Configuration file for the simulation | User provided jobname (in input file) | Job Status (Queued, Running Error, Done) | Error message for erroring jobs | Follow model type: Gipps/IDM | Number of total cars in the simulation. 

CarData:

| carID (int) | jobID (int) | Follow Strategy A (float)  | Follow Strategy B (float) | Follow Strategy C (float) | Politeness (float) |
| --- | --- | --- | --- | --- | --- |
| Unique Car Id to its simulation | Job ID that the car belongs to | Acceleration parameter to Car Following Strategy | Braking coefficent for car following strategy | C coefficent. C is either max braking for Gipps Car Following model or Minimum allowable gap for Intelligent driver model | Lane Changing Politeness (MOBIL Model) |

JobID is a foreign key to the TrafficJobs table. The combination CarID and JobID are gauranteed to be unique. 

Snapshot Data:

| carID (int) | jobID (int) | x (float) | v (float) | t (float) | Lane (int) |
| --- | --- | --- | --- | --- | --- |
| Car Id for simulation | Job the car belongs to | position | velocity | timestamp | current lane |


CarID and JobID is a foreign key to CarData's CarID and JobID values. The triplet CarID, JobID and timestamp (t) are gauranteed to be unique. 

# List of Api Endpoints

The API is hosted locally on port 8000 so the base url is `http://localhost:8000`

`GET "/jobs/{jobname}"` -> Returns Jobname and Config file for job name

`GET "/jobs"` -> Returns a list of all the jobs that have been submitted. 

`GET "/data/{job-name}/cars/{id}"` -> Returns data about a specific car for a specific job

`GET "/data/{job-name}/cars/"` -> Returns data about all the cars for a specific job

`GET "/data/{job-name}/raw/"` -> Returns raw snapshot data about all cars for a specific job. This is a _very_ large amount of data!

`GET "/data/{job-name}/raw/{id}"` -> Returns raw snapshot data for a single car in the specified job. 

`POST "/submit/{job-name}"` -> Submits a job with the specified job name. The Job name must be unique. Requires a query parameter `{"cfg" : "configfile.yaml"}` to specify the config file. 

`DELETE "/jobs/{jobname}"` -> Deletes the specified job if it exists. 