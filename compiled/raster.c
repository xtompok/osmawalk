#include <ucw/lib.h>
#include <stdlib.h>
#include <ucw/gary.h>

#include "types.h"
#include "raster.h"
#include "hashes.c"
#include "utils.h"


struct raster_t makeRaster(struct map_t map){
	int dimension = 20;

	struct raster_t raster;
	
	int64_t maxlon;
	int64_t maxlat;

	raster.minlon=map.nodes[0]->lon;
	raster.minlat=map.nodes[0]->lat;
	maxlon=map.nodes[0]->lon;
	maxlat=map.nodes[0]->lat;
	
	for (int i=0;i<map.n_nodes;i++){
		raster.minlon = MIN(map.nodes[i]->lon,raster.minlon);	
		maxlon = MAX(map.nodes[i]->lon,maxlon);	
		raster.minlat = MIN(map.nodes[i]->lat,raster.minlat);
		maxlat = MAX(map.nodes[i]->lat,maxlat);
	}

	int64_t dlon = maxlon-raster.minlon;
	int64_t dlat = maxlat-raster.minlat;

	printf("Lon: %d -- %d, Lat: %d -- %d\n",raster.minlon,maxlon,raster.minlat,maxlat);

	double londist;
	double latdist;
	
	londist = int2deg(maxlon-raster.minlon);
	latdist = int2deg(maxlat-raster.minlat);
	
	raster.lonparts = (int)(londist/dimension)+10;
	raster.latparts = (int)(latdist/dimension)+10;
	
	printf("Creating raster %d x %d\n",raster.lonparts,raster.latparts);

	raster.steplon = (dlon+raster.lonparts-1)/raster.lonparts;
	raster.steplat = (dlat+raster.latparts-1)/raster.latparts;

	GARY_INIT(raster.raster,raster.lonparts);
	for (int i=0;i<raster.lonparts;i++){
		GARY_INIT(raster.raster[i],raster.latparts);
		for (int j=0;j<raster.latparts;j++){
			GARY_INIT(raster.raster[i][j],0);	
		}
	}

	for (int i=0;i<map.n_nodes;i++){
		Premap__Node * node;
		node = map.nodes[i];
		int * ptr;
		ptr = GARY_PUSH(raster.raster[(node->lon-raster.minlon)/raster.steplon][(node->lat-raster.minlat)/raster.steplat]);
		* ptr = nodesIdx_find(node->id)->idx;
	}

	return raster;
}

int * getRasterBox(struct raster_t raster,int64_t lon, int64_t lat){
	int * box;
	box = malloc(sizeof(int)*2);
	box[0]=(lon-raster.minlon)/raster.steplon;
	box[1]=(lat-raster.minlat)/raster.steplat;	
	return box;
}
