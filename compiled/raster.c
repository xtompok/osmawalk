#include <ucw/lib.h>
#include <stdlib.h>
#include <ucw/gary.h>

#include "include/geodesic.h"
#include "types.h"
#include "raster.h"
#include "hashes.c"
#include "utils.h"


struct raster_t makeRaster(struct map_t map){
	int dimension = 20;

	double g_a = 6378137, g_f = 1/298.257223563; /* WGS84 */
//	double lat1, lon1, azi1, lat2, lon2, azi2, s12;
	struct geod_geodesic geod;
	geod_init(&geod, g_a, g_f);

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
	double tmp1;
	double tmp2;

	geod_inverse(&geod,int2deg(maxlat-dlat/2),int2deg(raster.minlon),int2deg(maxlat-dlat/2),int2deg(maxlon),&londist,&tmp1,&tmp2);
	geod_inverse(&geod,int2deg(raster.minlat),int2deg(maxlon-dlon/2),int2deg(maxlat),int2deg(maxlon-dlon/2),&latdist,&tmp1,&tmp2);
	
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
