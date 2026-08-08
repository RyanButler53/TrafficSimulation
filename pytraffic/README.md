Python scripts to more easily interact with the api and visualize data. 

Reader Class offers ability to read snapshot data, read car metadata, calculate average and stdev of travel times and travel times in general. 
Also can plot position and velocity vs time for all cars. 

MovieMaker classes allow the ability to make a ffmpeg based movie from a jobname or filepath with in x and t bounds. 

Usage: `python pytraffic/movie.py -t t0,tf -x x0,xf -s filepath|jobName -o outputName`