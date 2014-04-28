#!/usr/bin/env python2

import sys
sys.path.append("../imposm-parser")

import time
import yaml
import pprint
import premap_pb2 as pb
import types_pb2 as pbtypes
from Map import Map
from imposm.parser import OSMParser

scale = 1000000

class counter():
	def __init__(self,waysConf,areasConf,bridgesConf,tunnelsConf):
		self.Map = Map()
		self.waysConf = waysConf
		self.areasConf = areasConf
		self.bridgesConf = bridgesConf
		self.tunnelsConf = tunnelsConf

	def ways_cb(self,ways):
		for osmid,tags,refs in ways:
			pbway = pb.Way()
			pbway.id = int(osmid)
			pbway.refs.extend(map(int,refs))
			for key in tags.keys():
				if key=="boundary":
					break

				if key in waysConf:
					if waysConf[key].keys()==["*"]:
						pbway.type = waysConf[key]["*"]
					val = tags[key]
					if val in waysConf[key]:
						pbway.type = waysConf[key][val]

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

			if "boundary" in tags:
				continue
			if pbway.type == pbtypes.IGNORE:
				pbway.render = False
			else:
				pbway.render = True
			self.Map.ways.append(pbway)


	def nodes_cb(self,nodes):
		for osmid,tags,coords in nodes:
			pbnode = pb.Node()
			pbnode.id = int(osmid)
			pbnode.lat = int(coords[1]*scale)
			pbnode.lon = int(coords[0]*scale)
			self.Map.nodes.append(pbnode)
	
	def coords_cb(self,coords):
		for (osmid,lon,lat) in coords:
			pbnode = pb.Node()
			pbnode.id = int(osmid)
			pbnode.lat = int(lat*scale)
			pbnode.lon = int(lon*scale)
			self.Map.nodes.append(pbnode)

	def relations_cb(self,relations):
		for osmid,tags,refs in relations:
			if "boundary" in tags:
			    continue

			if "type" in tags and tags["type"]=="multipolygon":
				pbpol = pb.Multipolygon()
				for key in tags.keys():
					if key in waysConf:
						if waysConf[key].keys()==["*"]:
							pbpol.type = waysConf[key]["*"]
						val = tags[key]
						if val in waysConf[key]:
							pbpol.type = waysConf[key][val]
				pbpol.id = int(osmid)
				pbpol.refs.extend([int(item[0]) for item in refs])
				pbpol.roles.extend([pbpol.OUTER if item[2]=="outer" else pbpol.INNER for item in refs])
				self.Map.multipols.append(pbpol)


def loadWaysConf(filename):
	config = {}
	with open(filename) as conffile:
		config = yaml.load(conffile.read())
	ways={}
	str2pb = {"ignore": pbtypes.IGNORE, 
			"barrier":pbtypes.BARRIER, 
			"railway":pbtypes.RAILWAY, 
			"water": pbtypes.WATER, 
			"park": pbtypes.PARK, 
			"green":pbtypes.GREEN, 
			"forest":pbtypes.FOREST,
			"paved":pbtypes.PAVED,
			"unpaved":pbtypes.UNPAVED,
			"steps":pbtypes.STEPS,
			"highway":pbtypes.HIGHWAY,
                        "bridge":pbtypes.BRIDGE
			}
	for cat in config["Way"].keys():
		catenum = str2pb[cat]
		for key in config["Way"][cat]:
			if key not in ways:
				ways[key] = {}
			for value in config["Way"][cat][key]:
				ways[key][value]=catenum
	return ways


def loadBoolConf(filename):
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

def parseOSMfile(filename,waysConf,areasConf,bridgesConf,tunnelsConf):	
	cnt = counter(waysConf,areasConf,bridgesConf,tunnelsConf)
	p = OSMParser(concurrency=4, ways_callback=cnt.ways_cb, nodes_callback=cnt.nodes_cb, relations_callback=cnt.relations_cb, coords_callback=cnt.coords_cb)
	p.parse(filename)
	cnt.Map.nodes.sort(key=lambda node: node.id)
	cnt.Map.ways.sort(key=lambda way: way.id)
	for i in range(len(cnt.Map.nodes)):
		cnt.Map.nodesidx[cnt.Map.nodes[i].id]=i
	return cnt.Map

		
# Where find YAML config files
configDir="../../config/"
# Where save PBF file
dataDir="../../data/"

# Load config from YAML
waysConf=loadWaysConf(configDir+"types.yaml")
areasConf=loadBoolConf(configDir+"area.yaml")
bridgesConf=loadBoolConf(configDir+"bridge.yaml")
tunnelsConf=loadBoolConf(configDir+"tunnel.yaml")

# Parse file
start = time.time()
amap = parseOSMfile("../../osm/praha.osm",waysConf,areasConf,bridgesConf,tunnelsConf)
end = time.time()
print "Parsing took "+str(end-start)

# Delete nodes without ways
start = time.time()
nodeways=nodeWays(amap)
amap = deleteAloneNodes(amap,nodeways)
end = time.time()
print "Deleting alone nodes took "+str(end-start)

# Write map to file
start = time.time()
outfile = open(dataDir+"praha-pre.pbf","w")
outfile.write(amap.toPB().SerializeToString())
outfile.close()
end = time.time()
print "Saving took "+str(end-start)

print "Ways:",len(amap.ways)
print "Nodes:",len(amap.nodes)
print "Nodes on ways",len(nodeways)

