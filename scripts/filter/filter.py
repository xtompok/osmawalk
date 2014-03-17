#!/usr/bin/python
# -*- coding: utf8 -*-

import sys
sys.path.append("../imposm-parser")

import time
import math
import yaml
import pprint
import premap_pb2 as pb
import networkx as nx
from utils import angle
from Map import Map
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
	print "merging",wayids
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
	newway.render = False
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
			if way.type != pb.Way.BARRIER or way.area!=True:
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

