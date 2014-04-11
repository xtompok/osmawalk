#!/usr/bin/python
# -*- coding: utf8 -*-

import sys
sys.path.append("../imposm-parser")

import time
import math
import yaml
import pprint
import pyproj
import heapq
import bst
import premap_pb2 as pb
import types_pb2 as pbtypes
import networkx as nx
from utils import angle,int2deg,deg2int,distance
from Map import Map
from Raster import Raster

scale = 1000000
walkTypes = [pbtypes.PAVED,pbtypes.UNPAVED,pbtypes.STEPS,pbtypes.HIGHWAY]

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

def deleteAloneNodes(amap,nodeways):
	toidx = 0
	for fromidx in range(len(amap.nodes)):
		if len(nodeways[fromidx])==0:
			continue
		amap.nodes[toidx]=amap.nodes[fromidx]
		toidx+=1
	for i in range(len(amap.nodes)-1,toidx,-1):
		del amap.nodes[i]

	
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
			if wniid not in neighs:
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
	newway.id = amap.newWayid()
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
	if node.id in way.refs:
		return True
	elif way.area == False:
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
	incnt = 0
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
			if isIn(amap,amap.nodes[amap.nodesidx[node]],way):
				amap.nodes[amap.nodesidx[node]].inside = True
				incnt+=1
			
		print "Way ",way.id," should collide with ",len(nodes)," nodes."
	print "Nodes:",len(amap.nodes),"inside",incnt
def unmarkBorderNodes(amap):
	waycnt = 0
	for way in amap.ways:
		if way.area or way.type==pbtypes.BARRIER or way.type == pbtypes.IGNORE:
			continue
		memnode = amap.nodes[amap.nodesidx[way.refs[0]]]
		border = False
		for nodeid in way.refs[1:]:
			node = amap.nodes[amap.nodesidx[nodeid]]
			if memnode.inside==node.inside:
				memnode = node
				continue
			if memnode.inside and not node.inside:
				memnode.inside = False
				memnode = node 
				continue
			if border:
				memnode = node
				border = False
				continue
			node.inside = False
			border = True
		waycnt+=1
	print "Non-barrier ways:",waycnt


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
			if wniid not in neighs:
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
	newway.id = amap.newWayid()
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
				if multi.refs[i] in amap.waysidx:
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

def divideLongEdges(amap):
	longcnt = 0
	edgecnt = 0
	geod = pyproj.Geod(ellps="WGS84")
	for wayidx in range(len(amap.ways)):
		way = amap.ways[wayidx]		
		if not (way.type in walkTypes):
			continue
		newway = pb.Way()
		replace = False
		for i in range(len(way.refs)-1):
			ref1 = amap.nodes[amap.nodesidx[way.refs[i]]]
			ref2 = amap.nodes[amap.nodesidx[way.refs[i+1]]]
			newway.refs.append(ref1.id)
			try:
				r1lon = int2deg(ref1.lon)
				r1lat = int2deg(ref1.lat)
				r2lon = int2deg(ref2.lon)
				r2lat = int2deg(ref2.lat)
				(azim,_,dist)=geod.inv(r1lon,r1lat,r2lon,r2lat)
			except ValueError:
				continue
			if dist<30:
				continue
			replace=True
			lonlats = geod.npts(r1lon,r1lat,r2lon,r2lat,dist/20)
			for lon,lat in lonlats:
				newnode = pb.Node()
				newnode.id = amap.newNodeid()
				newnode.lon = deg2int(lon)
				newnode.lat = deg2int(lat)
				newnode.inside = ref1.inside and ref2.inside
				newway.refs.append(newnode.id)
				amap.nodes.append(newnode)
		if not replace:
			continue
		newway.type = way.type
		newway.id = way.id
		newway.area = way.area
		newway.barrier = way.barrier
		newway.bordertype = way.bordertype
		newway.refs.append(way.refs[-1])
		amap.ways[wayidx] = newway
		
def makeGraph(amap):
	barGraph = []
	wayGraph = {node.id:[] for node in amap.nodes}
	for way in amap.ways:
		if way.type in walkTypes:
			for i in range(len(way.refs)-1):
				wayGraph[way.refs[i]].append(way.refs[i+1])
				wayGraph[way.refs[i+1]].append(way.refs[i])
		elif way.type in [pbtypes.BARRIER]:
			for i in range(len(way.refs)-1):
				barGraph.append((way.refs[i],way.refs[i+1]))
	return (wayGraph,barGraph)
	
def makeDirectCandidates(amap,raster,wayGraph,maxdist):
	candidates = []
	for lonidx in range(raster.lonparts-1):
		for latidx in range(raster.latparts-1):
			waynodes = [nid for nid in raster.raster[lonidx][latidx] if wayGraph[nid] != []]
			candidates.extend([(u,v) for u in waynodes for v in waynodes if u<v])
			waynodesNeigh = [nid for nid in raster.raster[lonidx+1][latidx] if wayGraph[nid] != []]
			candidates.extend([(u,v) for u in waynodes 
					for v in waynodesNeigh 
					if distance(amap.nodes[amap.nodesidx[u]],amap.nodes[amap.nodesidx[v]]) <= 20])
			waynodesNeigh = [nid for nid in raster.raster[lonidx+1][latidx+1] if wayGraph[nid] != []]
			candidates.extend([(u,v) for u in waynodes 
					for v in waynodesNeigh 
					if distance(amap.nodes[amap.nodesidx[u]],amap.nodes[amap.nodesidx[v]]) <= 20])
			waynodesNeigh = [nid for nid in raster.raster[lonidx][latidx+1] if wayGraph[nid] != []]
			candidates.extend([(u,v) for u in waynodes 
					for v in waynodesNeigh 
					if distance(amap.nodes[amap.nodesidx[u]],amap.nodes[amap.nodesidx[v]]) <= 20])
        print "Unfiltered candidates:",len(candidates)
	remove = []
	for i in range(len(candidates)):
		(n1id,n2id) = candidates[i]
		if n2id in wayGraph[n1id]:
			remove.append(i)
	
	toidx = 0
	remidx = 0
	i = 0
	while (i<len(candidates)):
		if remidx < len(remove) and i == remove[remidx]:
			remidx+=1
			i+=1
			continue
		candidates[toidx] = candidates[i]
		toidx+=1
		i+=1
	for i in range(len(candidates)-1,toidx-1,-1):
		del candidates[i]
		
	print "Candidates:",len(candidates)
	return candidates
			
class Line:
	startlon = 0
	startid = 0
	startlat = 0
	endlon = 0
	endlat = 0
	endid = 0
	isBar = False
	broken = False
	angle = 0

	def __init__(self,start,end,isBar=False):
		self.startlon=start.lon
		self.startlat=start.lat
		self.endlon=end.lon
		self.endlat=end.lat
		self.startid=start.id
		self.endid=end.id
		self.isBar=isBar
		self.broken=False
		self.angle = angle(0,-100,self.endlon-self.startlon,self.endlat-self.startlat)

	def __eq__(self,other):
		if isinstance(other,Line):
			return self.__dict__ == other.__dict__
		else:
			return False

	def __cmp__(self,other):
		if not isinstance(other,Line):
			return NotImplemented
		return cmp((self.startid,self.endid,self.isBar),(other.startid,other.endid,other.isBar))
	def __str__(self):
		return "Start:"+str(self.startid)+" End:"+str(self.endid)+" Angle:"+str(self.angle)

	def calcLatForLon(self,lon):
		minlon = self.startlon
		maxlon = self.endlon
		if lon > maxlon:
			print "lon greater maxlon"
		minlat = min(self.startlat,self.endlat)
		maxlat = max(self.startlat,self.endlat)
		return ((lon-minlon)/(maxlon-minlon))*(maxlat-minlat)+minlat

	def compare(self,lon):
		return (self.calcLatForLon(lon),angle)
	
class Event:
	START = 2
	END = 0
	INTERSECT = 1

	lon = 0
	lat = 0
	angle = 0
	
	cat = 0
	lines = []

	def __cmp__(self,other):
		if not isinstance(other,Event):
			return NotImplemented
		return cmp((self.lon,self.cat,self.lat,self.angle),(other.lon,other.cat,other.lat,other.angle))


"""	def __cmp__(self,other):
		if not isinstance(other,Event):
			return NotImplemented
		if self.lon != other.lon:
			return cmp(self.lon,other.lon)
		if self.lat != other.lat:
			return cmp(self.lat,other.lat)
		sangle = angle(0,-1,self.end.lon-self.start.lon,self.end.lat-self.start.lat)
		oangle = angle(0,-1,other.end.lon-other.start.lon,other.end.lat-other.start.lat)
		return cmp(sangle,oangle)
"""			


def findDirectWays(amap,candidates,barGraph):
	lines = []
	for (n1id,n2id) in candidates:
		n1 = amap.nodes[amap.nodesidx[n1id]]
		n2 = amap.nodes[amap.nodesidx[n2id]]
		if (n2.lon < n1.lon):
			line = Line(n2,n1)
		else:
			line = Line(n1,n2)
		lines.append(line)

	for (n1id,n2id) in barGraph:
		n1 = amap.nodes[amap.nodesidx[n1id]]
		n2 = amap.nodes[amap.nodesidx[n2id]]
		if (n2.lon < n1.lon):
			line = Line(n2,n1,isBar=True)
		else:
			line = Line(n1,n2,isBar=True)
		lines.append(line)
	print "Lines:",len(lines)
	lines.sort()
	newlines = []
	memline = lines[0]
	newlines.append(memline)
	for line in lines[1:]:
		if memline!=line:
			newlines.append(line)
		memline = line

	lines = newlines

	print "Lines:",len(lines)

	queue = []
	for line in lines:
		evt = Event()
		evt.cat = evt.START
		evt.lon = line.startlon
		evt.lat = line.startlat
		evt.angle = line.angle
		evt.lines = [line]
		heapq.heappush(queue,evt)
		evt = Event()
		evt.cat = evt.END
		evt.lon = line.endlon
		evt.lat = line.endlat
		evt.angle = -line.angle
		evt.lines = [line]
		heapq.heappush(queue,evt)
	
	tree = bst.BinarySearchTree(100*len(queue))
	while len(queue) > 0:
		evt = heapq.heappop(queue)
		line = evt.lines[0]
		print line
		if line.startlon==line.endlon:
		#	print "Special case"
			continue
		if evt.cat==evt.START:
			(left,right)=tree.put(line,evt.lon)
			if left:
				col = calcCollision(line,left)
				if col:
					evt = Event()
					evt.cat = evt.INTERSECTION
					evt.lon = cal[0]
					evt.lat = cal[1]
					evt.lines = [line1,line2]
					heapq.heappush(queue,evt)
					line1.broken=True
					line2.broken=True
			if right:
				col = calcCollision(line,right)
				if col:
					evt = Event()
					evt.cat = evt.INTERSECTION
					evt.lon = cal[0]
					evt.lat = cal[1]
					evt.lines = [line1,line2]
					heapq.heappush(queue,evt)
					line1.broken=True
					line2.broken=True

			l =tree.get(line,evt.lon)
			if evt.lines[0] == l:
				pass
			else:
				pass
				#print "FAIL"
			continue
		if evt.cat==evt.END:
			tree.delete(evt.lines[0],evt.lon)
			continue
		if evt.cat==evt.INTERSECT:
			tree.delete(evt.lines[0],evt.lon)
			tree.delete(evt.lines[1],evt.lon)
			tree.put(evt.lines[0],evt.lon)
			tree.put(evt.lines[1],evt.lon)
			continue
	return lines


def calcCollision(line1,line2):
	lon1 = line1.startlon
	lon2 = line1.endlon
	lat1 = line1.startlat
	lat2 = line1.endlat
	lon3 = line2.startlon
	lon4 = line2.endlon
	lat3 = line2.startlat
	lat4 = line2.endlat
	cit = (lat2-lat1)*(lon4-lon3)*lon1+(lon2-lon1)*(lon4-lon3)*(lat3-lat1)-(lat4-lat3)*(lon2-lon1)*lon3
	jm = (lat2-lat1)*(lon4-lon3)-(lon2-lon1)*(lat4-lat3)
	if jm == 0:
		return None
	lon = cit/jm
	lat = (lon*(lat2-lat1)+lon1*(lon2-lon1)-lon1*(lat2-lat1))/(lon2-lon1)
	if (lon1 < lon and lon < lon2 and 
		lon3 < lon and lon < lon4 and 
		lat1 < lat and lat < lat2 and 
		lat3 < lat and lat < lat4):
		return (lon,lat)
	return None

def makeDirect(amap,lines):
	for line in lines:
		if (not line.broken) and (not line.isBar):
			way = pb.Way()
			way.type = pbtypes.DIRECT
			way.area = False
			way.id = amap.newWayid()
			way.barrier = False
			way.render = True
			way.refs.append(line.startid)
			way.refs.append(line.endid)
			amap.ways.append(way)
	


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
amap.updateWaysIdx()

end = time.time()
print "Multipolygons took "+str(end-start)
start = time.time()

(G,broken) = makeNeighGraph(amap,nodeways)
print "Components",len(nx.connected_components(G))

end = time.time()
print "Neighs took "+str(end-start)
start = time.time()

remove = mergeComponents(amap,G,broken)
print "To remove:",len(remove)
removeMerged(amap,remove)
amap.updateWaysIdx()

end = time.time()
print "Merge took "+str(end-start)
start = time.time()

nodeways=nodeWays(amap)
deleteAloneNodes(amap,nodeways)
amap.updateNodesIdx()

end = time.time()
print "Deleting alone nodes took "+str(end-start)
start = time.time()

raster = Raster(amap)

end = time.time()
print "Making raster took "+str(end-start)
start = time.time()

markInside(amap,raster)
unmarkBorderNodes(amap)

end = time.time()
print "Marking inside nodes took "+str(end-start)
start = time.time()

divideLongEdges(amap)
amap.updateNodesIdx()

end = time.time()
print "Long edges took "+str(end-start)
start = time.time()

#for lon in range(raster.lonparts):
#    for lat in range(raster.latparts):
#        print "raster[",lon,"][",lat,"]=",len(raster.raster[lon][lat])
(wayGraph,barGraph)=makeGraph(amap)
candidates = makeDirectCandidates(amap,raster,wayGraph,20)
#lines = findDirectWays(amap,candidates,barGraph)
#makeDirect(amap,lines)

end = time.time()
print "Making graph took "+str(end-start)
start = time.time()


print len(amap.nodes)," nodes, ",len(amap.ways)," ways"
outfile = open("praha-union.pbf","w")
outfile.write(amap.toPB().SerializeToString())
outfile.close()

end = time.time()
print "Saving took "+str(end-start)

