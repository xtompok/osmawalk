#include <ucw/lib.h>
#include <ucw/fastbuf.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <inttypes.h>

#include <ucw/gary.h>
#include <ucw/heap.h>
#include "include/geodesic.h"

#include "include/graph.pb-c.h"

#define SWEEP_TYPE int64_t
#include "types.h"
#include "raster.h"
//#include "tree.h"
//#include "hashes.c"
#include "utils.h"

int scale = 1000000;

struct nodeways_t{
	int n_ways;
	int * ways;
};

struct nodeways_t * makeNodeWays(Graph__Graph * graph){
	struct nodeways_t * nodeways;
	nodeways = malloc(sizeof(struct nodeways_t)*graph->n_vertices);
	for (int i=0;i<graph->n_vertices;i++){
		nodeways[i].n_ways=0;
	}
	for (int i=0;i<graph->n_edges;i++){
		nodeways[graph->edges[i]->vfrom].n_ways++;
		nodeways[graph->edges[i]->vto].n_ways++;
	}
	for (int i=0;i<graph->n_vertices;i++){
		nodeways[i].ways = malloc(sizeof(int)*nodeways[i].n_ways);
		nodeways[i].n_ways = 0;
	}
	for (int i=0;i<graph->n_edges;i++){
		int vfIdx;
		int vtIdx;
		vfIdx = graph->edges[i]->vfrom;
		vtIdx = graph->edges[i]->vto;
		nodeways[vfIdx].ways[nodeways[vfIdx].n_ways]= i;
		nodeways[vfIdx].n_ways++;
		nodeways[vtIdx].ways[nodeways[vtIdx].n_ways]= i;
		nodeways[vtIdx].n_ways++;
	}
	return nodeways;
}

double calcTime(Graph__Edge * edge){
	return 1;
}


struct dijnode_t {
	int fromIdx;
	int fromEdgeIdx;
	bool reached;
	bool completed;
	double dist;
};

struct dijnode_t *  prepareDijkstra(Graph__Graph * graph){
	struct dijnode_t * dijArray;
	dijArray = malloc(sizeof(struct dijnode_t)*graph->n_vertices);
	for (int i=0;i<graph->n_vertices;i++){
		dijArray[i].fromIdx=-1;
		dijArray[i].fromEdgeIdx=-1;
		dijArray[i].reached=false;
		dijArray[i].completed=false;
		dijArray[i].dist = DBL_MAX;
	}
	return dijArray;
}


void findWay(Graph__Graph * graph, struct nodeways_t * nodeways,struct dijnode_t * dijArray,int fromIdx, int toIdx){
	dijArray[fromIdx].reached=true;
	dijArray[fromIdx].dist=0;

	int * heap;
	int n_heap;
	n_heap = graph->n_vertices;
	heap = malloc(sizeof(int)*n_heap);
	for (int i=0;i<n_heap;i++){
		heap[i]=i;
	}
	#define DIJ_CMP(x,y) (dijArray[x].dist<dijArray[y].dist)

	HEAP_INIT(int,heap,n_heap,DIJ_CMP,HEAP_SWAP);

	while(!dijArray[toIdx].completed){
		printf("Heap has %d vertices\n", n_heap);
		HEAP_DELETE_MIN(int,heap,n_heap,DIJ_CMP,HEAP_SWAP);
		int vIdx;
		vIdx = heap[n_heap+1];
		for (int i=0;i<nodeways[vIdx].n_ways;i++){
			Graph__Edge * way;
			way = graph->edges[nodeways[vIdx].ways[i]];
			double len;
			len = calcTime(way);
			if (dijArray[way->vto].dist <= (dijArray[vIdx].dist+len))
				continue;
			dijArray[way->vto].dist = dijArray[vIdx].dist+len;
			dijArray[way->vto].fromIdx = vIdx;
			dijArray[way->vto].fromEdgeIdx = nodeways[vIdx].ways[i];
			dijArray[way->vto].reached = true;
		
		}
		dijArray[vIdx].completed=true;
		
	}


}



	

int main (int argc, char ** argv){
	
	FILE * IN;
	IN = fopen("../data/praha-graph.pbf","r");
	if (IN==NULL){
		printf("File opening error\n");	
		return 1;
	}
	fseek(IN,0,SEEK_END);
	long int len;
	len = ftell(IN);
	fseek(IN,0,SEEK_SET);
	uint8_t * buf;
	buf = (uint8_t *)malloc(len);
	printf("Allocated %d MB\n",len>>20);
	size_t read;
	read = fread(buf,1,len,IN);
	if (read!=len){
		printf("Not read all file");
		return 2;
	}
	fclose(IN);
	
	Graph__Graph * graph;
	graph = graph__graph__unpack(NULL,len,buf);
	if (graph==NULL){
		printf("Error unpacking protocolbuffer\n");	
		return 2;
	}
/*

	initMap(pbmap);
	printf("Loaded %d nodes, %d ways\n",map.n_nodes,map.n_ways);
	nodesIdx_refresh(map.n_nodes,map.nodes);
	waysIdx_refresh(map.n_ways,map.ways);
	nodeWays_refresh(map);
	struct graph_t graph;
	graph = makeGraph(map);
	struct raster_t raster;
	raster = makeRaster(map);
	int ** candidates;
	candidates = makeDirectCandidates(map,raster,graph.wayGraph,20);
	struct line_t * lines;
	lines = findDirectWays(map,candidates,graph.barGraph,raster.minlon,raster.minlat);
	map = addDirectToMap(lines,map);
//	struct walk_area_t * walkareas;
//	walkareas = findWalkAreas(map,raster);
//	map = removeBarriers(map);
//	printf("Found %d walk areas\n",GARY_SIZE(walkareas));

*/	
	
	return 0;
}
