import pandas as pd
import matplotlib.pyplot as plt
from abc import ABC, abstractmethod
import requests
import os
import pandas as pd
import subprocess
import shutil
import reader

class Location:
    def __init__(self, x:float, v:float, l:int):
        self.x = x
        self.v = v
        self.l = l

class MovieMaker(ABC):

    def __init__(self, x0, x1, t0, t1, nlanes):
        self.xlimits = (x0, x1)
        self.tlimits = (t0, t1)
        self.lanes = nlanes
        self.temp_path = ".movie_tmp"
        if os.path.exists(self.temp_path):
            shutil.rmtree(self.temp_path)
        os.makedirs(self.temp_path)

    def __repr__(self):
        pass

    def run(self, outputFilename:str):
        t = 0
        while t < self.tlimits[1]:
            t = self.generateNextFrame()
        framerate = round(1/self.dt())

        subprocess.run(f"ffmpeg -framerate {framerate} -i {self.temp_path}/frame%d.jpg -c:v libx264 -pix_fmt yuv420p {outputFilename}.mp4", shell=True)
        shutil.rmtree(self.temp_path)

    @abstractmethod
    def dt()->float:
        """Returns the time between each frame. """

    @abstractmethod
    def generateNextFrame() -> float:
        """Returns the timestamp t
        and a list of the cars (x, v, l) points"""

class TimeSeries(MovieMaker):

    def __init__(self, filepath:os.path,  x0, x1, t0, t1, nlanes):
        super().__init__(x0, x1, t0, t1, nlanes)
        self.filepath = filepath

        self.files = sorted(os.listdir(filepath))
        self.files = self.files[2:]
        self.files.sort(key=lambda s: float(s[5:-4]))
        self.files = list(filter(lambda f:  (float(f[5:-4]) <= self.tlimits[1] and float(f[5:-4]) >= self.tlimits[0]), self.files))
        self.index = 0


    def __repr__(self):
        return f"File Reader for folder {self.filepath}. X limits: {self.xlimits}, T limits: {self.tlimits}"

    def dt(self):
        t0 = float(self.files[0][5:-4])
        t1 = float(self.files[1][5:-4])
        return t1 - t0

    def generateNextFrame(self):
        file = self.files[self.index]
        t = float(file[5:-4])
        df = pd.read_csv(os.path.join(self.filepath, file), index_col=False)
        validData = df[(df["x"] >= self.xlimits[0]) & (df["x"] <= self.xlimits[1])
                    & (t >= self.tlimits[0]) & (t <= self.tlimits[1])]
        plt.xlim(self.xlimits)
        plt.ylim(-0.2, self.lanes+0.2)
        plt.yticks(list(range(self.lanes+1)))
        plt.scatter(validData["x"], validData["l"], c=validData["v"])
        plt.savefig(f"{self.temp_path}/frame{self.index}.jpg")
        plt.clf()
        self.index += 1
        return t

class Database(MovieMaker):
    def __init__(self, jobname,  x0, x1, t0, t1, nlanes):
        super().__init__(x0, x1, t0, t1, nlanes)

        self.reader = reader.Reader(jobname)
        data = self.reader.spatialQuery(x0, x1, t0, t1)
        if (data.status_code == 200):
            jsondata = data.json()
            self.times = jsondata["timestamps"]
            self.snapshots = jsondata["snapshots"]
        else:
            raise RuntimeError(data.json()["errmsg"])
        
        self.index = 0

    def dt(self):
        return self.times[1] - self.times[0]

    def generateNextFrame(self):
        t = self.times[self.index]

        data = self.snapshots[self.index]

        # df = pd.read_csv(os.path.join(self.filepath, file), index_col=False)
        xs = [snapshot["x"] for snapshot in data]
        vs = [snapshot["v"] for snapshot in data]
        lanes = [snapshot["l"] for snapshot in data]

        plt.xlim(self.xlimits)
        plt.ylim(-0.2, self.lanes+0.2)
        plt.yticks(list(range(self.lanes+1)))
        plt.scatter(xs, lanes, c=vs)
        plt.savefig(f"{self.temp_path}/frame{self.index}.jpg")
        plt.clf()
        self.index += 1
        return t
