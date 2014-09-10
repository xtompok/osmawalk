#!/usr/bin/env python2

import subprocess as sp
import json 
import sys
import re
import gpx

jsonFile = open(sys.argv[1],"r")
transfers = json.load(jsonFile)
jsonFile.close()

splitter = re.compile("\((?P<flat>[0-9.]*), (?P<flon>[0-9.]*)\) \((?P<tlat>[0-9.]*), (?P<tlon>[0-9.]*)\)")
outgpx = gpx.GPX("output.gpx")

search=sp.Popen("./search-transfer",stdin=sp.PIPE,stdout=sp.PIPE)
print "PID: ",search.pid
i = 0

for k,v in transfers.iteritems():
    print i
    i += 1
    coords = splitter.match(k)
    flat = coords.group("flat")
    flon = coords.group("flon")
    tlat = coords.group("tlat")
    tlon = coords.group("tlon")
    search.stdin.write(flat+" "+flon+" "+tlat+" "+tlon+"\n")
    search.stdin.flush()
    line = search.stdout.readline()
    print line
    (dist,time)=map(float,line.split(" "))
    transfers[k]["owlength"]=dist
    transfers[k]["owtime"]=time
    line = search.stdout.readline()
    while line != "\n": 
        outgpx.fd.write(line)
        line = search.stdout.readline()
    if dist==0:
        outgpx.addWpt("FAIL_"+str(i)+"_Z",flat,flon,0)
        outgpx.addWpt("FAIL_"+str(i)+"_K",tlat,tlon,0)
    else:
        outgpx.addWpt("OK_"+str(i)+"_Z",flat,flon,0)
        outgpx.addWpt("OK_"+str(i)+"_K",tlat,tlon,0)

search.kill()
outgpx.close()
outjson = open("output.json","w")
json.dump(transfers,outjson,sort_keys=True,indent=4)
outjson.close()
