#!/usr/bin/python
# -*- coding: utf8 -*-

import time
import premap_pb2 as pbmap
import graph_pb2 as pbgraph
import types_pb2 as pbtypes
from Map import Map
from Graph import Graph

def makeVerticesIndex(graph):
	index = {}
	for vertex in graph.vertices:
		index[vertex.osmid]=vertex.idx
	return index

def makeVertices(amap,graph):
	for node in amap.nodes:
		vertex = pbgraph.Vertex()
		vertex.osmid = node.id
		vertex.lat = node.lat
		vertex.lon = node.lon
		graph.vertices.append(vertex)
	
	graph.vertices.sort(key=lambda v:v.osmid)
	for i in range(len(graph.vertices)):
		graph.vertices[i].idx = i

	

def makeEdges(amap,graph,index):
	for way in amap.ways:
		if way.type == pbtypes.IGNORE:
			continue
		for i in range(len(way.refs)-1):
			edge = pbgraph.Edge()
			edge.osmid = way.id
			edge.vfrom = index[way.refs[i]]
			edge.vto = index[way.refs[i+1]]
			edge.type = way.type
			edge.crossing.extend(way.crossing)
	
	graph.edges.sort(key=lambda e:e.osmid)
	for i in range(len(graph.edges)):
		graph.edges[i].idx=i

amap = Map()

start = time.time()

amap.loadFromPB("praha-union.pbf")

end = time.time()
print "Loading took "+str(end-start)
start = time.time()

graph = Graph()
makeVertices(amap,graph)
index = makeVerticesIndex(graph)
makeEdges(amap,graph,index)

end = time.time()
print "Processing took "+str(end-start)
start = time.time()

with open("praha-graph.pbf","w") as f:
	f.write(graph.toPB().SerializeToString())

end = time.time()
print "Saving took "+str(end-start)
