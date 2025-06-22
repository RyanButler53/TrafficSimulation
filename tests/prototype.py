# implement each of the car following models in python
import math
import matplotlib.pyplot as plt
import numpy as np

# gipps and 

MPH2MPS = 0.44704
FT2M = 3.28084

# Freedom Units initial exploration
def gipps2(v, v_des, gap, v_l, lead_len) -> float: 
    """Returns the final velocity"""
    a = 6.5 # accel in ft/s^2
    dt = 1 # timestep
    b = -9.5 # max braking
    b_max = -11.5 # max braking of lead

    ratio = v/v_des
    free_road = v + 2.5 * a * dt * (1 - ratio) * math.sqrt(0.025 + ratio)
    bt = b * dt
    braking = bt + math.sqrt((bt * bt) - b * ((2 * (gap+lead_len)) - v*dt - (v_l* v_l/b_max)))
    print(f"free: {free_road/1.46667}, braking: {braking/1.46667}")
    return min(free_road, braking)

# Metric Units
def gipps(v,v_des, gap, v_l, lead_len):
    """Returns final velocity and dx"""
    a = 1.981 # accel in m/s^2
    dt = 1 # timestep
    b = -2.8955 # max braking
    b_max = -3.505 # max braking of lead

    ratio = v/v_des
    free_road = v + 2.5 * a * dt * (1 - ratio) * math.sqrt(0.025 + ratio)
    bt = b * dt
    braking = bt + math.sqrt((bt * bt) - b * ((2 * (gap+lead_len)) - v*dt - (v_l* v_l/b_max)))
    print(f"free: {free_road * FT2M}, braking: {braking * FT2M}")
    return min(free_road, braking)


def idm(v, v_des, gap, v_l, lead_len):
    s0 = 2
    dt = 1
    a = 1
    b = 1.5
    dv = v_l - v
    ratio = v/v_des
    s_des = s0 + max(0, v * dt + (v * -dv)/(2 * math.sqrt(2*a*b)))
    final_accel = a * (1 - ratio**4 - (s_des/(gap+lead_len))**2)
    return v + final_accel * dt

# lead speeds
mph = [52.37, 50.95, 48,46,44,42,41,39,37,35,33,31,29,25,22,19,21, 24, 26, 29, 31, 33, 36, 39, 42, 45, 49, 50.91, 51.5, 50.67]

def plot(xleads, xfollows, vleads, vfollows):
    fig, ax = plt.subplots(2,1)
    times = list(range(30))
    ax[0].plot(times, np.array(xleads), color='orange')
    ax[0].plot(times, np.array(xfollows), color='blue')

    ax[1].plot(times, np.array(vleads), color='orange')
    ax[1].plot(times, np.array(vfollows), color='blue')

    plt.show()

def sim1(mph):
    speeds = [x * 1.46667 for x in mph] # ft/s

    xlead = 0 # x value
    xfollow = -200 # value

    vlead = speeds[0]
    vfollow = 54.3 * 1.46667 # ft/s
    gaps = []

    # track these
    xleads = [xlead]
    xfollows = [xfollow]
    vfollows = [vfollow]

    vdes = 75 * 1.4667
    dt = 1
    lead_len = 25
    for i in range(29):
        vlead = speeds[i]
        gap = xlead - xfollow
        gaps.append(gap)
        next_v = gipps2(vfollow, vdes, gap, vlead, lead_len)
        # update and store
        xlead += vlead * dt # dt = 1
        xfollow += (vfollow * dt) # update with the old velocity
        vfollow = next_v

        print(f"xlead:{xlead}, xfollow:{xfollow}, vfollow: {vfollow /1.46667}, vlead: {vlead/1.46667}, gap: {gap}")

        xleads.append(xlead) # convert to ft per second
        xfollows.append(xfollow)
        vfollows.append(vfollow)
        gaps.append(gap)

    plot(xleads, xfollows, speeds, vfollows)
    
def sim(mph, updateFunc):
    """Same as sim1 but in metric"""

    speeds = [x * MPH2MPS for x in mph] # m/s

    xlead = 0 # x value
    xfollow = -200 / FT2M # value

    vlead = speeds[0]
    vfollow = 54.3 * MPH2MPS # m/s

    # track these
    gaps = []
    xleads = [xlead]
    xfollows = [xfollow]
    vfollows = [vfollow]

    # desired speed
    vdes = 75 * MPH2MPS

    dt = 1
    lead_len = 25 / FT2M

    for i in range(29):
        vlead = speeds[i]
        gap = xlead - xfollow
        gaps.append(gap)
        if (gap < 0): 
            print (f"ACCIDENT ON TIMESTEP {i}")
            return
        next_v = updateFunc(vfollow, vdes, gap, vlead, lead_len)
        # update and store
        xlead += vlead * dt # dt = 1
        xfollow += (vfollow * dt) # update with the old velocity
        vfollow = next_v

        print(f"xlead:{xlead * FT2M}, xfollow:{xfollow * FT2M}, vfollow: {vfollow * FT2M}, vlead: {vlead*FT2M}, gap: {gap*FT2M}")

        xleads.append(xlead) # convert to ft per second
        xfollows.append(xfollow)
        vfollows.append(vfollow)
        gaps.append(gap)

    plot(xleads, xfollows, speeds, vfollows)

sim1(mph)
sim(mph, gipps)
sim(mph, idm)




