
import requests
import matplotlib.pyplot as plt
import bisect
import numpy as np

class Reader():
    """
    Class to read data about specific jobs. 
    """

    def __init__(self, jobname):
        self.jobname = jobname
        self.base_url = "http://localhost:8000"

    def all_snapshots(self):
        response = requests.get(f"{self.base_url}/data/{self.jobname}/raw/")
        data = response.json()
        if response.status_code != 200:
            raise RuntimeError(f"Error with api request: {data["errmsg"]}")
        return data

    def snapshot(self, car_id:int):
        response = requests.get(f"{self.base_url}/data/{self.jobname}/raw/{car_id}")
        data = response.json()
        if response.status_code != 200:
            raise RuntimeError(f"Error with api request: {data["errmsg"]}")
        return data
    
    def all_metadata(self):
        response = requests.get(f"{self.base_url}//data/{self.jobname}/cars")
        data = response.json()
        if response.status_code != 200:
            raise RuntimeError(f"Error with api request: {data["errmsg"]}")
        return data["cars"]
    
    def metadata(self, car_id):
        response = requests.get(f"{self.base_url}//data/{self.jobname}/cars/{car_id}")
        data = response.json()
        if response.status_code != 200:
            raise RuntimeError(f"Error with api request: {data["errmsg"]}")
        return data

    def position(self, car_id:int):
        """All position points of the car"""
        return self.snapshot(car_id)['x']
    
    def velocity(self, car_id:int):
        """All velocity values of the car"""
        return self.snapshot(car_id)['v']

    def time_in_lane(self, car_id:int):
        "All timestamps where the car was recorded between the start and end of them lane"
        return self.snapshot(car_id)['t']

    def _travel_time(self, snapshot, x0, xf) -> float:
        """Helper function to compute travel time given a json snapshot. 
        Used in travel_time and avg_travel_time to reduce api queries
        """

        x = snapshot['x']
        v = snapshot['v']
        t = snapshot['t']

        if x0 < x[0] or xf > x[-1]:
            return None

        def getTimestamp(x_value):
            cur = bisect.bisect_left(x, x_value)
            prev = cur - 1
            if x[cur] == x_value: # in the list
                t_val = t[cur]
            else:
                t_val = t[prev] + (x_value - x[prev])/v[cur]
            return t_val
        
        t0 = getTimestamp(x0)
        tf = getTimestamp(xf)

        return tf - t0

    def travel_time(self, car_id:int, x0:float, xf:float) -> float:
        data = self.snapshot(car_id)
        return self._travel_time(data, x0, xf)


    def plot_snapshots(self, fname = None):

        """
        Plots position and time for each specified car. Connects to the API to get the data. 
        """
    
        data = self.all_snapshots()
        _, ax = plt.subplots(2,1)
        ax[0].set_title("Position vs time")
        ax[1].set_title("Velocity vs time")
        ax[0].grid(True)
        ax[1].grid(True)
        for car in data["data"]:
            ax[0].plot(car["t"], car["x"])
            ax[1].plot(car["t"], car["v"])
        plt.tight_layout()
        if (fname):
            plt.savefig(f"{fname}.png")
        else:
            plt.show()
        return

    def all_travel_times(self, x0, xf):
        all_data = self.all_snapshots()
        all_times = [self._travel_time(snap, x0, xf) for snap in all_data["data"]]
        return [x for x in all_times if x]

    def travel_time_stats(self, x0, xf):
        all_times = self.all_travel_times(x0, xf)
        return np.average(all_times), np.std(all_times)
    
    def plot_travel_time(self, x0, xf, param):
        """
        Plots the Travel Time as a function of the car follow parameter. 
        """
        all_data = self.all_snapshots()
        all_times = [self._travel_time(snap, x0, xf) for snap in all_data["data"]]
        all_cars = self.all_metadata()
        parms = [metadata["followModel"][param] for metadata in all_cars]

        times, param_vals = [], []
        for t,p in zip (all_times, parms):
            if (t):
                times.append(t)
                param_vals.append(p)

    
        plt.scatter(param_vals, times)
        plt.show()
