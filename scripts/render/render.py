#!/usr/bin/python

import sys
sys.path.append("../svgwrite-1.1.3")
sys.path.append("../filter")

import premap_pb2 as pb
import svgwrite

scale = 1000000
shiftlat = scale*49.941901
shiftlon = scale*14.224436

def drawWays(pbMap,d, node_ids):
	for way in pbMap.ways:
		coord=[]
		for ref in way.refs:
			try:
				node = (pbMap.nodes[node_ids[ref]])
				coord.append((node.lon-shiftlon,-(node.lat-shiftlat)))
			except KeyError:
				pass
		style={"stroke" : "black", "stroke-width" : "5", "fill" : "none"}
		if way.type==way.WAY:
			pass
		elif way.type==way.RAILWAY:
			style["stroke"] = "purple"
		elif way.type==way.WATER:
			style["stroke"] = "blue"
		elif way.type==way.BARRIER:
			style["stroke"] = "red"
		elif way.type==way.PARK:
			style["stroke"] = "green"
		elif way.type==way.GREEN:
			style["stroke"] = "greenyellow"
		elif way.type==way.FOREST:
			style["stroke"] = "forestgreen"
		elif way.type==way.PAVED:
			style["stroke"] = "gray"
		elif way.type==way.IGNORE:
			style["stroke"] = "cyan"
		elif way.type==way.UNPAVED:
			style["stroke"] = "brown"
		elif way.type==way.STEPS:
			style["stroke"] = "steelblue"
		elif way.type==way.HIGHWAY:
			style["stroke"] = "darkslategray"

		elif way.type==way.MULTIPOLYGON:
			style["stroke"] = "orange"
			print "Multipoly found"

		
		if way.area:
			style["fill"]=style["stroke"]
			style["fill-opacity"] = "0.5"
		if not way.render and way.type==way.BARRIER:
			style["fill"]="aqua"

		stylestr = ";".join([key+":"+value for key,value in style.iteritems()])
		d.add(svgwrite.shapes.Polyline(coord,style=stylestr))

print "Loading map..."
pbMap = pb.Map()
with open("../filter/praha-union.pbf","rb") as infile:
	pbMap.ParseFromString(infile.read())

node_ids = {}
for i in range(len(pbMap.nodes)):
	node_ids[pbMap.nodes[i].id]=i

print "Generating SVG..."

d = svgwrite.Drawing()
drawWays(pbMap,d,node_ids)

print "Saving SVG..."
d.save()
print "Done"
