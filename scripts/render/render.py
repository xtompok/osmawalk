#!/usr/bin/env python2

import sys
sys.path.append("../svgwrite-1.1.3")
sys.path.append("../filter")

import premap_pb2 as pb
import types_pb2 as pbtypes
import svgwrite

scale = 1000000
shiftlat = scale*49.941901
shiftlon = scale*14.224436

def drawWays(pbMap,d, node_ids,minlon,minlat):
	for way in pbMap.ways:
		coord=[]
		for ref in way.refs:
			try:
				node = (pbMap.nodes[node_ids[ref]])
				coord.append((node.lon-minlon,-(node.lat-minlat)))
			except KeyError:
				pass
		style={"stroke" : "black", "stroke-width" : "5", "fill" : "none"}
		if way.type==pbtypes.WAY:
			pass
		elif way.type==pbtypes.RAILWAY:
			style["stroke"] = "purple"
		elif way.type==pbtypes.WATER:
			style["stroke"] = "blue"
		elif way.type==pbtypes.BARRIER:
			style["stroke"] = "red"
		elif way.type==pbtypes.PARK:
			style["stroke"] = "green"
		elif way.type==pbtypes.GREEN:
			style["stroke"] = "greenyellow"
		elif way.type==pbtypes.FOREST:
			style["stroke"] = "forestgreen"
		elif way.type==pbtypes.PAVED:
			style["stroke"] = "gray"
		elif way.type==pbtypes.IGNORE:
			style["stroke"] = "cyan"
		elif way.type==pbtypes.UNPAVED:
			style["stroke"] = "brown"
		elif way.type==pbtypes.STEPS:
			style["stroke"] = "steelblue"
		elif way.type==pbtypes.HIGHWAY:
			style["stroke"] = "darkslategray"
		elif way.type==pbtypes.DIRECT:
			style["stroke"] = "magenta"
		elif way.type==pbtypes.DIRECT_BAD:
			style["stroke"] = "orange"
                        #print "Found broken line"

		elif way.type==pbtypes.MULTIPOLYGON:
			style["stroke"] = "orange"
			print "Multipoly found"

                elif way.type==pbtypes.WALKAREA:
                        style["stroke"] = "blue"
                elif way.type==pbtypes.AREAWAY:
                        style["stroke"] = "khaki"
                elif way.type==pbtypes.AREABAR:
                        style["stroke"] = "orange"


		
		if way.area:
			style["fill"]=style["stroke"]
			style["fill-opacity"] = "0.5"
		#if not way.render and way.type==pbtypes.BARRIER:
		#	style["fill"]="aqua"

		stylestr = ";".join([key+":"+value for key,value in style.iteritems()])
		d.add(svgwrite.shapes.Polyline(coord,style=stylestr))
	for node in pbMap.nodes:
	#	style={"stroke" : "blue", "stroke-width" : "5", "fill" : "black"}
	#	stylestr = ";".join([key+":"+value for key,value in style.iteritems()])
	#	d.add(svgwrite.shapes.Circle((node.lon-minlon,-(node.lat-minlat)),10))

		if node.onBridge:
			style={"stroke" : "blue", "stroke-width" : "5", "fill" : "black"}
			stylestr = ";".join([key+":"+value for key,value in style.iteritems()])
			d.add(svgwrite.shapes.Circle((node.lon-minlon,-(node.lat-minlat)),10,style=stylestr))
		if node.inTunnel:
			style={"stroke" : "black", "stroke-width" : "5", "fill" : "black"}
			stylestr = ";".join([key+":"+value for key,value in style.iteritems()])
			d.add(svgwrite.shapes.Circle((node.lon-minlon,-(node.lat-minlat)),10,style=stylestr))

datadir="../../data/"

print "Loading map..."
pbMap = pb.Map()
with open(datadir+"/praha-union-c.pbf","rb") as infile:
	pbMap.ParseFromString(infile.read())

node_ids = {}
minlon = 99999999999999999999
minlat = 99999999999999999999
for i in range(len(pbMap.nodes)):
	node_ids[pbMap.nodes[i].id]=i
        minlon = min(minlon,pbMap.nodes[i].lon)
        minlat = min(minlat,pbMap.nodes[i].lat)

print "Generating SVG..."

d = svgwrite.Drawing()
drawWays(pbMap,d,node_ids,minlon,minlat)

print "Saving SVG..."
d.save()
print "Done"
