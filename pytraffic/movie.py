import pandas as pd
import matplotlib.pyplot as plt
from abc import ABC, abstractmethod
import requests
import os
import pandas as pd
import subprocess
import shutil

class Location:
    def __init__(self, x:float, v:float, l:int):
        self.x = x
        self.v = v
        self.l = l

class DataReader(ABC):

    def __init__(self, x0, x1, t0, t1, nlanes):
        self.xlimits = (x0, x1)
        self.tlimits = (t0, t1)
        self.lanes = nlanes

    def __repr__(self):
        pass

    def run(self, outputFilename:str):
        t = 0
        while t < self.tlimits[1]:
            t = self.generateNextFrame()
        framerate = round(1/self.dt())

        subprocess.run(f"ffmpeg -framerate {framerate} -i movie_tmp/frame%d.jpg -c:v libx264 -pix_fmt yuv420p {outputFilename}.mp4", shell=True)
        shutil.rmtree("movie_tmp")

    @abstractmethod
    def dt()->float:
        """Returns the time between each frame. """

    @abstractmethod
    def generateNextFrame() -> float:
        """Returns the timestamp t
        and a list of the cars (x, v, l) points"""

class FileReader(DataReader):

    def __init__(self, filepath:os.path,  x0, x1, t0, t1, nlanes):
        super().__init__(x0, x1, t0, t1, nlanes)
        self.filepath = filepath

        self.files = sorted(os.listdir(filepath))
        self.files = self.files[2:]
        self.files.sort(key=lambda s: float(s[5:-4]))
        self.files = list(filter(lambda f:  (float(f[5:-4]) <= self.tlimits[1] and float(f[5:-4]) >= self.tlimits[0]), self.files))
        print(self.files)
        self.index = 0

        if os.path.exists("movie_tmp"):
            shutil.rmtree("movie_tmp")
        os.makedirs("movie_tmp")

    def __repr__(self):
        return f"File Reader for folder {self.filepath}. X limits: {self.xlimits}, T limits: {self.tlimits}"

    def dt(self):
        t0 = float(self.files[0][5:-4])
        t1 = float(self.files[1][5:-4])
        return t1 - t0

    def generateNextFrame(self):
        file = self.files[self.index]
        t = float(file[5:-4])
        print(f"File: {file}")
        df = pd.read_csv(os.path.join(self.filepath, file), index_col=False)
        validData = df[(df["x"] >= self.xlimits[0]) & (df["x"] <= self.xlimits[1])
                    & (t >= self.tlimits[0]) & (t <= self.tlimits[1])]
        plt.xlim(self.xlimits)
        plt.ylim(-0.2, self.lanes+0.2)
        plt.yticks(list(range(self.lanes+1)))
        plt.scatter(validData["x"], validData["l"], c=validData["v"])
        plt.savefig(f"movie_tmp/frame{self.index}.jpg")
        plt.clf()
        self.index += 1
        return t
