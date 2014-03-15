#!/usr/bin/python

import sys
sys.path.append("../imposm-parser")

import time
import yaml
import pprint
import premap_pb2 as pb
from imposm.parser import OSMParser

scale = 1000000

class Map:
	nodes = []
	ways = []
	relations = []
	nodesidx = {}
	
	def toPB(self):
		pbMap = pb.Map()
		pbMap.nodes.extend(self.nodes)
		pbMap.ways.extend(self.ways)
		pbMap.relations.extend(self.relations)
		return pbMap

class counter():
	ways = 0
	nodes = 0
	relations = 0
	areas=0

	def __init__(self,waysConf):
		self.Map = Map()
		self.waysConf = waysConf

	def ways_cb(self,ways):
		for osmid,tags,refs in ways:
			self.ways+=1
			pbway = pb.Way()
			pbway.id = int(osmid)
			pbway.refs.extend(map(int,refs))
			for key in tags.keys():
				if key[:5]=="addr:" or key[:4]=="name" or key=="source" or key=="is_in" or key[:4]=="ref:" or key=="created_by":
					del tags[key]

				if key in waysConf.keys():
					if waysConf[key].keys()==["*"]:
						pbway.type = waysConf[key]["*"]
					val = tags[key]
					if val in waysConf[key].keys():
						pbway.type = waysConf[key][val]

				if key in areasConf.keys():
					if areasConf[key].keys()==["*"]:
						pbway.area = areasConf[key]["*"]
					val = tags[key]
					if val in areasConf[key].keys():
						pbway.area = areasConf[key][val]
			pbway.str_keys.extend(tags.keys())
			pbway.str_vals.extend(tags.values())
			if pbway.type == pbway.IGNORE:
				pbway.render = False
			else:
				pbway.render = True
			self.Map.ways.append(pbway)

			if "area" in tags.keys() and tags["area"]=="yes":
				self.areas+=1
			if "building" in tags.keys():
				self.areas+=1
			if "landuse" in tags.keys():
				self.areas+=1
			if "leisure" in tags.keys():
				self.areas+=1
			if "natural" in tags.keys():
				self.areas+=1


	def nodes_cb(self,nodes):
		for osmid,tags,coords in nodes:
			self.nodes+=1
			pbnode = pb.Node()
			pbnode.id = int(osmid)
			pbnode.lat = int(coords[1]*scale)
			pbnode.lon = int(coords[0]*scale)
			for key in tags.keys():
				if key[:5]=="addr:" or key=="name" or key=="source" or key=="is_in" or key[:4]=="ref:" or key=="created_by":
					del tags[key]
			pbnode.str_keys.extend(tags.keys())
			pbnode.str_vals.extend(tags.values())
			pbnode.render = True
			self.Map.nodes.append(pbnode)
	
	def coords_cb(self,coords):
		for (osmid,lon,lat) in coords:
			self.nodes+=1
			pbnode = pb.Node()
			pbnode.id = int(osmid)
			pbnode.lat = int(lat*scale)
			pbnode.lon = int(lon*scale)
			pbnode.render = True
			self.Map.nodes.append(pbnode)

	def relations_cb(self,relations):
		for osmid,tags,refs in relations:
			self.relations+=1
			for key in tags.keys():
				if key[:5]=="addr:" or key=="name" or key=="source" or key=="is_in" or key[:4]=="ref:" or key=="created_by":
					del tags[key]
			if "type" in tags.keys() and tags["type"]=="multipolygon":
				self.areas+=1
				pbpol = pb.Multipolygon()
				pbpol.id = int(osmid)
				pbpol.str_keys.extend(tags.keys())
				pbpol.str_vals.extend(tags.values())
				pbpol.refs.extend([int(item[0]) for item in refs])
				pbpol.roles.extend([pbpol.OUTER if item[2]=="outer" else pbpol.INNER for item in refs])


def loadWaysConf(filename):
	config = {}
	with open(filename) as conffile:
		config = yaml.load(conffile.read())
	ways={}
	pbW = pb.Way()
	str2pb = {"ignore": pbW.IGNORE, "barrier":pbW.BARRIER, "railway":pbW.RAILWAY, "water": pbW.WATER, "park": pbW.PARK, "green":pbW.GREEN, "forest":pbW.FOREST }
	for cat in config["Way"].keys():
		catenum = str2pb[cat]
		for key in config["Way"][cat]:
			if key not in ways.keys():
				ways[key] = {}
			for value in config["Way"][cat][key]:
				ways[key][value]=catenum
	return ways


def loadAreasConf(filename):
	config = {}
	with open(filename) as conffile:
		config = yaml.load(conffile.read())
	areas = {}
	for key in config[True]:
		if key not in areas.keys():
			areas[key] = {}
		for value in config[True][key]:
			areas[key][value]=True
	for key in config[False]:
		if key not in areas.keys():
			areas[key] = {}
		for value in config[False][key]:
			areas[key][value]=False
	return areas

def nodeWays(amap):
	missingcnt=0
	nodeways = [ [] for i in amap.nodes]
	for way in amap.ways:
		missing = []
		for node in way.refs:
			try:
				nodeways[amap.nodesidx[node]].append(way.id)
			except KeyError:
				missingcnt+=1
				missing.append(node)
		for node in missing:
			way.refs.remove(node)
	print "Missing "+str(missingcnt)+" nodes"

	return nodeways	

def deleteAloneNodes(amap,nodeways):
	toidx = 0
	for fromidx in range(len(amap.nodes)):
		if len(nodeways[fromidx])==0:
			continue
		amap.nodes[toidx]=amap.nodes[fromidx]
		toidx+=1
	for i in range(len(amap.nodes)-1,toidx,-1):
		del amap.nodes[i]
	for i in range(len(amap.nodes)):
		amap.nodesidx[amap.nodes[i].id]=i
	return amap

def parseOSMfile(filename):	
	cnt = counter(waysConf)
	p = OSMParser(concurrency=4, ways_callback=cnt.ways_cb, nodes_callback=cnt.nodes_cb, relations_callback=cnt.relations_cb, coords_callback=cnt.coords_cb)
	p.parse(filename)
	cnt.Map.nodes.sort(key=lambda node: node.id)
	cnt.Map.ways.sort(key=lambda way: way.id)
	for i in range(len(cnt.Map.nodes)):
		cnt.Map.nodesidx[cnt.Map.nodes[i].id]=i
	return cnt.Map

		


waysConf=loadWaysConf("types.yaml")
areasConf=loadAreasConf("area.yaml")

start = time.time()

amap = parseOSMfile("../../osm/praha.osm")

end = time.time()
print "Parsing took "+str(end-start)
start = time.time()

nodeways=nodeWays(amap)
amap = deleteAloneNodes(amap,nodeways)

end = time.time()
print "Nodeways took "+str(end-start)
start = time.time()

outfile = open("praha-pre.pbf","w")
outfile.write(amap.toPB().SerializeToString())
outfile.close()

end = time.time()
print "Saving took "+str(end-start)

print "Ways:",len(amap.ways)
print "Nodes:",len(amap.nodes)
print "Nodes on ways",len(nodeways)

