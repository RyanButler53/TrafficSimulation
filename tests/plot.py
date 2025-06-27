# Point to a directory of carx.txt files. Then graph x vs t and v vs t
import sys
import matplotlib.pyplot as plt
import os
import re
import pandas as pd

if len(sys.argv) == 1:
    path = "sims"
else:
    path = sys.argv[1]

cars = os.listdir(path)


fig, ax = plt.subplots(2,1)
ax[0].set_title("Position vs time")
ax[1].set_title("Velocity vs time")
ax[0].grid(True)
ax[1].grid(True)
for carfile in cars:
    carid = int(re.findall(r"\d+", carfile)[0])
    df = pd.read_csv(os.path.join(path,carfile))
    ax[0].plot(df['t'], df['x'])
    ax[1].plot(df['t'], df['v'])


plt.tight_layout()
plt.show()



