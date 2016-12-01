# -*- coding: utf-8 -*-
from flask import Flask
from flask import render_template,Response
from flask import request
from find import prepareData, findPath, SearchResult, getMapBBox 
import json
import re
from gpx import GPX

local_name = "localhost:5000"
deploy_name = "walk.bezva.org"

app = Flask(__name__)

data = prepareData("../config/speeds.yaml","../data/praha-graph.pbf")
bbox = getMapBBox(data)

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
	path = findPath(data,flat,flon,tlat,tlon)
	if path.n_points == 0:
		return ""
	coords = []
	for i in range(path.n_points):
		coords.append((path.points[i].lon,path.points[i].lat))
	print "Points: {}".format(len(coords))
	
	linestring = {"type": "Feature",
			"geometry": {"type": "LineString","coordinates": coords},
			"properties": {"time":path.time,"dist":path.dist}
		}
	return json.dumps(linestring)

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

	hdr = {'Content-Type': 'application/xml+gpx','Content-Disposition': 'attachment; filename=route.gpx'}

	return Response(out_gpx.str,mimetype="application/xml+gpx",headers=hdr)
	



def checkString(s):
	if re.match('^[a-zA-Z_-]+$',s):
		return s
	else:
		return None



