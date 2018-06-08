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

char dataFileName[] = "../data/praha-graph.pbf";
char configFileName[] = "../config/speeds.yaml";
char timetableFileName[] = "../ext-lib/mmpf/raptor/tt.bin";

// Print results of searching way
void printResults(struct search_route_t result){
	if (result.n_points==0){
		printf("Route not found\n");
		return;
	}
	printf("Lenght: %1fm, Time: %1fmin\n",result.dist/1000, result.time/60);
	for (int i=0;i<result.n_points;i++){
		struct point_t p;
		p = result.points[i];
		printf("%f,%f,%d,%d,%lld\n",p.lat,p.lon,p.height,-1,-1);
	}
}

// Print coordinates of vertices (for debug)
void printVertices(Graph__Graph * graph){
	for (int i=0;i<graph->n_vertices;i++){
		printf("%f %f\n",graph->vertices[i]->lon,graph->vertices[i]->lat);
	}
}


// Print usage
void usage(void){
	printf("Usage: ./search fromlat fromlon tolat tolon\n");		
}

int main (int argc, char ** argv){
	printf("Argc: %d\n",argc);
	if (argc<5 && argc >2 ){
		usage();
		return 1;
	}
	struct search_data_t * data;
	double flon;
	double flat;
	double tlon;
	double tlat;
	uint64_t atime;

	atime = 0;

	if (argc==8){
		data = prepareData(argv[1],argv[2],argv[3]);
		flat = atof(argv[3]);
		flon = atof(argv[4]);
		tlat = atof(argv[5]);
		tlon = atof(argv[6]);
	}else if (argc==5){
		data = prepareData(configFileName,dataFileName,timetableFileName); 
		flat = atof(argv[1]);
		flon = atof(argv[2]);
		tlat = atof(argv[3]);
		tlon = atof(argv[4]);
		struct pbf_data_t result;
		result = findPath(data,flat,flon,tlat,tlon,0);
	}else if (argc==6){
		data = prepareData(configFileName,dataFileName,timetableFileName); 
		flat = atof(argv[1]);
		flon = atof(argv[2]);
		tlat = atof(argv[3]);
		tlon = atof(argv[4]);
		atime = atoll(argv[5]); 
	}else if (argc==2){
		data = prepareData(configFileName,dataFileName,timetableFileName); 
		FILE * infile;
		infile = fopen(argv[1],"r");
		while (1) {
			if (fscanf(infile,"%lf",&flat) != 1){
				break;	
			}
			if (fscanf(infile,"%lf",&flon) != 1){
				break;	
			}
			if (fscanf(infile,"%lld",&atime) != 1){
				break;	
			}
			struct search_result_t result;
			printf("Searching from %f %f\n",flat,flon);
			result = findPathToMetro(data,flat,flon,atime);
			for (int i=0;i<result.n_routes;i++){
				free(result.routes[i].points);	
			}
			free(result.routes);

		}
		freeData(data);
		return 0;
	}else{
		data = prepareData(configFileName,dataFileName,timetableFileName); 
		for (double flon=14; flon < 15; flon += 0.5){
		for (double tlon=14; tlon < 15; tlon += 0.5){
		for (double flat=49; flat < 51; flat += 0.5){
		for (double tlat=49; tlat < 51; tlat += 0.5){
		struct pbf_data_t result;

		result = findPath(data,flat,flon,tlat,tlon,0);

		freePackedPBF(result);
		}}}}
		freeData(data);
		return 0;
	}

	printf("Graph has %d vertices and %d edges\n",data->graph->n_vertices,data->graph->n_edges);
	printMapBBox(*data);

	struct search_result_t result;
	result = findPathToMetro(data,flat,flon,atime);

	//freePackedPBF(result);
	freeData(data);

	if (result.n_routes == 0){
		printf("Route not found\n");
		return 0;
	} 

	//writeGpxFile(result,"track.gpx");

	return 0;
}
