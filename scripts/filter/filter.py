#!/usr/bin/python
# -*- coding: utf8 -*-

import sys
sys.path.append("../imposm-parser")

import time
import math
import yaml
import pprint
import premap_pb2 as pb
import types_pb2 as pbtypes
import networkx as nx
from utils import angle
from Map import Map
from Raster import Raster
from imposm.parser import OSMParser

scale = 1000000

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

	
def onBorder(amap,neighs,nodeid,lastnodeid):
	lastnode = amap.nodes[amap.nodesidx[lastnodeid]]
	node = amap.nodes[amap.nodesidx[nodeid]]

	vx = lastnode.lon-node.lon
	vy = lastnode.lat-node.lat
	
	neighangles = []
	for nid in neighs[node.id]:
		if nid==lastnodeid:
			continue
		#print "with ",nid
		n = amap.nodes[amap.nodesidx[nid]]
		ux = n.lon-node.lon
		uy = n.lat-node.lat
		ang = angle(ux,uy,vx,vy)
		neighangles.append((ang,nid))
	neighangles.sort(key=lambda n: n[0])
	return neighangles[-1][1]


def firstBorder(amap,neighs,node):
	vx = 0
	vy = -1000
	
	neighangles = []
	#print "Node ",node.id,"neighs:",neighs[node.id]
	for nid in neighs[node.id]:
		n = amap.nodes[amap.nodesidx[nid]]
		ux = n.lon-node.lon
		uy = n.lat-node.lat
		ang = angle(ux,uy,vx,vy)
		neighangles.append((ang,nid))
	neighangles.sort(key=lambda n: n[0])
	return neighangles[-1][1]


def mergeWays(amap,wayids):
	newway = pb.Way()
	way1 = amap.ways[amap.waysidx[wayids[0]]]
	neighs = {}
	nodes = []
	for wayid in wayids:
		way = amap.ways[amap.waysidx[wayid]]
		waynodes = [amap.nodes[amap.nodesidx[ref]] for ref in way.refs]
		if len(waynodes)<3:
			continue
		if len(waynodes)==3 and waynodes[-1]==waynodes[0]:
			continue
		if waynodes[-1] != waynodes[0]:
			waynodes.append(waynodes[0])
		for i in range(len(waynodes)):
			wniid = waynodes[i].id
			if wniid not in neighs.keys():
				neighs[wniid] = []
				nodes.append(waynodes[i])	
			if i!=0:
				if waynodes[i-1].id not in neighs[wniid]:
					neighs[wniid].append(waynodes[i-1].id)
			if i!=len(waynodes)-1:
				if waynodes[i+1].id not in neighs[wniid]:
					neighs[wniid].append(waynodes[i+1].id)

	if len(nodes)<3:
		print "merging",wayids
		print "Merge failed -- too few nodes"
		return None
	bylon = sorted(nodes,key=lambda n:n.lon)
	bylatlon = sorted(bylon,key=lambda n:n.lat)

	first = bylatlon[0]
	#print neighs[first.id]
	second = amap.nodes[amap.nodesidx[firstBorder(amap,neighs,first)]]
	#print "edge:",first.id," ",second.id
	newway.refs.append(first.id)
	newway.refs.append(second.id)
	nextid = onBorder(amap,neighs,second.id,first.id)
	while(nextid != first.id):
		nextid = onBorder(amap,neighs,newway.refs[-1],newway.refs[-2])
		if nextid in newway.refs and nextid != first.id:
			print "merging",wayids
			print "Merge failed -- wrong cycle"
			print newway.refs,", repeated:",nextid
			return None
		newway.refs.append(nextid)
	#	print "F",first.id,"n",nextid
	#	print newway.refs
	newway.refs.append(first.id)
	newway.type = way1.type
	newway.id = -way1.id #FIXME
	newway.area = way1.area
	newway.render = True
	return newway


def makeNeighGraph(amap,nodeways):
	G = nx.Graph()
	G.add_nodes_from([way.id for way in amap.ways])
	broken = {way.id : False for way in amap.ways }
	bcnt = 0
	for n in amap.nodes:
		nidx = amap.nodesidx[n.id]
		ways = nodeways[nidx]
		isbroken = False
		buildings = []
		for wayid in ways:
			wayidx = amap.waysidx[wayid]
			way = amap.ways[wayidx]
			if way.type != pbtypes.BARRIER or way.area!=True:
				isbroken = True
			else:
				bcnt+=1
				buildings.append(wayidx)
		if isbroken:
			for wayidx in buildings:
				broken[amap.ways[wayidx].id]=True
		else:
			firstwayid = amap.ways[buildings[0]].id
			for wayidx in buildings[1:]:
				G.add_edge(firstwayid,amap.ways[wayidx].id)
	print bcnt
	return (G,broken)

def mergeComponents(amap,G,broken):
	remove = []
	for comp in nx.connected_components(G):
		nbc = [c for c in comp if broken[c]==False]
		if len(nbc) <= 1:
			continue
		way = mergeWays(amap,nbc)
		if way!=None:
			remove += nbc
			amap.ways.append(way)
	return remove

def removeMerged(amap,remove):
	removeidxs = [amap.waysidx[r] for r in remove]
	removeidxs.sort()
	toidx = 0
	ridx = 0
	for fromidx in range(len(amap.ways)):
		if ridx<len(removeidxs) and fromidx == removeidxs[ridx]:
			ridx+=1
			continue
		amap.ways[toidx] = amap.ways[fromidx]
		toidx += 1
	for i in range(len(amap.ways)-1,toidx,-1):
		del amap.ways[i]

def getbbox(amap,wayid):
	way = amap.ways[amap.waysidx[wayid]]
	nodeids = way.refs
	minlon = 10**10
	minlat = 10**10
	maxlon = 0
	maxlat = 0
	for nid in nodeids:
		node = amap.nodes[amap.nodesidx[nid]]
		maxlon = max(maxlon,node.lon)
		maxlat = max(maxlat,node.lat)
		minlon = min(minlon,node.lon)
		minlat = min(minlat,node.lat)
	return (minlon,minlat,maxlon,maxlat)


		
def isIn(amap,node,way):
	if way.area == False:
		if node.id in way.refs:
			return True
		else:
			return False
	bbox = getbbox(amap,way.id)
	if node.lon < bbox[0] and node.lon > bbox[2] and node.lat < bbox[1] and node.lat > bbox[3]:
		return False
	intcnt = 0
	up = False
	lat = node.lat
	lon = node.lon
	memnode = amap.nodes[amap.nodesidx[way.refs[0]]]
	if memnode.lat == lat and memnode.lon >= lon: #FIXME
		return True
	for nid in way.refs[1:]:
		node = amap.nodes[amap.nodesidx[nid]]
		if node.lat == lat and node.lon == lon:
			return True
		if (memnode.lon < lon) and node.lon < lon:
			memnode = node
			continue
		if (memnode.lat < lat and node.lat < lat) or (memnode.lat > lat and node.lat > lat):
			memnode = node
			continue
		if (memnode.lon >= lon) and node.lon >=lon:
			if (memnode.lat < lat and node.lat > lat) or (memnode.lat>lat and node.lat<lat):
				intcnt+=1
				memnode = node
				continue
			if (node.lat==lat):
				if memnode.lat > lat:
					up = True
				elif memnode.lat < lat:
					up = False
				memnode = node
				continue
			if memnode.lat==lat:
				if node.lat > lat:
					if not up:
						intcnt+=1
				if node.lat < lat:
					if up:
						intcnt+=1
				memnode = node
				continue
		if (memnode.lat < lat and node.lat > lat) or (memnode.lat>lat and node.lat<lat):
			dlon = node.lon - memnode.lon
			dlat = node.lat - memnode.lat
			ndlat = lat - memnode.lat
			if (memnode.lon+dlon*(ndlat*1.0/dlat) >= lon):
				intcnt +=1
			memnode = node
			continue
		if memnode.lat == lat:
			return True
		if memnode.lat < lat:
			up = False
		elif memnode.lat > lat:
			up = True
		memnode = node 
		continue
	
	if intcnt%2 == 0:
		return False
	return True

def markInside(amap,raster):
	for way in amap.ways:
		if not (way.area and way.type==pbtypes.BARRIER) :
			continue
		bbox = getbbox(amap,way.id)
		minbox = raster.getBox(bbox[0],bbox[1])
		maxbox = raster.getBox(bbox[2],bbox[3])
		nodes  = []
		for i in range(minbox[0],maxbox[0]+1):
			for j in range(minbox[1],maxbox[1]+1):
				nodes += raster.raster[i][j]
		for node in nodes:
			isIn(amap,amap.nodes[amap.nodesidx[node]],way)
		print "Way ",way.id," should collide with ",len(nodes)," nodes."

def mergeMultipolygon(amap,wayids):
	newway = pb.Way()
	way1 = amap.ways[amap.waysidx[wayids[0]]]
	neighs = {}
	nodeways = {}
	for wayid in wayids:
		way = amap.ways[amap.waysidx[wayid]]
		waynodes = [amap.nodes[amap.nodesidx[ref]] for ref in way.refs]
		for i in range(len(waynodes)):
			wniid = waynodes[i].id
			if wniid not in nodeways:
				nodeways[wniid] = []
			nodeways[wniid].append(wayid)
			if wniid not in neighs.keys():
				neighs[wniid] = []
			if i!=0:
				if waynodes[i-1].id not in neighs[wniid]:
					neighs[wniid].append(waynodes[i-1].id)
			if i!=len(waynodes)-1:
				if waynodes[i+1].id not in neighs[wniid]:
					neighs[wniid].append(waynodes[i+1].id)


	for k,v in neighs.iteritems():
		if len(v)!=2:
			print "Error in node",k
			print neighs
			return (None,None)

	first = amap.nodes[amap.nodesidx[way1.refs[0]]]
	second = amap.nodes[amap.nodesidx[neighs[first.id][0]]]
	#print "edge:",first.id," ",second.id
	newway.refs.append(first.id)
	newway.refs.append(second.id)
	nextid = neighs[newway.refs[-1]][0]
	if nextid == newway.refs[-2]:
		nextid = neighs[newway.refs[-1]][1]
	while(nextid != first.id):
		nextid = neighs[newway.refs[-1]][0]
		if nextid == newway.refs[-2]:
			nextid = neighs[newway.refs[-1]][1]
		if nextid in newway.refs and nextid != first.id:
			print "merging",wayids
			print "Merge failed -- wrong cycle"
			print newway.refs,", repeated:",nextid
			return (None,None)
		newway.refs.append(nextid)
	#	print "F",first.id,"n",nextid
	#	print newway.refs
	for nodeid in newway.refs:
		for wayid in nodeways[nodeid]:
			if wayid in wayids:
				wayids.remove(wayid)
	newway.refs.append(first.id)
	newway.type = way1.type
	newway.id = -way1.id #FIXME
	newway.render = True
	return (newway,wayids)

def mergeMultipolygons(amap):
	print "Multipolygons: ",len(amap.multipols)
	winner = 0
	woinner = 0
	for multi in amap.multipols:
		outer = []
		hasInner = False
		for i in range(len(multi.roles)):
			if multi.roles[i] == pb.Multipolygon.INNER:
				hasInner = True
			else:
				if multi.refs[i] in amap.waysidx.keys():
					outer.append(multi.refs[i])
		if hasInner:
			winner +=1
		else:
			woinner +=1
		if len(outer)<=1:
			continue
		print "Merging",len(outer),"in multipolygon",multi.id
		while (len(outer)>0):
			(way,outer) = mergeMultipolygon(amap,outer)
			if way == None:
				print "Merging Error"
				break
			print "Remains",len(outer),"ways"
			if multi.type != pbtypes.WAY:
				way.type = multi.type
			way.area = True
			print "Way created"
			amap.ways.append(way)

	print "With Inner: ",winner
	print "Without Inner: ",woinner

			




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

mergeMultipolygons(amap)

end = time.time()
print "Multipolygons took "+str(end-start)
start = time.time()

raster = Raster(amap)
#markInside(amap,raster)

end = time.time()
print "Making raster took "+str(end-start)
start = time.time()

(G,broken) = makeNeighGraph(amap,nodeways)
print "Components",len(nx.connected_components(G))

end = time.time()
print "Neighs took "+str(end-start)
start = time.time()

remove = mergeComponents(amap,G,broken)

end = time.time()
print "Merge took "+str(end-start)
start = time.time()

print "To remove:",len(remove)
removeMerged(amap,remove)

end = time.time()
print "Removing took "+str(end-start)
start = time.time()


outfile = open("praha-union.pbf","w")
outfile.write(amap.toPB().SerializeToString())
outfile.close()

end = time.time()
print "Saving took "+str(end-start)

