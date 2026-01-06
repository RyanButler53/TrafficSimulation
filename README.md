# Traffic Simulator

Car factory: Creates cars with different distributions of aggressiveness, braking and reaction times. 

Roadmap:
Piecewise Functions
Multiple lanes
Valuable Metrics (probably traffic flow at a specific X at a given time)
Continuous Traffic Flow 
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

| jobID (int) | configfile (text) | jobname (text) |
| --- | --- | --- |
| Unique Job identifier | Configuration file for the simulation | User provided jobname (in input file) |

CarData:

| carID (int) | jobID (int) | Follow Strategy (text) | Lead Strategy (text) |
| --- | --- | --- | --- |
| Unique Car Id to its simulation | Job ID that the car belongs to | Car Following strategy when not in lead | Car Lead strategy when in lead |

JobID is a foreign key to the TrafficJobs table. CarID and JobID are gauranteed to be unique. 

Snapshot Data:

| carID (int) | jobID (int) | x (float) | v (float) | t (float) |
| --- | --- | --- | --- | --- |
| Car Id for simulation | Job the car belongs to | position | velocity | timestamp |


CarID and JobID is a foreign key to CarData's CarID and JobID values. CarID, JobID and timestamp (t) are gauranteed to be unique. 

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