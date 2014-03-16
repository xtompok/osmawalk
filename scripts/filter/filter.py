#!/usr/bin/python
# -*- coding: utf8 -*-

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
	
	def loadFromPB(self,filename):
		f = open(filename,"r")
		pbMap = pb.Map()
		pbMap.ParseFromString(f.read())
		f.close()
		self.nodes = list(pbMap.nodes)
		self.ways = list(pbMap.ways)
		self.relations = list(pbMap.relations)
		for i in range(len(self.nodes)):
			self.nodesidx[self.nodes[i].id]=i
		


def nodeWays(amap):
	missingcnt=0
	nodeways = [ [] for i in amap.nodes]
	for way in amap.ways:
		missing = []
		for node in way.refs:
			try:
				nodeways[amap.nodesidx[node]].append(way.id)
			except IndexError:
				missingcnt+=1
				missing.append(node)
		for node in missing:
			way.refs.remove(node)
	print "Missing "+str(missingcnt)+" nodes"

	return nodeways	


def binary_search(alist,func,item):
    lo = 0
    hi = len(alist)
    mid = 1
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
	return intervals


"""
Postup:
 - najdi všechny body na poledníku
 - seřaď je od jihu k severu
 - jdi od jihu
   - vezmi bod
   - seřaď jeho sousedy podle úhlu od jihu k severu
   - smaž všechny intervaly, které v něm končí
   - je bod uvnitř intervalu budovy b?
     - je nejnižší hrana hranou b?
       - ne - budova je broken
       - ano
        - je nejvyšší hrana hranou b?
	  - ne - budova je broken
	  - ano 
	    - rozdělit interval
	    - vyřešit vnitřek
   - je bod na kraji intervalu budovy b?
     - horní
       - je nejnižší hrana hranou b?
         - ne - budova je broken
	 - ano
	   - uprav interval
     - dolní
       - je nejvyšší hrana hranou b?
         - ne - budova je broken
	 - ano
	   - uprav interval
   - vyřešit vnitřek

Vyřešit vnitřek:
 - jdi po kružnici od jihu
 - je cesta budova?
  - ne - vezmi další
  - ano - budova b
    - je další cesta budova b?
      - ano - uprav interval
      - ne
        - b je broken
	- ber cesty, dokud nenarazíš na b
	- uprav interval

"""

def sortedNeighs(amap,vectors,nodeidx):
	lat = amap.nodes[nodeidx].lat
	lon = amap.nodes[nodeidx].lon
	try:
		neighs = vectors[nodeidx]
	except KeyError:
		#print "Key Error: "+str(amap.nodes[nodeidx].id)
		return []
	angles = []
	top = []
	bot = []
	for n in neighs:
		nlat = amap.nodes[n[0]].lat
		nlon = amap.nodes[n[0]].lon
		if nlon == lon:
			if nlat>lat:
				top.append(n)
			elif nlat<lat:
				bot.append(n)
			continue
		angle = (1.0*nlat-lat)/(1.0*nlon-lon)
		angles.append((angle,n))
	angles.sort(key=lambda a: a[0])
	sneighs = bot+[a[1] for a in angles]+top
	out = []
	lastidx = 0
	for n in sneighs:
		if n[0] == lastidx:
			if lastidx == 0:
				print "Error in "+str(amap.nodes[n[0]].id)
				continue
			out[-1].append(n)
		else:
			out.append([n])
			lastidx = n[0] 
	return out

class Edge:
	fromlat = 0
	fromlon = 0
	tolat = 0
	tolon = 0
	vectoridx = 0

	def __init__(self,vectors,amap,fromidx,toidx):
		self.fromlat = amap.nodes[fromidx].lat
		self.fromlon = amap.nodes[fromidx].lon
		self.tolat = amap.nodes[vectors[toidx][0]].lat
		self.tolon = amap.nodes[vectors[toidx][0]].lon
		self.vectoridx = toidx

def zametani(amap,vectors):
	nodes = sorted(amap.nodes,key=lambda node: node.lon)
	broken = {}
	intervals = []
	graph = []
	ends = {}
	nodeidx=0
	while nodeidx<len(nodes):
		actual=[nodes[nodeidx]]
		while (nodeidx<len(nodes)-1) and (nodes[nodeidx].lon == nodes[nodeidx+1].lon):
			actual.append(nodes[nodeidx+1])
			nodeidx+=1
		nodeidx+=1
		if nodeidx<len(nodes)-1:
			nextlon = nodes[nodeidx].lon
		else:
			 return graph
		 
		#print nextlon
		#print nodeidx

		actual.sort(key=lambda node: node.lat)
		for node in actual:
			neighs = sortedNeighs(amap,vectors,amap.nodesidx[node.id])
"""
			inside = isInside(node,intervals)
			buildings = []
			isbroken = False
			for n in neighs:
				if amap.ways[n[1]].area == False:
					isbroken = True
					continue
				if amap.ways[n[1]].type != pb.Way.BARRIER:
					isbroken = True
					continue
				buildings.append(n)
			if inside == -1:
				buildings.sort(key=lambda b: amap.nodes[b[0]].lat)
				i = 0
				while i+1 < len(buildings):
					if buildings[i][1]!=buildings[i+1][1]:
						print "Error in buildings!"
					aint = Interval()
					aint.lfrom = amap.nodesidx[node.id]
					aint.lto = buildings[i][0]
					aint.ufrom = aint.lfrom
					aint.uto = buildings[i+1][0]
					aint.way_idx = buildings[i][1]
					idx = binary_search(intervals,lambda aint: aint.upper,node.lat)
					intervals.insert(idx+1,aint)

					broken[aint.way_idx] = isbroken

					i+=2
			else:
				buildings.sort(key=lambda b: b[1])
				remove = []
				iidx = 0
				for inter in intervals:
					if inter.uto == node.id and inter.lto == node.id:
						remove.append(i)
					
					bidx = binary_search(buildings,lambda b:b[1],inter.way_idx)
					if inter.uto == node.id and bidx > 0:
						inter.ufrom = amap.nodesidx[node.id]
						inter.uto = buildings[bidx][0]
						buildings.remove(buildings[bidx])

					if inter.lto == node.id and bidx > 0:
						inter.lfrom = amap.nodesidx[node.id]
						i.lto = buildings[bidx][0]
						buildings.remove(buildings[bidx])
					
					if iidx+1 < len(intervals) and inter.uto == intervals[iidx+1].lto and inter.uto == node.id:
						intervals[iidx+1].lfrom = inter.lfrom
						intervals[iidx+1].lto = inter.lto
						remove.append(i)

				buildings.sort(key=lambda b: amap.nodes[b[0]].lat)
				i = 0
				while i+1 < len(buildings):
					if buildings[i][1]!=buildings[i+1][1]:
						print "Error in buildings!"
					aint = Interval()
					aint.lfrom = amap.nodesidx[node.id]
					aint.lto = buildings[i][0]
					aint.ufrom = aint.lfrom
					aint.uto = buildings[i+1][0]
					aint.way_idx = buildings[i][1]
					idx = binary_search(intervals,lambda aint: aint.upper,node.lat)
					intervals.insert(idx+1,aint)

					broken[aint.way_idx] = isbroken

					i+=2

					iidx +=1 
				for inter in remove:
					intervals.remove(inter)

				 
				 
				
		intervals = updateIntervals(amap.nodes,intervals,nextlon)

		"""


start = time.time()

amap = Map()
amap.loadFromPB("praha-pre.pbf")

end = time.time()
print "Loading took "+str(end-start)
start = time.time()

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

end = time.time()
print "Zametani took "+str(end-start)
start = time.time()

del vectors
del nodeways

outfile = open("praha-union.pbf","w")
outfile.write(amap.toPB().SerializeToString())
outfile.close()

end = time.time()
print "Saving took "+str(end-start)

#print "Ways:",cnt.ways
#print "Nodes:",cnt.nodes
#print "Relations:",cnt.relations
#print "Areas:",cnt.areas
#print "Nodes on ways",len(nodeways.keys())

