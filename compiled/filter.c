#include <ucw/lib.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include <ucw/gary.h>
#include <ucw/heap.h>
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

void mapToPBF(struct map_t map, Premap__Map * pbmap){
	pbmap->n_nodes = map.n_nodes;
	pbmap->nodes = map.nodes;
	pbmap->n_ways = map.n_ways;
	pbmap->ways = map.ways;
	pbmap->n_relations = map.n_relations;
	pbmap->relations = map.relations;
	pbmap->n_multipols = map.n_multipols;
	pbmap->multipols = map.multipols;
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


enum evt_type  {EVT_START,EVT_END,EVT_INTERSECT};
struct event_t {
	enum evt_type type;
	int64_t lon;
	int64_t lat;
	int64_t dlon;
	int64_t dlat;	
	unsigned int lineIdx;
};


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
	int *** raster;
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
					(* ptr)[0]=nodesIdx_find(way->refs[j])->idx;
					(* ptr)[1]=nodesIdx_find(way->refs[j+1])->idx;
				}
		}
	}

	struct graph_t graph;
	graph.barGraph = barGraph;
	graph.wayGraph = wayGraph;
	return graph;
}

int ** makeDirectCandidates(struct map_t map, struct raster_t raster, int ** wayGraph, int maxdist){

	int ** candidates;

	GARY_INIT_SPACE(candidates,100000);

	for (int lonidx=0;lonidx<raster.lonparts-1;lonidx++){
		for (int latidx=0;latidx<raster.latparts-1;latidx++){
			int count;
			count = GARY_SIZE(raster.raster[lonidx][latidx]);
			//printf("raster[%"PRId64"][%"PRId64"]=%d\n",lonidx,latidx,count);
			for (int i=0; i<count; i++) {
				int nidx1;
				nidx1 = raster.raster[lonidx][latidx][i];
				if (!wayGraph[nidx1]){
					continue;
				}

				for (int j=0;j<count;j++){
					int nidx2;
					nidx2 = raster.raster[lonidx][latidx][j];
					if ((nidx1 < nidx2)&&wayGraph[nidx2]){
						int ** ptr;
						ptr = GARY_PUSH(candidates);
						* ptr = malloc(2*sizeof(int));
						(* ptr)[0] = nidx1;
						(* ptr)[1] = nidx2;
					}
				}
				int neighCount;
				neighCount=GARY_SIZE(raster.raster[lonidx+1][latidx]);
				for (int j=0;j<neighCount;j++){
					int nidx2;
					nidx2 = raster.raster[lonidx+1][latidx][j];
					if (wayGraph[nidx2]&&(distance(map.nodes[nidx1],map.nodes[nidx2])<=maxdist)){
						int ** ptr;
						ptr = GARY_PUSH(candidates);
						* ptr = malloc(2*sizeof(int));
						(* ptr)[0] = nidx1; 
						(* ptr)[1] = nidx2; 
						
					}
				}
				neighCount=GARY_SIZE(raster.raster[lonidx+1][latidx+1]);
				for (int j=0;j<neighCount;j++){
					int nidx2;
					nidx2 = raster.raster[lonidx+1][latidx+1][j];
					if (wayGraph[nidx2]&&(distance(map.nodes[nidx1],map.nodes[nidx2])<=maxdist)){
						int ** ptr;
						ptr = GARY_PUSH(candidates);
						* ptr = malloc(2*sizeof(int));
						(* ptr)[0] = nidx1; 
						(* ptr)[1] = nidx2; 
						
					}
				}
				neighCount=GARY_SIZE(raster.raster[lonidx][latidx+1]);
				for (int j=0;j<neighCount;j++){
					int nidx2;
					nidx2 = raster.raster[lonidx][latidx+1][j];
					if (wayGraph[nidx2]&&(distance(map.nodes[nidx1],map.nodes[nidx2])<=maxdist)){
						int ** ptr;
						ptr = GARY_PUSH(candidates);
						* ptr = malloc(2*sizeof(int));
						(* ptr)[0] = nidx1; 
						(* ptr)[1] = nidx2; 
						
					}
				}
			}

		}

	}
	int candCount;
	candCount = GARY_SIZE(candidates);
	printf("Candidates: %d\n",candCount);
	return candidates;
		
}

void findDirectWays(struct map_t map, int ** candidates, int ** barGraph){
	int n_cand;
	n_cand = GARY_SIZE(candidates);
	struct line_t * lines;
	GARY_INIT_SPACE(lines,n_cand);
	for (int i=0;i<n_cand;i++){
		Premap__Node * n1;
		Premap__Node * n2;
		n1 = map.nodes[candidates[i][0]];
		n2 = map.nodes[candidates[i][1]];
		
		struct line_t * line;
		line = GARY_PUSH(lines);

		if (n2->lon < n1->lon){
			line->startlon = n2->lon;
			line->startlat = n2->lat;
			line->endlon = n1->lon;
			line->endlat = n1->lat;
			line->startid = n2->id;
			line->endid = n1->id;
		} else {
			line->startlon = n1->lon;
			line->startlat = n1->lat;
			line->endlon = n2->lon;
			line->endlat = n2->lat;
			line->startid = n1->id;
			line->endid = n2->id;
		}
		line->isBar = 0;
		line->broken = 0;
	}

	int n_bars;
	n_bars = GARY_SIZE(barGraph);

	for (int i=0;i<n_bars;i++){
		Premap__Node * n1;
		Premap__Node * n2;
		n1 = map.nodes[barGraph[i][0]];
		n2 = map.nodes[barGraph[i][1]];

		struct line_t * line;
		line = GARY_PUSH(lines);

		if (n2->lon < n1->lon){
			line->startlon = n2->lon;
			line->startlat = n2->lat;
			line->endlon = n1->lon;
			line->endlat = n1->lat;
			line->startid = n2->id;
			line->endid = n1->id;
		} else {
			line->startlon = n1->lon;
			line->startlat = n1->lat;
			line->endlon = n2->lon;
			line->endlat = n2->lat;
			line->startid = n1->id;
			line->endid = n2->id;
		}
		line->isBar = 1;
		line->broken = 0;
	}

	// Sort
	#define ASORT_PREFIX(X) lines_##X
	#define ASORT_KEY_TYPE struct line_t
	#define ASORT_LT(x,y) (((x).startid<(y).startid)||((x).endid<(y).endid)||((x).isBar<(y).isBar))
	#include <ucw/sorter/array-simple.h>
	// Remove duplicities
	// Make queue
	int n_lines;
	n_lines = GARY_SIZE(lines);
	struct event_t * queue;
	GARY_INIT_SPACE(queue,n_lines);
	for (int i=0;i<n_lines;i++){
		struct event_t * event;
		event = GARY_PUSH(queue);
		event->type = EVT_START;
		event->lon = lines[i].startlon;
		event->lat = lines[i].startlat;
		event->dlon = lines[i].endlon-lines[i].startlon;
		event->dlat = lines[i].endlat-lines[i].startlat;
		event->lineIdx = i;

		event = GARY_PUSH(queue);
		event->type = EVT_END;
		event->lon = lines[i].endlon;
		event->lat = lines[i].endlat;
		event->dlon = lines[i].startlon-lines[i].endlon;
		event->dlat = lines[i].startlat-lines[i].endlat;
		event->lineIdx = i;
	}
	
	#define EVENT_CMP(x,y) (((x).lon<(y).lon)||((x).lat<(y).lat)||(((x).dlon*(y).dlat)<((x).dlat*(y).dlon)))


	// Do sweeping

} 


struct map_t addCandidatesToMap(int ** candidates, struct map_t map){
	for (int i=0;i<GARY_SIZE(candidates);i++){
		Premap__Way * way;
		way = malloc(sizeof(Premap__Way));
		premap__way__init(way);
		way->id = -2000-i;
		way->n_refs=2;
		way->refs = malloc(sizeof(way->refs[0])*2);
		way->refs[0] = map.nodes[candidates[i][0]]->id;
		way->refs[1] = map.nodes[candidates[i][1]]->id;
		way->type = OBJTYPE__DIRECT;
		Premap__Way ** ptr;
		ptr = GARY_PUSH(map.ways);
		*ptr = way;
		map.n_ways++;
	}

	return map;
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
	size_t read;
	read = fread(buf,1,len,IN);
	if (read!=len){
		printf("Not read all file");
		return 2;
	}
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
	waysIdx_find(0);
	struct graph_t graph;
	graph = makeGraph(map);
	struct raster_t raster;
	raster = makeRaster(map);
	int ** candidates;
	candidates = makeDirectCandidates(map,raster,graph.wayGraph,20);
	findDirectWays(map,candidates,graph.barGraph);
	map = addCandidatesToMap(candidates,map);
	mapToPBF(map,pbmap);
	len = premap__map__get_packed_size(pbmap);
	buf = malloc(len);
	premap__map__pack(pbmap,buf);
	FILE * OUT;
	OUT = fopen("../scripts/filter/praha-union-c.pbf","w");
	if (IN==NULL){
		printf("File opening error\n");	
		return 1;
	}
	fwrite(buf,1,len,OUT);
	fclose(OUT);

	
	
	return 0;
}
