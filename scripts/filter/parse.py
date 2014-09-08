#!/usr/bin/env python2

import sys
sys.path.append("../imposm-parser")

import time
import sys
import yaml
import pyproj
import premap_pb2 as pb
import types_pb2 as pbtypes
from utils import nodeWays, deleteAloneNodes 
from Map import Map
from imposm.parser import OSMParser

scale = 10

class classifier():
	""" Class for parsing OSM XML and classifying elements"""
	def __init__(self,waysConf,areasConf,bridgesConf,tunnelsConf,srtm):
		self.proj = pyproj.Proj(proj='utm', zone=33, ellps='WGS84')
		self.Map = Map()
		self.waysConf = waysConf
		self.areasConf = areasConf
		self.bridgesConf = bridgesConf
		self.tunnelsConf = tunnelsConf
		self.srtm = srtm

	def calcHeight(self,lon, lat):
		""" Calculate height of given point"""
		lon -= self.srtm.minlon
		lat -= self.srtm.minlat
		lon *= 1200
		lat *= 1200
		intlon = int(lon)
		intlat = int(lat)
		
		ll = self.srtm.raster[intlat][intlon]
		lr = self.srtm.raster[intlat][intlon+1]
		ul = self.srtm.raster[intlat+1][intlon]
		ur = self.srtm.raster[intlat+1][intlon+1]

		left = (lat-intlat)*ll+(1-lat+intlat)*ul
		right = (lat-intlat)*lr+(1-lat+intlat)*ur
		height = (lon-intlon)*left+(1-lon+intlon)*right
		return int(height)

	def ways_cb(self,ways):
		""" Way callback"""
		for osmid,tags,refs in ways:
			pbway = pb.Way()
			pbway.id = int(osmid)
			pbway.refs.extend(map(int,refs))
                        priority = -1
			if "boundary" in tags:
				continue
			for key in tags.keys():
				if key in waysConf:
					if waysConf[key].keys()==["*"] and pbway.type == pbtypes.WAY:
                                                (atype,apriority)=waysConf[key]["*"]
                                                if priority<apriority:
                                                    pbway.type = atype
                                                    priority = apriority
					val = tags[key]
					if val in waysConf[key]:
                                                (atype,apriority)=waysConf[key][val]
                                                if priority<apriority:
                                                    pbway.type = atype
                                                    priority = apriority

				if key in areasConf:
					if areasConf[key].keys()==["*"]:
						pbway.area = areasConf[key]["*"]
					val = tags[key]
					if val in areasConf[key]:
						pbway.area = areasConf[key][val]

				if key in bridgesConf:
					if bridgesConf[key].keys()==["*"]:
						pbway.bridge = bridgesConf[key]["*"]
					val = tags[key]
					if val in bridgesConf[key]:
						pbway.bridge = bridgesConf[key][val]

				if key in tunnelsConf:
					if tunnelsConf[key].keys()==["*"]:
						pbway.tunnel = tunnelsConf[key]["*"]
					val = tags[key]
					if val in tunnelsConf[key]:
						pbway.tunnel = tunnelsConf[key][val]

			if pbway.type == pbtypes.IGNORE:
				pbway.render = False
			else:
				pbway.render = True
                        if pbway.type == pbtypes.IGNORE or pbway.type == pbtypes.WAY:
                            continue
			self.Map.ways.append(pbway)


	def nodes_cb(self,nodes):
		""" Nodes callback"""
		for osmid,tags,coords in nodes:
			pbnode = pb.Node()
			pbnode.id = int(osmid)
			pbnode.height = self.calcHeight(coords[0],coords[1])
			(lon,lat) = self.proj(coords[0],coords[1])
			pbnode.lat = int(lat*scale)
			pbnode.lon = int(lon*scale)
			self.Map.nodes.append(pbnode)
	
	def coords_cb(self,coords):
		""" Callback for nodes with no attributes"""
		for (osmid,lon,lat) in coords:
			pbnode = pb.Node()
			pbnode.id = int(osmid)
			pbnode.height = self.calcHeight(lon,lat)
			(lon,lat) = self.proj(lon,lat)
			pbnode.lat = int(lat*scale)
			pbnode.lon = int(lon*scale)
			self.Map.nodes.append(pbnode)

	def relations_cb(self,relations):
		""" Relations callback"""
		for osmid,tags,refs in relations:
			if not ("type" in tags and tags["type"]=="multipolygon"):

				pbpol = pb.Multipolygon()
                                priority=-1
				for key in tags.keys():
					if key in waysConf:
						if waysConf[key].keys()==["*"]:
                                                        (atype,apriority)=waysConf[key]["*"]
							if priority<apriority:
                                                            pbpol.type = atype
                                                            priority = apriority
						val = tags[key]
						if val in waysConf[key]:
                                                        (atype,apriority)=waysConf[key][val]
							if priority<apriority:
                                                            pbpol.type = atype
                                                            priority = apriority
				pbpol.id = int(osmid)
				pbpol.refs.extend([int(item[0]) for item in refs])
				pbpol.roles.extend([pbpol.OUTER if item[2]=="outer" else pbpol.INNER for item in refs])
				self.Map.multipols.append(pbpol)
class SRTM:
	""" Class for SRTM data """
	minlon = 0
	minlat = 0
	maxlon = 0
	maxlat = 0
	raster = [[]] 

def loadSRTM(filename):
	""" Load SRTM from file"""
	with open(filename) as srtmfile:
	    srtm = SRTM()
	    (srtm.minlon, srtm.minlat, srtm.maxlon, srtm.maxlat) = [ int(x) for x in srtmfile.readline().split(" ")]
	    srtm.raster=[[int(y) for y in x.split(" ")[:-2]] for x in srtmfile]
	    return srtm


	

def loadWaysConf(filename):
	""" Load configuration for classifying ways from file"""
	config = {}
	with open(filename) as conffile:
		config = yaml.load(conffile.read())
	desc = pbtypes._OBJTYPE # depends on implementation detail, but no other way
	str2pb = {}
	for i in desc.values:
		str2pb[i.name] = i.number
	ways={}
	for cat in config.keys():
		catenum = str2pb[cat]
                for priority in config[cat]:
                    for key in config[cat][priority]:
                            if key not in ways:
                                    ways[key] = {}
                            for value in config[cat][priority][key]:
                                    ways[key][value]=(catenum,priority)
	return ways


def loadBoolConf(filename):
	""" Load generic configuration from file, which returns for OSM attributes only True of False"""
	config = {}
	with open(filename) as conffile:
		config = yaml.load(conffile.read())
	boolcfg = {}
	for key in config[True]:
		if key not in boolcfg:
			boolcfg[key] = {}
		for value in config[True][key]:
			boolcfg[key][value]=True
	for key in config[False]:
		if key not in boolcfg:
			boolcfg[key] = {}
		for value in config[False][key]:
			boolcfg[key][value]=False
	return boolcfg

def markNonPlanarNodes(amap):
	""" Mark nodes, which are above or below ground """
	for way in amap.ways:
		if way.bridge:
			for nid in way.refs:
				amap.nodes[amap.nodesIdx[nid]].onBridge=True
		if way.tunnel:
			for nid in way.refs:
				amap.nodes[amap.nodesIdx[nid]].inTunnel=True


def parseOSMfile(filename,waysConf,areasConf,bridgesConf,tunnelsConf,srtm):	
	""" Parse OSM file """
	clas = classifier(waysConf,areasConf,bridgesConf,tunnelsConf,srtm)
	p = OSMParser(concurrency=4, ways_callback=clas.ways_cb, nodes_callback=clas.nodes_cb, relations_callback=clas.relations_cb, coords_callback=clas.coords_cb)
	p.parse(filename)
	clas.Map.nodes.sort(key=lambda node: node.id)
	clas.Map.ways.sort(key=lambda way: way.id)
	for i in range(len(clas.Map.nodes)):
		clas.Map.nodesIdx[clas.Map.nodes[i].id]=i
	return clas.Map

		
# Where find YAML config files
configDir="../../config/"
# Where save PBF file
dataDir="../../data/"

# Load config from YAML
waysConf=loadWaysConf(configDir+"types.yaml")
areasConf=loadBoolConf(configDir+"area.yaml")
bridgesConf=loadBoolConf(configDir+"bridge.yaml")
tunnelsConf=loadBoolConf(configDir+"tunnel.yaml")

# Load SRTM file
srtm = loadSRTM("../../osm/heights.txt")

# Parse file
start = time.time()
amap = parseOSMfile("../../osm/praha.osm",waysConf,areasConf,bridgesConf,tunnelsConf,srtm)
end = time.time()
print "Parsing took "+str(end-start)

# Delete nodes without ways
start = time.time()
nodeways=nodeWays(amap)
amap = deleteAloneNodes(amap,nodeways)
end = time.time()
print "Deleting alone nodes took "+str(end-start)

# Mark nodes in tunnels and on bridges
amap.updateNodesIdx()
markNonPlanarNodes(amap)

# Write map to file
start = time.time()
outfile = open(dataDir+"praha-pre.pbf","w")
outfile.write(amap.toPB().SerializeToString())
outfile.close()
end = time.time()
print "Saving took "+str(end-start)

print "Ways:",len(amap.ways)
print "Nodes:",len(amap.nodes)
print "Multipolygons:",len(amap.multipols)

