#include <ucw/lib.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <ucw/gary.h>
#include "include/premap.pb-c.h"
#include "include/geodesic.h"

//#define ARRAY_TYPE int
//#define ARRAY_PREFIX(name) intarray_##name
//#include <array.h>

int scale = 1000000;

struct nodesIdxNode {
	int64_t key;
	int idx;
};
struct waysIdxNode {
	int64_t key;
	int idx;
};

#define HASH_NODE struct nodesIdxNode
#define HASH_PREFIX(name) nodesIdx_##name
#define HASH_KEY_ATOMIC key
#define HASH_ATOMIC_TYPE int64_t
#define HASH_DEFAULT_SIZE 100000
#define HASH_WANT_CLEANUP
#define HASH_WANT_FIND
#define HASH_WANT_NEW
#include <ucw/hashtable.h>

#define HASH_NODE struct waysIdxNode
#define HASH_PREFIX(name) waysIdx_##name
#define HASH_KEY_ATOMIC key
#define HASH_ATOMIC_TYPE int64_t
#define HASH_DEFAULT_SIZE 100000
#define HASH_WANT_CLEANUP
#define HASH_WANT_FIND
#define HASH_WANT_NEW
#include <ucw/hashtable.h>


void nodesIdx_refresh(int n_nodes, Premap__Node ** nodes){
	nodesIdx_cleanup();
	nodesIdx_init();
	for (int i=0;i<n_nodes;i++){
		struct nodesIdxNode * val;
		val = nodesIdx_new(nodes[i]->id);
		val->idx = i;
	}
}

void waysIdx_refresh(int n_ways, Premap__Way ** ways){
	waysIdx_cleanup();
	waysIdx_init();
	for (int i=0;i<n_ways;i++){
		struct waysIdxNode * val;
		val = waysIdx_new(ways[i]->id);
		val->idx = i;
	}
}

struct map_t {
	int n_nodes;
	Premap__Node ** nodes;
	int n_ways;
	Premap__Way ** ways;
	int n_relations;
	Premap__Relation ** relations;
	int n_multipols;
	Premap__Multipolygon ** multipols;
};

struct map_t map;
void initMap(Premap__Map *pbmap){
	map.n_nodes = pbmap->n_nodes;
	GARY_INIT(map.nodes,map.n_nodes);
	for(int i=0;i<map.n_nodes;i++){
		map.nodes[i]=pbmap->nodes[i];
	}
	map.n_ways = pbmap->n_ways;
	GARY_INIT(map.ways,map.n_ways);
	for(int i=0;i<map.n_ways;i++){
		map.ways[i]=pbmap->ways[i];
	}
	map.n_relations = pbmap->n_relations;
	GARY_INIT(map.relations,map.n_relations);
	for(int i=0;i<map.n_relations;i++){
		map.relations[i]=pbmap->relations[i];
	}
	map.n_multipols = pbmap->n_multipols;
	GARY_INIT(map.multipols,map.n_multipols);
	for(int i=0;i<map.n_multipols;i++){
		map.multipols[i]=pbmap->multipols[i];
	}
}

struct line_t {
	int64_t startlon;
	int64_t startlat;
	int64_t endlon;
	int64_t endlat;
	int64_t startid;
	int64_t endid;
	bool isBar;
	bool broken;
};
int calcLatForLon(struct line_t line,int64_t lon){
	if (line.endlon < lon)
		printf("Line already ended!");
	int64_t minlat = MIN(line.startlat,line.endlat);
	int64_t maxlat = MAX(line.startlat,line.endlat);

	return ((lon-line.startlon)/(line.endlon-line.startlon))*(maxlat-minlat)+minlat;
}

int calcCollision(struct line_t line1, struct line_t line2){
	int64_t lon1 = line1.startlon;
	int64_t lon2 = line1.endlon;
	int64_t lat1 = line1.startlat;
	int64_t lat2 = line1.endlat;
	int64_t lon3 = line2.startlon;
	int64_t lon4 = line2.endlon;
	int64_t lat3 = line2.startlat;
	int64_t lat4 = line2.endlat;
	int64_t cit = (lat2-lat1)*(lon4-lon3)*lon1+(lon2-lon1)*(lon4-lon3)*(lat3-lat1)-(lat4-lat3)*(lon2-lon1)*lon3;
	int64_t jm = (lat2-lat1)*(lon4-lon3)-(lon2-lon1)*(lat4-lat3);
	if (jm == 0)
		return -1;
	int64_t lon = cit/jm;
	int64_t lat = (lon*(lat2-lat1)+lon1*(lon2-lon1)-lon1*(lat2-lat1))/(lon2-lon1);
	if (lon1 < lon && lon < lon2 &&
		lon3 < lon && lon < lon4 &&
		lat1 < lat && lat < lat2 &&
		lat3 < lat && lat < lat4)
		//return (lon,lat)
	return -1;
	
}


enum evt_type  {START,END,INTERSECT};
struct event_t {
	enum evt_type type;
	int64_t lon;
	int64_t lat;
	int64_t dlon;
	int64_t dlat;	
};

#define CMP_EVENT(x,y) (((x).lon<(y).lon)||((x).lat<(y).lat)||(((x).dlon*(y).dlat)<((x).dlat*(y).dlon)))

inline double int2deg(int intdeg){
	return 1.0*intdeg/scale;		
}

inline int deg2int(double deg){
	return (int)(deg*scale);	
}

inline int distance(Premap__Node * node1, Premap__Node * node2){
	double g_a = 6378137, g_f = 1/298.257223563; /* WGS84 */
	struct geod_geodesic geod;
	geod_init(&geod, g_a, g_f);
	double dist;
	double tmp1;
	double tmp2;
	geod_inverse(&geod,int2deg(node1->lat),int2deg(node1->lon),int2deg(node2->lat),int2deg(node2->lon),&dist,&tmp1,&tmp2);
	return (int)dist;
}

struct raster_t {
	int64_t minlon;
	int64_t minlat;
	int64_t lonparts;
	int64_t latparts;
	int64_t steplon;
	int64_t steplat;
	int64_t *** raster;
};


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
		* ptr = node->id;
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

struct graph_t {
	int ** barGraph;
	int ** wayGraph;
};
struct graph_t makeGraph(struct map_t map)
{
	int ** barGraph;
	int ** wayGraph;
	GARY_INIT_SPACE_ZERO(barGraph,map.n_ways);
	GARY_INIT_ZERO(wayGraph,map.n_nodes);

	for (int i=0;i < map.n_ways;i++){
		Premap__Way * way;
		way = map.ways[i];
		if (map.ways[i]->type<25){ // ugly
			for (int j=0;j<way->n_refs-1;j++){
				int n1idx = nodesIdx_find(way->refs[j])->idx;
				int n2idx = nodesIdx_find(way->refs[j+1])->idx;
				if (wayGraph[n1idx] == NULL){
					GARY_INIT(wayGraph[n1idx],1);
					wayGraph[n1idx][0]=n2idx;
				} else {
					int * ptr = GARY_PUSH(wayGraph[n1idx]);
					* ptr = n2idx;
				}
				if (wayGraph[n2idx] == NULL){
					GARY_INIT(wayGraph[n2idx],1);
					wayGraph[n1idx][0]=n1idx;
				} else {
					int * ptr = GARY_PUSH(wayGraph[n2idx]);
					* ptr = n1idx;
				}
			}
		}
		else if ((map.ways[i]->type>=30)&&(map.ways[i]->type<40)){ //ugly
			for (int j=0;j<way->n_refs-1;j++){
					int ** ptr = GARY_PUSH(barGraph);
					* ptr = malloc(sizeof(int)*2);
					* ptr[0]=nodesIdx_find(way->refs[j])->idx;
					* ptr[1]=nodesIdx_find(way->refs[j+1])->idx;
				}
		}
	}

	struct graph_t graph;
	graph.barGraph = barGraph;
	graph.wayGraph = wayGraph;
	return graph;
}

int * makeDirectCandidates(struct map_t map, struct raster_t raster, int ** wayGraph, int maxdist){

	int * candidates;

	GARY_INIT(candidates,128);
		
}
	

int main (int argc, char ** argv){
	
	FILE * IN;
	IN = fopen("../scripts/filter/praha-union.pbf","r");
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
	printf("Allocated %d memory\n",len);
	fread(buf,1,len,IN);
	fclose(IN);
	
	Premap__Map *pbmap;
	pbmap = premap__map__unpack(NULL,len,buf);
	if (pbmap==NULL){
		printf("Error unpacking protocolbuffer\n");	
		return 2;
	}
	initMap(pbmap);
	printf("Loaded %d nodes, %d ways\n",map.n_nodes,map.n_ways);
	nodesIdx_refresh(map.n_nodes,map.nodes);
	waysIdx_refresh(map.n_ways,map.ways);
	makeGraph(map);
	makeRaster(map);
	printf("%d",map.nodes[nodesIdx_find(1132352548)->idx]->id);
	
	return 0;
}
