# -*- coding: utf-8 -*-
from flask import Flask
from flask import render_template
from flask import request
from find import prepareData, findPath, SearchResult 
import json
import re

local_name = "localhost:5000"
deploy_name = "walk.bezva.org"

app = Flask(__name__)

data = prepareData("../config/speeds.yaml","../data/praha-graph.pbf")

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
	return render_template('index.html',name=local_name)

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
			"properties": {}
		}
	return json.dumps(linestring)



def checkString(s):
	if re.match('^[a-zA-Z_-]+$',s):
		return s
	else:
		return None



