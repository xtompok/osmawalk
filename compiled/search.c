//#include <ucw/lib.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <inttypes.h>

#include <yaml.h>
#include <proj_api.h>
#include <ucw/heap.h>

#include "include/graph.pb-c.h"
#include "searchlib.h"
#include "writegpx.h"


void printResults(struct search_result_t result){
	if (result.n_points==0){
		printf("Route not found\n");
		return;
	}
	printf("Lenght: %1fm, Time: %1fmin\n",result.dist/1000, result.time/60);
	for (int i=0;i<result.n_points;i++){
		struct point_t p;
		p = result.points[i];
		printf("%f,%f,%d,%d\n",p.lat,p.lon,p.height,p.type);
	}
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

	printf("Graph has %d vertices and %d edges\n",data.graph->n_vertices,data.graph->n_edges);

	struct search_result_t result;
	result = findPath(data,atof(argv[1]),atof(argv[2]),atof(argv[3]),atof(argv[4]));
	printResults(result);
	writeGpxFile(result,"track.gpx");

	return 0;
}
