import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

DT = 5

dir = "/Users/ryan/projects/TrafficSimulation/cont/car8.csv"
df = pd.read_csv(dir)

times = []
# rows represents the times, cols represents the x values. 
# ns[15][40] = how many cars pass through x = 40 at t= 15
max_x = 100
max_t = 100
ns = [[0 for _ in range(max_t)] for _ in range(max_x)]
x_ind = 0
for x in range(max_x):
    cur_x = df["x"][x_ind]
    if cur_x < x:
        x_ind += 1
        cur_x  = df["x"][x_ind]
    t = df["t"][x_ind]
    v = df["v"][x_ind]

    times.append((x - cur_x)/v + t)

# ns = [[0 for _ in range(max_t)] for _ in range(max_x)]

# print(times)
# plt.plot(list(range(len(times))), times)
# plt.show()
for x in range(max_t):
    for t in range(max_x):
        if times[x] >= t and times[x] <= (t + DT):
            ns[x][t] += 1

plt.imshow(ns)
# plt.grid(True)
# plt.xlim(0, 9)
# plt.ylim(0,5)
# plt.savefig("fig1.png")
plt.ylabel("Position x")
plt.xlabel("Time")
plt.show()

# print this out to a CSV for easier testing, or values separated by 
def writeData(ns):
    f = open("data.csv")
    for r in ns:
        for c in ns:
            f.write(str(ns[r][c]) + ",")
 # have the timestamps which a car passes through x = 0 to 100. 
 # do this for 
# print(times)
