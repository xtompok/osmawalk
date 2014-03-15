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

def binary_search(alist,func,item):
    lo = 0
    hi = len(alist)
    while lo < hi:
        mid = (lo+hi)//2
        midval = alist[mid]
        if func(midval) < item:
            lo = mid+1
        elif func(midval) > item: 
            hi = mid
        else:
            return mid
    return -(mid-1)



	
def nodeVectors(amap,nodeways):
	vectors = {}
	for nodeidx in range(len(nodeways)):
		 for way_id in nodeways[nodeidx]:
			 wayidx = binary_search(amap.ways,lambda way: way.id, way_id)
			 way = amap.ways[wayidx]
			 wayrefs = [amap.nodesidx[r] for r in way.refs]
			 lastindex = 0
			 refs = []
			 if wayrefs.count(nodeidx)>2:
				 print "Error in node "+str(amap.nodes[nodeidx].id)+" on way "+str(way.id)
				 continue
			 for i in range(wayrefs.count(nodeidx)):
				 idx = wayrefs.index(nodeidx,lastindex)
				 refs.append(idx)
				 lastindex = idx
			 for i in refs:
				 if i != 0:
					 node2_idx = wayrefs[i-1]
					 if node2_idx < 0:
						 continue
					 if (amap.nodes[node2_idx].lon > amap.nodes[nodeidx].lon) or ( amap.nodes[node2_idx].lon == amap.nodes[nodeidx].lon  and amap.nodes[node2_idx].lat > amap.nodes[nodeidx].lat):
						 try:
						 	vectors[nodeidx].append((node2_idx,wayidx))
						 except KeyError:
						 	vectors[nodeidx] = [(node2_idx,wayidx)]
				 if i!= len(wayrefs)-1:
					 node2_id = wayrefs[i+1]
					 if node2_idx < 0:
						 continue
					 if (amap.nodes[node2_idx].lon > amap.nodes[nodeidx].lon) or ( amap.nodes[node2_idx].lon == amap.nodes[nodeidx].lon  and amap.nodes[node2_idx].lat > amap.nodes[nodeidx].lat):
						 try:
						 	vectors[nodeidx].append((node2_idx,wayidx))
						 except KeyError:
						 	vectors[nodeidx] = [(node2_idx,wayidx)]
	return vectors



	
class Interval:
	lfrom = 0
	lto = 0
	lower=0
	ufrom = 0
	uto = 0
	upper = 0
	way_idx = 0

def isInside(node,intervals):
	if len(intervals) == 0:
		return -1
	idx = binary_search(intervals, lambda interval: interval.lower, node.lat)
	if idx >= 0:
		return idx
	elif intervals[-idx].upper > node.lat:
		return -idx
	else:
		return -1
def updateIntervals(nodes,intervals,lon):
	for i in range(len(intervals)):
		aint = intervals[i]
		dlat = nodes[aint.lto].lat - nodes[aint.lfrom].lat
		dlon = nodes[aint.lto].lon - nodes[aint.lfrom].lon
		scale = (lon - nodes[aint.lfrom].lon)/dlon
		intervals[i].lower = dlat*scale + nodes[aint.lfrom].lat
		dlat = nodes[aint.uto].lat - nodes[aint.ufrom].lat
		dlon = nodes[aint.uto].lon - nodes[aint.ufrom].lon
		scale = (lon - nodes[aint.ufrom].lon)/dlon
		intervals[i].upper = dlat*scale + nodes[aint.ufrom].lat


def zametani(amap,vectors):
	nodes = sorted(amap.nodes,key=lambda node: node.lon)
	broken = {}
	intervals = []
	graph = []
	ends = {}
	i=0
	while i<len(nodes):
		actual=[nodes[i]]
		while (nodes[i].lon == nodes[i+1].lon)and(i<len(nodes)-1):
			actual.append(nodes[i+1])
			i+=1
		if i<len(nodes)-1:
			nextlon = nodes[i+1].lon
		else:
			 return graph
		 
		actual.sort(key=lambda node: node.lat)
		for node in actual:
			neighs = vectors[amap.nodesidx[node.id]]
			inside = isInside(node,intervals)
			if inside == -1:
				buildings = []
				broken = False
				for n in neighs:
					if amap.ways[n[1]].area == False:
						broken = True
						continue
					if amap.ways[n[1]].waytype != pb.Way.BARRIER:
						broken = True
						continue
					buildings.append(n)
				buildings.sort(key=lambda b: amap.nodes[b[0]].lat)
				i = 0
				while buildings != []:
					if buildings[i][1]!=buildings[i+1][1]:
						print "Error in buildings!"
					aint = Interval()
					interval.lfrom = amap.nodesidx[node.id]
					interval.lto = buildings[i][0]
					interval.ufrom = interval.lfrom
					interval.uto = buildings[i+1][0]
					interval.way_idx = buildings[i][1]
					idx = binary_search(intervals,lambda aint: aint.upper,node.lat)
					intervals.insert(idx+1,interval)

					try: 
						ends[
					i+=2


					


			else:
				pass 
				
		intervals = updateIntervals(amap.nodes,intervals,nextlon)

		


waysConf=loadWaysConf("types.yaml")
areasConf=loadAreasConf("area.yaml")

start = time.time()

amap = parseOSMfile("../../osm/praha.osm")

end = time.time()
print "Parsing took "+str(end-start)
start = time.time()

nodeways=nodeWays(amap)
amap = deleteAloneNodes(amap,nodeways)
nodeways=nodeWays(amap)

end = time.time()
print "Nodeways took "+str(end-start)
start = time.time()

vectors = nodeVectors(amap,nodeways)
print len(vectors)

end = time.time()
print "Vectors took "+str(end-start)
start = time.time()

zametani(amap,vectors)
del vectors
del nodeways

outfile = open("praha-pre.pbf","w")
outfile.write(amap.toPB().SerializeToString())
outfile.close()

end = time.time()
print "Saving took "+str(end-start)

print "Ways:",cnt.ways
print "Nodes:",cnt.nodes
print "Relations:",cnt.relations
print "Areas:",cnt.areas
print "Nodes on ways",len(nodeways.keys())

