//#include <ucw/lib.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <inttypes.h>

#include <yaml.h>
#include <proj_api.h>
#include <ucw/heap.h>

#include "../compiled/include/graph.pb-c.h"
#include "../compiled/searchlib.h"
#include "../compiled/writegpx.h"

// Print results of searching way
void printResults(struct search_result_t result){
	if (result.n_points<2){
		printf("0 0\n");
		return;
	}
	printf("%lf %lf\n",result.dist, result.time);
	writeGpxStartTrack(stdout);
	for (int i=0;i<result.n_points;i++){
		struct point_t p;
		p = result.points[i];
		writeGpxTrkpt(stdout,p.lat,p.lon,p.height);
	}
	writeGpxEndTrack(stdout);
}

// Print usage
void usage(void){
	printf("Usage: ./search fromlat fromlon tolat tolon\n");		
}

int main (int argc, char ** argv){
	struct search_data_t data;
	double flon;
	double flat;
	double tlon;
	double tlat;

	data = prepareData("../config/speeds.yaml","../data/praha-graph.pbf"); 
	fprintf(stderr,"Loaded\n");
	while (1){
		scanf("%lf",&flat);
		scanf("%lf",&flon);
		scanf("%lf",&tlat);
		scanf("%lf",&tlon);

		fprintf(stderr,"Searching from %f, %f to %f, %f\n",flat,flon,tlat,tlon);

		struct search_result_t result;
		result = findPath(data,flat,flon,tlat,tlon);
		printResults(result);
		printf("\n");
		fflush(stdout);
	}

	return 0;
}
