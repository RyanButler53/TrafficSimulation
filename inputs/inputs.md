## Input Files
Files in this folder are sample input files to the traffic simulator. 

They can all be run as 
`./traffic <input>.yml`

### Files in this folder: 
- `2lane.yml`: Simulates two lanes with one starting from a standstill and the other starting from 35 m/s
- `3lane.yml`: Simulates 3 lanes of traffic for 8000 seconds. All 3 lanes start near the desired velocityr
- `zeroFlow.yml`: Simulates 2 lanes of traffic but the left lane has no incoming flow. 
- `lane_closure.yml` Simulates having lanes closed and opening another lane. The right lane is closed from x = 2000 to x = 5000 and a left lane opens at x = 6000. 