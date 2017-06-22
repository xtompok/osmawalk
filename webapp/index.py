# -*- coding: utf-8 -*-
from flask import Flask
from flask import render_template,Response
from flask import request
from find import prepareData, findPath, SearchResult, getMapBBox, freePackedPBF 
import json
import re
import result_pb2
import types_pb2 as objtypes
from gpx import GPX
import timetable

local_name = "localhost:5000"
deploy_name = "walk.bezva.org"

app = Flask(__name__)

data = prepareData("../config/speeds.yaml","../data/praha-graph.pbf","../ext-lib/mmpf/raptor/tt.bin")
bbox = getMapBBox(data)
tt = timetable.Timetable("../ext-lib/mmpf/raptor/tt.bin")

def get_attribute(request,name,convert=None,default=None):
	attr = request.args.get(name,None)
	if not attr:
		return (default,"Attribute '{}' undefined".format(name))
	if convert:
		try:
			attr = convert(attr)
		except:
			return (default,"Attribute '{}' in wrong format".format(name))
	return (attr,None)
	
def same_line(pt1,pt2):
	""" Returns false only if both edges are public transport and the line differs"""
	if pt1.edgetype != result_pb2.PT:
		return True
	if pt2.edgetype != result_pb2.PT:
		return True
	return pt1.ptedge.name == pt2.ptedge.name #TODO use ids

def split_route(route):
	subroutes = []
	mempt = route.points[0]
	subroute = [mempt]
	for pt in route.points:
		print pt.edgetype
		if (pt.edgetype != mempt.edgetype or not same_line(pt,mempt)):
			subroutes.append(subroute)
			subroute = [mempt]
		mempt = pt
		subroute.append(pt)
	subroutes.append(subroute)
	return subroutes

def routeseg2geojson(rtseg):
	print "Route segment len:{}".format(len(rtseg))
	if (rtseg[1].edgetype == result_pb2.WALK):
		etype = rtseg[1].walkedge.type
	elif (rtseg[1].edgetype == result_pb2.PT):
		etype = objtypes.PUBLIC_TRANSPORT
	else:
		etype = objtypes.NONE
	properties = {"type":etype}
	if (rtseg[1].edgetype == result_pb2.PT):
		print "Route name: ",rtseg[1].ptedge.name
		route = rtseg[1].ptedge
		properties["name"] = route.name
	coords = list(map(lambda pt: [pt.lon,pt.lat,pt.height],rtseg))
	linestring = {"type": "Feature",
		"geometry": {"type": "LineString", "coordinates": coords},
		"properties": properties
	}
	return linestring

def point2geojson(coord,properties):
	point = {"type": "Feature",
		"geometry": {"type": "Point", "coordinates": coord},
		"properties":properties
	}
	return point

def stopName(routeidx,stopidx):
	return tt.stops[stopidx].name

def points2geojson(route):
	points = []
	for (idx,pt) in enumerate(route.points):
		if pt.edgetype == result_pb2.PT:
			prevpt = route.points[idx-1]
			geojson = point2geojson([prevpt.lon,prevpt.lat],
				{"type":objtypes.PUBLIC_TRANSPORT,
				"name":prevpt.stop.name,
				"subtype":"departure",
				"departure":prevpt.departure,
				"arrival":prevpt.arrival})
			points.append(geojson)
			geojson = point2geojson([pt.lon,pt.lat],
				{"type":objtypes.PUBLIC_TRANSPORT,
				"name":pt.stop.name,
				"subtype":"arrival",
				"departure":pt.departure,
				"arrival":pt.arrival})
			points.append(geojson)
	return points
		
	
def route2geojson(route):
	subroutes = split_route(route)
	features = list(map(routeseg2geojson,split_route(route)))
	features += points2geojson(route)
	collection = {"type": "FeatureCollection",
		"features": features,
		"properties": {"time": route.time,
			"dist":route.dist,
			"penalty":route.penalty}
		}
	return collection



@app.route('/')
def leaflet_page(name=None):
	return render_template('index.html',name=local_name,bbox=bbox)

@app.route('/search')
def do_search():
	(flon,error) = get_attribute(request,'flon',float)
	if error:
		print "Error: {}".format(error)
	(flat,error) = get_attribute(request,'flat',float)
	if error:
		print "Error: {}".format(error)
	(tlon,error) = get_attribute(request,'tlon',float)
	if error:
		print "Error: {}".format(error)
	(tlat,error) = get_attribute(request,'tlat',float)
	if error:
		print "Error: {}".format(error)

	print "From lon: {}, from lat: {}, to lon: {}, to lat: {}".format(flon,flat,tlon,tlat)
	print "./search {} {} {} {}".format(flat,flon,tlat,tlon) 
	path = findPath(data,flat,flon,tlat,tlon)
	print "Result size {}B".format(path.len)
	pbf = "".join(map(chr,path.data[:path.len]))
	freePackedPBF(path)
	result = result_pb2.Result()
	result.ParseFromString(pbf)
	print "Routes: {}".format(len(result.routes))
	routes = []
	for r in result.routes:
		print "Route len: {}, time: {}".format(r.time,r.dist)
		routes.append({"time":r.time,"dist":r.dist,"penalty":r.penalty,"geojson":route2geojson(r)})
	return json.dumps(routes)


@app.route('/gpx')
def make_gpx():
	(flon,error) = get_attribute(request,'flon',float)
	if error:
		print "Error: {}".format(error)
	(flat,error) = get_attribute(request,'flat',float)
	if error:
		print "Error: {}".format(error)
	(tlon,error) = get_attribute(request,'tlon',float)
	if error:
		print "Error: {}".format(error)
	(tlat,error) = get_attribute(request,'tlat',float)
	if error:
		print "Error: {}".format(error)

	print "From lon: {}, from lat: {}, to lon: {}, to lat: {}".format(flon,flat,tlon,tlat)
	path = findPath(data,flat,flon,tlat,tlon)
	if path.n_points == 0:
		return ""
	coords = []
	out_gpx = GPX()
	out_gpx.startTrack()
	for i in range(path.n_points):
		pt = path.points[i]
		out_gpx.writeTrkpt(pt.lat,pt.lon,pt.height)
	out_gpx.endTrack()
	out_gpx.close()
	freePackedPBF(path)

	hdr = {'Content-Type': 'application/xml+gpx','Content-Disposition': 'attachment; filename=route.gpx'}

	return Response(out_gpx.str,mimetype="application/xml+gpx",headers=hdr)
	



def checkString(s):
	if re.match('^[a-zA-Z_-]+$',s):
		return s
	else:
		return None



