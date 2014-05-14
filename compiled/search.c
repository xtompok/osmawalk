//#include <ucw/lib.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <inttypes.h>

#include <proj_api.h>
#include <ucw/heap.h>

#include "include/graph.pb-c.h"

#include <yaml.h>

#define SWEEP_TYPE int64_t
#include "searchlib.h"
#include "writegpx.h"


void printResults(Graph__Graph * graph, struct config_t conf, struct dijnode_t * dijArray, int fromIdx, int toIdx){
	if (!dijArray[toIdx].completed){
		printf("Route not found\n");
		return;
	}
	printf("D1: %f, D2:%f\n",dijArray[fromIdx].dist,dijArray[toIdx].dist);
	int idx;
	idx = fromIdx;
	while (dijArray[idx].fromIdx!=-1){
		printf("%f,%f,%d\n",graph->vertices[idx]->lat,graph->vertices[idx]->lon,graph->edges[dijArray[idx].fromEdgeIdx]->type);
		idx = dijArray[idx].fromIdx;
	}
	printf("%f,%f,\n",graph->vertices[idx]->lat, graph->vertices[idx]->lon);	
}


void printVertices(Graph__Graph * graph){
	for (int i=0;i<graph->n_vertices;i++){
		printf("%f %f\n",graph->vertices[i]->lon,graph->vertices[i]->lat);
	}
}

void usage(void){
	printf("Usage: ./search fromlat fromlon tolat tolon\n");		
}

int main (int argc, char ** argv){
	if (argc<5){
		usage();
		return 1;
	}
	struct search_data_t data;
	data = prepareData("../config/speeds.yaml","../data/praha-graph.pbf"); 
//	printVertices(graph);
//	return 0;

	printf("Graph has %d vertices and %d edges\n",data.graph->n_vertices,data.graph->n_edges);
	double lon;
	double lat;
	lat = atof(argv[1]);
	lon = atof(argv[2]);
	wgs2utm(data,&lon,&lat);
	int fromIdx;
	fromIdx = findNearestVertex(data.graph,lon,lat);

	lat = atof(argv[3]);
	lon = atof(argv[4]);
	wgs2utm(data,&lon,&lat);
	int toIdx;
	toIdx = findNearestVertex(data.graph,lon,lat);

	printf("Searching from %lld(%f,%f) to %lld(%f,%f)\n",data.graph->vertices[fromIdx]->osmid,
			data.graph->vertices[fromIdx]->lat,
			data.graph->vertices[fromIdx]->lon,
			data.graph->vertices[toIdx]->osmid,
			data.graph->vertices[toIdx]->lat,
			data.graph->vertices[toIdx]->lon
			);

	struct dijnode_t * dijArray;
	dijArray = prepareDijkstra(data.graph);

	findWay(data, dijArray,fromIdx, toIdx);
	printResults(data.graph, data.conf, dijArray, fromIdx, toIdx);
	writeGpxFile(data, dijArray,"track.gpx", fromIdx, toIdx);

	return 0;
}
