# Traffic Simulation Tests

## Unit tests:
There are 4 modules with unit tests: 

- Flow Generator
- Lane Info
   - Includes bias calculation and LaneInterval test cases
- Threadsafe Queue
- Parsing
- Environments
   - Includes a comparison of File/DB based logging

## Integration Tests: 

#### DB Reader + Job Manager Test: 
Submits jobs via the Job Manager and reads data from the Database via the DB Reader. 
Compares the values from the Multiple Cars cases to the Single Car cases. 

#### Regression Test: 
Submit one job that uses the file logger. Submit one that uses the Test db logger and compare
Submit one jog with the file logger and one with time series logger and compare output results. 
Submit one job with the time series logger. Runs the same job with 10:1 thinning and checks if the logs match (Requires OpenSSL)
Submit one job with the file logger with a known hash of the outputs. Compare with the known hash.

#### Api Integration Test: (Requires curl and json)
Runs the API in Test mode
Sends api requests and checks the status codes for code 200 
Sends bad api requests and checks for appropriate error messages and code 400

#### Algorithm Test: 
Runs each test case and outputs to the test database. For each job, checks 4 things:
- All cars are moving forward and velocities are positive
- All cars are in the bounds of the lane beginning to end
- There is at least one lane change in both directions
- At least 1 car is generated in all 3 lanes. 

##### Algorithm Test Cases: 
- 3lane: 3 lane case with flow generating in all 3 lanes. No lane closures
- ZeroFlow: 2 lane case where one lane has zero flow and ensures lane changes can happen
- LaneClosure: Has lane 0 closed from x = 2000 to x = 5000. Tests that no car is in lane 0 in that range. 

## Running the tests:

To run all: `ctest . `

To run unit tests: `ctest -L unit`

To run integration tests `ctest -L integration`

Unit tests should finish in under a second but the integration tests will take about 20-25 seconds. 