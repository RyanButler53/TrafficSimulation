# Visualizes v vs t 
config = "../build/dbConfig.yaml"

import os

BASE_URL ="http://localhost:8000"
import requests

def submit(jobname, config):
    file = os.path.abspath(config)
    if not os.path.exists(file):
        raise FileNotFoundError(f"File {config} not found!")
    response = requests.post(f"{BASE_URL}/submit/{jobname}", params = {"config": file})
    if (response.status_code != 200):
        return response.json()["errmsg"]
    else:
        return response.json()
    
def delete(jobname):
    response = requests.delete(f"{BASE_URL}/jobs/{jobname}")
    if (response.status_code != 200):
        return response.json()["errmsg"]
    else:
        return response.json()
