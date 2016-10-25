# -*- coding: utf-8 -*-
from flask import Flask
from flask import render_template
from flask import request
import json
import datetime
import time
import re
import urllib

local_name = "localhost:5000"
deploy_name = "walk.bezva.org"

app = Flask(__name__)

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
def search():
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
	return ""



def checkString(s):
	if re.match('^[a-zA-Z_-]+$',s):
		return s
	else:
		return None

