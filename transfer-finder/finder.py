#!/usr/bin/env python2

import subprocess as sp
import json 
import sys
import re
import gpx

jsonFile = open(sys.argv[1],"r")
transfers = json.load(jsonFile)
jsonFile.close()

splitter = re.compile("(?P<lat>[0-9.]*) (?P<lon>[0-9.]*)")

search=sp.Popen("./search-transfer",stdin=sp.PIPE,stdout=sp.PIPE)
print "PID: ",search.pid
i = 0
try:
    for transfer in transfers["transfers"]:
        print i
        i += 1
        flat = transfer["location1"]["location"]["latitude"]
        flon = transfer["location1"]["location"]["longitude"]
        tlat = transfer["location2"]["location"]["latitude"]
        tlon = transfer["location2"]["location"]["longitude"]
        search.stdin.write(str(flat)+" "+str(flon)+" "+str(tlat)+" "+str(tlon)+"\n")
        search.stdin.flush()
        line = search.stdout.readline()
        print "Search line: {}".format(line)
        (dist,time)=map(float,line.split(" "))
        transfer["transfer"]=dict()
        transfer["transfer"]["distance"]=dist
        transfer["transfer"]["time"]=time
        coords = []
        line = search.stdout.readline()
        while line != "\n": 
            matched = splitter.match(line)
            lat = float(matched.group("lat"))
            lon = float(matched.group("lon"))
            coords.append([lat,lon])
            line = search.stdout.readline()
        transfer["transfer"]["trace"]=coords
finally:
    search.kill()
    
outjson = open("output.json","w")
json.dump(transfers,outjson,sort_keys=True,indent=4)
outjson.close()
