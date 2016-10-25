#!/usr/bin/env python2

import fetchTransfer as ft
import sys
import json
import re
from lxml import etree
import gpx

with open(sys.argv[1],"r") as infile:
    injson = json.load(infile)
outfilename = sys.argv[2]
fetcher = ft.FetchTransfer(outfilename)
splitter = re.compile("\((?P<flat>[0-9.]*), (?P<flon>[0-9.]*)\) \((?P<tlat>[0-9.]*), (?P<tlon>[0-9.]*)\)")
outgpx=gpx.GPX("output-seznam.gpx")

for key in injson:
    print key
    coordsre = splitter.match(key)
    flat = coordsre.group("flat")
    flon = coordsre.group("flon")
    tlat = coordsre.group("tlat")
    tlon = coordsre.group("tlon")
    datastring = fetcher.fetchTransfer((flat,flon),(tlat,tlon))
    if datastring == None:
        continue
    route = etree.fromstring(datastring)
    pointstree = route.find("points")
    points = [(flat,flon,0)]+[(child.get("lat"),child.get("lon"),child.get("altitude")) for child in pointstree.iterchildren()]+[(tlat,tlon,0)]
    outgpx.writeTrack(points)


fetcher.dumpData()
outgpx.close()
