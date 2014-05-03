#include <ucw/lib.h>
#include <ucw/fastbuf.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include <ucw/gary.h>
#include <ucw/heap.h>
#include "include/geodesic.h"
#include "include/graph.pb-c.h"

#define PF2(tok) #tok
#define PF(tok) PF2(tok)

#define SCALE 1000000
#define SWEEP_SCALE 100
#define EPSILON 0
#define S_P d

#include "types.h"
#include "raster.h"
#include "hashes.c"
#include "utils.h"
#include "mixnum.h"

int scale = 1000000;

#define ASORT_PREFIX(X) int64_##X
#define ASORT_KEY_TYPE int64_t
#include <ucw/sorter/array-simple.h>

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

void nodeWays_refresh(struct map_t map){
	nodeWays_cleanup();
	nodeWays_init();
	for (int i=0;i<map.n_nodes;i++){
		struct nodeWaysNode * val;
		val = nodeWays_new(map.nodes[i]->id);
		GARY_INIT(val->ways,0);
	}
	for (int i=0;i<map.n_ways;i++){
		Premap__Way * way;
		way = map.ways[i];
		for (int j=0;j<way->n_refs;j++){
			struct nodeWaysNode * node;
			node = nodeWays_find(way->refs[j]);
			int64_t * wayid;
			wayid = GARY_PUSH(node->ways);
			* wayid = way->id;
		}
	}

	for (int i=0;i<map.n_nodes;i++){
		int64_t * ways;
		ways = nodeWays_find(map.nodes[i]->id)->ways;
		int64_sort(ways,GARY_SIZE(ways));
	}

}

struct map_t  initMap(Premap__Map *pbmap){
	struct map_t map;
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
	return map;
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



struct mixed_num_t  calcMixLatForLon(struct line_t line,SWEEP_TYPE lon){
	SWEEP_TYPE minlat = MIN(line.startlat,line.endlat);
	SWEEP_TYPE maxlat = MAX(line.startlat,line.endlat);	

	struct mixed_num_t lat;
	if (minlat==maxlat){
		lat = makeMix(minlat,0,1);
		return lat;
	}

	lat = makeMix(line.startlat,(line.endlat-line.startlat)*(lon-line.startlon),line.endlon-line.startlon);
	return lat;
}

	

struct col_t calcMixCollision(struct line_t line1, struct line_t line2){
	struct col_t result;
	result.isCol=1;
	if (sameStart(line1,line2)){
		result.atEndpoint = 1;
		result.lon = makeMix(line1.startlon,0,1);
		result.lat = makeMix(line1.startlat,0,1);
		return result;
	}
	if (sameEnd(line1,line2)){
		result.atEndpoint = 1;
		result.lon = makeMix(line1.endlon,0,1);
		result.lat = makeMix(line1.endlat,0,1);
		return result;
	}

	result.atEndpoint = 0;

	SWEEP_TYPE lon1 = line1.startlon;
	SWEEP_TYPE lon2 = line1.endlon;
	SWEEP_TYPE lat1 = line1.startlat;
	SWEEP_TYPE lat2 = line1.endlat;
	SWEEP_TYPE lon3 = line2.startlon;
	SWEEP_TYPE lon4 = line2.endlon;
	SWEEP_TYPE lat3 = line2.startlat;
	SWEEP_TYPE lat4 = line2.endlat;

	// Check bboxes
	if ((MAX(lon1,lon2)<MIN(lon3,lon4))||(MAX(lon3,lon4)<MIN(lon1,lon2))){
		result.isCol=0;	
		return result;
	}
	if ((MAX(lat1,lat2)<MIN(lat3,lat4))||(MAX(lat3,lat4)<MIN(lat1,lat2))){
		result.isCol=0;
		return result;
	}
	

	SWEEP_TYPE minlon = MIN(lon1,lon3);
	SWEEP_TYPE minlat = MIN(MIN(lat1,lat2),MIN(lat3,lat4));

	lon1-=minlon;
	lon2-=minlon;
	lon3-=minlon;
	lon4-=minlon;
	lat1-=minlat;
	lat2-=minlat;
	lat3-=minlat;
	lat4-=minlat;

	SWEEP_TYPE cit = (lon1*lat2-lat1*lon2)*(lon3-lon4)-(lon3*lat4-lat3*lon4)*(lon1-lon2);
	SWEEP_TYPE jm = (lon1-lon2)*(lat3-lat4)-(lat1-lat2)*(lon3-lon4);	

	// denominator is zero
	if (jm == 0){
		printf("denom is null\n");
		result.isCol=0;
		return result;
	}

	//printf("minlon: %lld cit: %lld, jm: %lld, res: %lld, %lld %lld %lld %lld \n",minlon,cit,jm,cit/jm,lon1,lon2,lon3,lon4);
	// longitude of collision is not on both lines
	result.lon = makeMix(minlon,cit,jm);
	int64_t maxmin;
	int64_t minmax;
	maxmin = MAX(MIN(lon1,lon2),MIN(lon3,lon4)) + minlon;
	minmax = MIN(MAX(lon1,lon2),MAX(lon3,lon4)) + minlon;
	printf("base: %lld, maxmin: %lld, minmax: %lld\n",result.lon.base,maxmin,minmax);
	if ( (result.lon.base < maxmin) || 
		(result.lon.base > minmax)){
		printf("lon out of range\n");
		result.isCol=0;
		return result;
		}

	cit = (lon1*lat2-lat1*lon2)*(lat3-lat4)-(lon3*lat4-lat3*lon4)*(lat1-lat2);
	// latitude of collision is not on both lines
	result.lat = makeMix(minlat,cit,jm);
	maxmin = MAX(MIN(lat1,lat2),MIN(lat3,lat4)) + minlat;
	minmax = MIN(MAX(lat1,lat2),MAX(lat3,lat4)) + minlat;
	printf("base: %lld, maxmin: %lld, minmax: %lld\n",result.lat.base,maxmin,minmax);
	if ( (result.lat.base < maxmin) || 
		(result.lat.base > minmax)){
		printf("lat out of range\n");
		result.isCol=0;
		return result;
		}

	return result;
}




static inline int distance(Premap__Node * node1, Premap__Node * node2){
	double g_a = 6378137, g_f = 1/298.257223563; /* WGS84 */
	struct geod_geodesic geod;
	geod_init(&geod, g_a, g_f);
	double dist;
	double tmp1;
	double tmp2;
	geod_inverse(&geod,int2deg(node1->lat),int2deg(node1->lon),int2deg(node2->lat),int2deg(node2->lon),&dist,&tmp1,&tmp2);
	return (int)dist;
}

void addCandidate(int *** candidates,int nidx1,int nidx2){
	int ** ptr;
	ptr = GARY_PUSH(*candidates);
	* ptr = malloc(2*sizeof(int));
	(* ptr)[0] = nidx1;
	(* ptr)[1] = nidx2;
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
		if (isDirectable(map.ways[i])){ 
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
		else if (isBarrier(map.ways[i])){ 
			for (int j=0;j<way->n_refs-1;j++){
				addCandidate(&barGraph,nodesIdx_find(way->refs[j])->idx,nodesIdx_find(way->refs[j+1])->idx);	
			}
		}
	}

	struct graph_t graph;
	graph.barGraph = barGraph;
	graph.wayGraph = wayGraph;
	return graph;
}

int64_t onSameWay(int64_t nid1,int64_t nid2){
	int64_t * ways1;
	int64_t * ways2;
	ways1 = nodeWays_find(nid1)->ways;
	ways2 = nodeWays_find(nid2)->ways;
	int n_ways1 = GARY_SIZE(ways1);
	int n_ways2 = GARY_SIZE(ways2);
	int i1;
	int i2;
	i1 = 0;
	i2 = 0;
	while (i1 < n_ways1 && i2 < n_ways2){
		if (ways1[i1]==ways2[i2])
			return ways1[i1];
		if (ways1[i1]<ways2[i2]){
			i1++;
			continue;
		}
		i2++;
		continue;
	}
	return 0;
}
int ** makeAreaBarGraph(struct map_t map,struct walk_area_t area){
	Premap__Way * way;
	int ** barGraph;
	GARY_INIT(barGraph,0);
	for (int i=0;i<area.n_bars;i++){
		way = map.ways[area.barIdxs[i]];
		for (int j=0;j<way->n_refs-1;j++){
			addCandidate(&barGraph,nodesIdx_find(way->refs[j])->idx,nodesIdx_find(way->refs[j+1])->idx);
		}
	}
	for (int i=0;i<area.n_ways;i++){
		way = map.ways[area.wayIdxs[i]];
		for (int j=0;j<way->n_refs-1;j++){
			addCandidate(&barGraph,nodesIdx_find(way->refs[j])->idx,nodesIdx_find(way->refs[j+1])->idx);
		}
	}
	way = map.ways[area.periIdx];
	for (int j=0;j<way->n_refs-1;j++){
		addCandidate(&barGraph,nodesIdx_find(way->refs[j])->idx,nodesIdx_find(way->refs[j+1])->idx);
	}
	return barGraph;

}


int ** makeAreaCandidates(struct map_t map, struct walk_area_t area,  int maxdist, int deg){

	int ** candidates;

	GARY_INIT_SPACE(candidates,0);
	printf("Area nodes:%d\n",area.n_nodes);

	struct line_t memline;
	memline = makeLine(0,0,0,0,-1,-1);
	
	int modulus;
	modulus = (area.n_nodes)/deg+1;

	for (int i=0;i<area.n_nodes;i++){

		for (int j=i+1; j<area.n_nodes;j++){
			int nidx1;
			int64_t nid1;
			int nidx2;
			int64_t nid2;

			nidx1 = area.nodeIdxs[i];
			nid1 = map.nodes[nidx1]->id;
			nidx2 = area.nodeIdxs[j];
			nid2 = map.nodes[nidx2]->id;
			Premap__Node * node1;
			Premap__Node * node2;
			node1 = map.nodes[nidx1];
			node2 = map.nodes[nidx2];
			
			if (onSameWay(node1->id,node2->id)||distance(node1,node2)>maxdist)
				continue;
			/*if (memline.startid==-1){
				memline = makeLine(node1->lon,node1->lat,node2->lon,node2->lat,0,0);
				printf("Candidates %d %d, distance: %d\n",nidx1,nidx2,distance(map.nodes[nidx1],map.nodes[nidx2]));	
				addCandidate(&candidates,nidx1,nidx2);
				continue;
			} */
			struct line_t line;
			line = makeLine(node1->lon,node1->lat,node2->lon,node2->lat,0,0);
			/*
				
			struct col_t col;
			col = calcMixCollision(line,memline);
			if (col.isCol && !col.atEndpoint){
				printf("Not at endpoint!\n");
				memline = line;
				continue;
			}
			if (col.isCol && col.atEndpoint){
				printf("At endpoint!\n");
				continue;
			}*/

			if (rand()%modulus!=0)
				continue;
			printf("Candidates %d %d, distance: %d\n",nidx1,nidx2,distance(map.nodes[nidx1],map.nodes[nidx2]));	
			addCandidate(&candidates,nidx1,nidx2);

		}
	}
	int candCount;
	candCount = GARY_SIZE(candidates);
	printf("Candidates: %d\n",candCount);
	return candidates;
		
}

int ** makeDirectCandidates(struct map_t map, struct raster_t raster, int ** wayGraph, int maxdist, int deg){

	int ** candidates;

	GARY_INIT_SPACE(candidates,100000);

	for (int lonidx=0;lonidx<raster.lonparts-1;lonidx++){
		for (int latidx=0;latidx<raster.latparts-1;latidx++){
			int count;
			count = GARY_SIZE(raster.raster[lonidx][latidx]);
			//printf("raster[%"PRId64"][%"PRId64"]=%d\n",lonidx,latidx,count);
			for (int i=0; i<count; i++) {
				int nidx1;
				int64_t nid1;
				int nidx2;
				int64_t nid2;
				nidx1 = raster.raster[lonidx][latidx][i];
				nid1 = map.nodes[nidx1]->id;
				if (!wayGraph[nidx1]){
					continue;
				}

				int modulus;
				modulus = (count)/deg+1;
				for (int j=i+1;j<count;j++){
					if (rand()%modulus!=0)
						continue;
					nidx2 = raster.raster[lonidx][latidx][j];
					nid2 = map.nodes[nidx2]->id;
					if (wayGraph[nidx2]&& !onSameWay(nid1,nid2)){
						addCandidate(&candidates,nidx1,nidx2);
					}
				}
				int neighCount;
				neighCount=GARY_SIZE(raster.raster[lonidx+1][latidx]);
				modulus = (neighCount)/deg+1;
				for (int j=0;j<neighCount;j++){
					if (rand()%modulus!=0)
						continue;
					nidx2 = raster.raster[lonidx+1][latidx][j];
					nid2 = map.nodes[nidx2]->id;
					if (wayGraph[nidx2]&&(distance(map.nodes[nidx1],map.nodes[nidx2])<=maxdist)&& !onSameWay(nid1,nid2)){
						addCandidate(&candidates,nidx1,nidx2);
					}
				}
				neighCount=GARY_SIZE(raster.raster[lonidx+1][latidx+1]);
				modulus = (neighCount)/deg+1;
				for (int j=0;j<neighCount;j++){
					if (rand()%modulus!=0)
						continue;
					nidx2 = raster.raster[lonidx+1][latidx+1][j];
					nid2 = map.nodes[nidx2]->id;
					if (wayGraph[nidx2]&&(distance(map.nodes[nidx1],map.nodes[nidx2])<=maxdist)&& !onSameWay(nid1,nid2)){
						addCandidate(&candidates,nidx1,nidx2);
					}
				}
				neighCount=GARY_SIZE(raster.raster[lonidx][latidx+1]);
				modulus = (neighCount)/deg+1;
				for (int j=0;j<neighCount;j++){
					if (rand()%modulus!=0)
						continue;
					nidx2 = raster.raster[lonidx][latidx+1][j];
					nid2 = map.nodes[nidx2]->id;
					if (wayGraph[nidx2]&&(distance(map.nodes[nidx1],map.nodes[nidx2])<=maxdist)&& !onSameWay(nid1,nid2)){
						addCandidate(&candidates,nidx1,nidx2);
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

// Sort
#define ASORT_PREFIX(X) lines_##X
#define ASORT_KEY_TYPE struct line_t
#define ASORT_LT(x,y) (((x).startid<(y).startid)||((x).endid<(y).endid)||((x).isBar<(y).isBar))
#include <ucw/sorter/array-simple.h>




SWEEP_TYPE lon;
unsigned char anglesign;

struct line_t * lines;

int tree_cmp(int idx1, int idx2){

	struct line_t line1;
	struct line_t line2;
	
	line1 = lines[idx1];
	line2 = lines[idx2];
	
	struct mixed_num_t lat1;
	struct mixed_num_t lat2;
	lat1 = calcMixLatForLon(line1,lon);
	lat2 = calcMixLatForLon(line2,lon);
	int cmp;
	cmp = mixedCmp(lat1,lat2);
	if (cmp)
		return cmp;

	SWEEP_TYPE dlat1;
	SWEEP_TYPE dlon1;
	SWEEP_TYPE dlat2;
	SWEEP_TYPE dlon2;
	dlat1 = line1.endlat - line1.startlat;
	dlon1 = line1.endlon - line1.startlon;
	dlat2 = line2.endlat - line2.startlat;
	dlon2 = line2.endlon - line2.startlon;
	//printf("Comparing angles, first:(%d,%d):(%d,%d), second:(%d,%d):(%d,%d)\n", line1.startlon,line1.startlat,dlon1,dlat1,line2.startlon,line2.startlat,dlon2,dlat2);
	if (dlat1*dlon2 < dlat2*dlon1){
	//	printf("First smaller\n");
		return -1*anglesign;
	}
	if (dlat1*dlon2 > dlat2*dlon1){
	//	printf("Second smaller\n");
		return 1*anglesign;	
	}
	return 0;

}



#define TREE_PREFIX(X) tree_##X 
#define TREE_NODE struct tree_node_t
#define TREE_KEY_ATOMIC lineIdx
#define TREE_WANT_CLEANUP
#define TREE_WANT_FIND
#define TREE_WANT_ADJACENT
//#define TREE_WANT_DELETE
#define TREE_WANT_REMOVE
#define TREE_WANT_NEW
#define TREE_WANT_ITERATOR
#define TREE_GIVE_CMP
#include <ucw/redblack.h>


struct int_event_t makeIntEvent(struct line_t * lines,int l1Idx, int l2Idx, struct mixed_num_t lon){
	struct int_event_t intevt;
	struct col_t col;
	printf("Lon: %lld + %lld/%lld ~ %lld, l1: %d, l2: %d\n",
		lon.base,lon.numer,lon.denom,lon.base+(lon.numer/lon.denom),
		l1Idx,l2Idx);
	printf("Line: (%"PF(S_P)",%"PF(S_P)")--(%lld,%lld) %lld/%lld\n",
			lines[l1Idx].startlon,lines[l1Idx].startlat,
			lines[l1Idx].endlon,lines[l1Idx].endlat,
			lines[l1Idx].endlat-lines[l1Idx].startlat,
			lines[l1Idx].endlon-lines[l1Idx].startlon);
	printf("Line: (%"PF(S_P)",%"PF(S_P)")--(%lld,%lld) %lld/%lld\n",
			lines[l2Idx].startlon,lines[l2Idx].startlat,
			lines[l2Idx].endlon,lines[l2Idx].endlat,
			lines[l2Idx].endlat-lines[l2Idx].startlat,
			lines[l2Idx].endlon-lines[l2Idx].startlon);
	col = calcMixCollision(lines[l1Idx],lines[l2Idx]);					
	if (col.isCol==0){
		intevt.lineIdx=-1;
		return intevt;
	}
	if (!col.atEndpoint){
		printf("Col: %lld + %lld/%lld ~ %lld\n",col.lon.base,col.lon.numer,col.lon.denom,col.lon.base+col.lon.numer/col.lon.denom);
	}
	if ((mixedCmp(col.lon,lon)<=0)){
		intevt.lineIdx=-1;
		return intevt;
	}
	printf("Intersection");

	intevt.type = EVT_INTERSECT;
	intevt.lon = col.lon;
	intevt.lat = col.lat;
	SWEEP_TYPE dlat1;
	SWEEP_TYPE dlat2;
	SWEEP_TYPE dlon1;
	SWEEP_TYPE dlon2;
	dlat1 = lines[l1Idx].endlat-lines[l1Idx].startlat;
	dlat2 = lines[l2Idx].endlat-lines[l2Idx].startlat;
	dlon1 = lines[l1Idx].endlon-lines[l1Idx].startlon;
	dlon2 = lines[l2Idx].endlon-lines[l2Idx].startlon;
	if (dlat1*dlon2 < dlon1*dlat2){
		intevt.dlat = dlat1;
		intevt.dlon = dlon1;
	}else{
		intevt.dlat = dlat2;
		intevt.dlon = dlon2;
	}
	intevt.lineIdx = l1Idx;
	intevt.line2Idx = l2Idx;
	if (lines[intevt.lineIdx].isBar || lines[intevt.line2Idx].isBar){
		lines[l1Idx].broken = 1;
		lines[l2Idx].broken = 1;
	}
	return intevt; 


}
int eventCmp(struct event_t evt1,struct event_t evt2){
	if (evt1.lon<evt2.lon)
		return 1;
	if (evt1.lon>evt2.lon)
		return 0;
	if (evt1.lat<evt2.lat)
		return 1;
	if (evt1.lat>evt2.lat)
		return 0;
	if (evt1.type<evt2.type)
		return 1;
	if (evt1.type>evt2.type)
		return 0;
	if (evt1.dlat*evt2.dlon < evt1.dlon*evt2.dlat)
		return 1;
	if (evt1.dlat*evt2.dlon > evt1.dlon*evt2.dlat)
		return 0;
	if (evt1.lineIdx < evt2.lineIdx)
		return 1;
	if (evt1.lineIdx > evt2.lineIdx)
		return 0;
	if (evt1.line2Idx < evt2.line2Idx)
		return 1;
	if (evt1.line2Idx > evt2.line2Idx)
		return 0;
	return 0;
}

int intEventCmp(struct int_event_t evt1,struct int_event_t evt2){
	if (evt1.lon.base<evt2.lon.base)
		return 1;
	if (evt1.lon.base>evt2.lon.base)
		return 0;
	if (evt1.lon.numer*evt2.lon.denom<evt2.lon.numer*evt1.lon.denom)
		return 1;
	if (evt1.lon.numer*evt2.lon.denom>evt2.lon.numer*evt1.lon.denom)
		return 0;
	if (evt1.lat.base<evt2.lat.base)
		return 1;
	if (evt1.lat.base>evt2.lat.base)
		return 0;
	if (evt1.lat.numer*evt2.lat.denom<evt2.lat.numer*evt1.lat.denom)
		return 1;
	if (evt1.lat.numer*evt2.lat.denom>evt2.lat.numer*evt1.lat.denom)
		return 0;
	if (evt1.type<evt2.type)
		return 1;
	if (evt1.type>evt2.type)
		return 0;
	if (evt1.dlat*evt2.dlon < evt1.dlon*evt2.dlat)
		return 1;
	if (evt1.dlat*evt2.dlon > evt1.dlon*evt2.dlat)
		return 0;
	if (evt1.lineIdx < evt2.lineIdx)
		return 1;
	if (evt1.lineIdx > evt2.lineIdx)
		return 0;
	if (evt1.line2Idx < evt2.line2Idx)
		return 1;
	if (evt1.line2Idx > evt2.line2Idx)
		return 0;
	return 0;
}

int int_seEventCmp(struct int_event_t evt1,struct event_t evt2){
	printf("lon1: %lld,lon2:%lld\n",evt1.lon.base,evt2.lon);
	if (evt1.lon.base<evt2.lon)
		return -1;
	if (evt1.lon.base>evt2.lon)
		return 1;
	evt2.lon-=evt1.lon.base;
	if (evt1.lon.numer < evt1.lon.denom*evt2.lon)
		return -1;
	if (evt1.lon.numer > evt1.lon.denom*evt2.lon)
		return 1;
	if (evt1.lat.base<evt2.lat)
		return -1;
	if (evt1.lat.base>evt2.lat)
		return 1;
	evt2.lat-=evt1.lat.base;
	if (evt1.lat.numer < evt1.lat.denom*evt2.lat)
		return -1;
	if (evt1.lat.numer > evt1.lat.denom*evt2.lat)
		return 1;
	return 0;
}

struct line_t * findDirectWays(struct map_t map, int ** candidates, int ** barGraph){
	int n_cand;
	n_cand = GARY_SIZE(candidates);
	//struct line_t * lines;
	GARY_INIT_SPACE(lines,n_cand);
	for (int i=0;i<n_cand;i++){
		Premap__Node * n1;
		Premap__Node * n2;
		n1 = map.nodes[candidates[i][0]];
		n2 = map.nodes[candidates[i][1]];
		
		struct line_t * line;
		line = GARY_PUSH(lines);

		if ((n2->lon < n1->lon)||((n2->lon==n1->lon)&&(n2->lat<n1->lat))){
			line->startlon = n2->lon*SWEEP_SCALE;
			line->startlat = (n2->lat)*SWEEP_SCALE;
			line->endlon = (n1->lon)*SWEEP_SCALE;
			line->endlat = (n1->lat)*SWEEP_SCALE;
			line->startid = n2->id;
			line->endid = n1->id;
		} else {
			line->startlon = (n1->lon)*SWEEP_SCALE;
			line->startlat = (n1->lat)*SWEEP_SCALE;
			line->endlon = (n2->lon)*SWEEP_SCALE;
			line->endlat = (n2->lat)*SWEEP_SCALE;
			line->startid = n1->id;
			line->endid = n2->id;
		}
		line->isBar = 0;
		line->broken = 0;
		line->started = 0;
		line->ended = 0;
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

		if ((n2->lon < n1->lon)||((n2->lon==n1->lon)&&(n2->lat<n1->lat))){
			line->startlon = (n2->lon)*SWEEP_SCALE;
			line->startlat = (n2->lat)*SWEEP_SCALE;
			line->endlon = (n1->lon)*SWEEP_SCALE;
			line->endlat = (n1->lat)*SWEEP_SCALE;
			line->startid = n2->id;
			line->endid = n1->id;
		} else {
			line->startlon = (n1->lon)*SWEEP_SCALE;
			line->startlat = (n1->lat)*SWEEP_SCALE;
			line->endlon = (n2->lon)*SWEEP_SCALE;
			line->endlat = (n2->lat)*SWEEP_SCALE;
			line->startid = n1->id;
			line->endid = n2->id;
		}
		line->isBar = 1;
		line->broken = 0;
		line->started = 0;
		line->ended = 0;
	}

	// Remove duplicities
	// Make queue
	int n_seQueue;
	n_seQueue = GARY_SIZE(lines)*2;
	struct event_t * seQueue;
	GARY_INIT_SPACE(seQueue,n_seQueue);
	GARY_PUSH(seQueue);
	for (int i=0;i<n_seQueue/2;i++){
		struct event_t * event;
		event = GARY_PUSH(seQueue);
		event->type = EVT_START;
		event->lon = lines[i].startlon;
		event->lat = lines[i].startlat;
		event->dlon = lines[i].endlon-lines[i].startlon;
		event->dlat = lines[i].endlat-lines[i].startlat;
		event->lineIdx = i;

		event = GARY_PUSH(seQueue);
		event->type = EVT_END;
		event->lon = lines[i].endlon;
		event->lat = lines[i].endlat;
		event->dlon = lines[i].startlon-lines[i].endlon;
		event->dlat = lines[i].startlat-lines[i].endlat;
		event->lineIdx = i;
	}
	
	#define EVENT_CMP(x,y) eventCmp(x,y)

	HEAP_INIT(struct event_t,seQueue,n_seQueue,EVENT_CMP,HEAP_SWAP);
	
	struct int_event_t * intQueue;
	GARY_INIT(intQueue,1);
	int n_intQueue;
	n_intQueue = 0;

	struct tree_node_t ** lineNodes;
	lineNodes = malloc(sizeof(struct tree_node_t *) * GARY_SIZE(lines));
	for (int i=0;i<GARY_SIZE(lines);i++){
		lineNodes[i]=NULL;
	}
	struct tree_tree * tree;
	tree = malloc(sizeof(struct tree_tree));
	tree_init(tree);
	struct event_t memevt;
	lon=0;
	while (n_seQueue >0)
	{
		if ((n_seQueue & 0xFFFF) == 0)
			printf("Queue: %d, tree: %d\n",n_seQueue,tree->count);
		printf("Tree has %d nodes\n",tree->count);
		printf("Heap has %d events\n",n_seQueue);
		printf("Heap2 has %d events\n",n_intQueue);

		int evtcmp;
		if (n_intQueue==0){
			evtcmp=1;	
		}else{
			evtcmp = int_seEventCmp(intQueue[1],seQueue[1]);
		}
		if ((evtcmp==1)||(evtcmp==0 && seQueue[1].type==EVT_END)){ // START / END

			HEAP_DELETE_MIN(struct event_t,seQueue,n_seQueue,EVENT_CMP,HEAP_SWAP);
			struct event_t evt;
	//		printf("LineIdx: %d %d\n",evt.lineIdx, evt.type);
			evt = seQueue[n_seQueue+1];
			lon = evt.lon;
			/*if ((evt.type==memevt.type)&&
				(evt.lineIdx==memevt.lineIdx)&&
				((evt.type!=EVT_INTERSECT)||(evt.line2Idx==memevt.line2Idx))){
				continue;	
			}
			memevt.type=evt.type;
			memevt.lineIdx=evt.lineIdx;
			memevt.line2Idx=evt.line2Idx;
			
			*/
			printf("Event: %"PF(S_P)" %"PF(S_P)" %d %d\n",evt.lon,evt.lat,evt.type,(evt.dlon==0)?-1:evt.dlat/evt.dlon);

			struct line_t line;
			line = lines[evt.lineIdx];
			if (line.startlon == line.endlon){
	//			printf("Special case\n");
				continue;
			}
			struct int_event_t intevt;
			struct tree_node_t * prev;
			struct tree_node_t * next;

			switch (evt.type){
				case EVT_START:
					anglesign = 1;
					printf("EVT_START\n");
					printf("Lon: %lld, lat: %lld\n",evt.lon,evt.lat);
					struct tree_node_t * node;
					node = tree_find(tree,evt.lineIdx);
					if (node){
						printf("Node already present\n");
						continue;
					}
					lines[evt.lineIdx].started=1;
					printf("Line: F:%d, T:%d (%"PF(S_P)",%"PF(S_P)")--(%"PF(S_P)",%"PF(S_P)")\n",line.startid, line.endid,line.startlon,line.startlat,line.endlon,line.endlat);
					node = tree_new(tree,evt.lineIdx);
					lineNodes[evt.lineIdx]=node;
					prev = tree_adjacent(node,0);
					next = tree_adjacent(node,1);

					struct mixed_num_t mixlon;
					mixlon = makeMix(evt.lon,0,1);

					//printf("Neighs: %p %d %d\n",*tree,prev.lineIdx,next.lineIdx);
					if (prev){
						printf("Int after start - prev\n");
						intevt = makeIntEvent(lines,evt.lineIdx,prev->lineIdx,mixlon);
						if (intevt.lineIdx!=-1){
							GARY_PUSH(intQueue);
							HEAP_INSERT(struct int_event_t, intQueue, n_intQueue, intEventCmp, HEAP_SWAP, intevt);
						}
					}
					if (next){
						printf("Int after start - next\n");
						intevt = makeIntEvent(lines,evt.lineIdx,next->lineIdx,mixlon);
						if (intevt.lineIdx!=-1){
							GARY_PUSH(intQueue);
							HEAP_INSERT(struct int_event_t, intQueue, n_intQueue, intEventCmp, HEAP_SWAP, intevt);
						}
					}
					break;
				case EVT_END:
					anglesign=-1;
					printf("EVT_END\n");
					
					line = lines[evt.lineIdx];
					printf("Line: F:%d, T:%d (%"PF(S_P)",%"PF(S_P)")--(%"PF(S_P)",%"PF(S_P)")\n",line.startid, line.endid,line.startlon,line.startlat,line.endlon,line.endlat);
					if (!line.started){
						printf("Line not started!\n");
						break;
	//					printf("%d:(%d,%d)--%d:(%d,%d),%d\n",line.startid, line.startlat, line.startlon, line.endid, line.endlat, line.endlon, line.isBar);
					} else {
	//					printf("Ended ok\n");
					}
					lines[evt.lineIdx].ended=1;
					node = lineNodes[evt.lineIdx];
					prev = tree_adjacent(node,0);
					next = tree_adjacent(node,1);

					mixlon = makeMix(evt.lon,0,1);

					//printf("Neighs: %p %d %d\n",*tree,prev.lineIdx,next.lineIdx);
					if (prev && next){
						printf("Int after end\n");
						intevt = makeIntEvent(lines,prev->lineIdx,next->lineIdx,mixlon);
						if (intevt.lineIdx!=-1){
							GARY_PUSH(intQueue);
							HEAP_INSERT(struct int_event_t, intQueue, n_intQueue, intEventCmp, HEAP_SWAP, intevt);
						}
					}
					tree_remove(tree,node);
					break;
				case EVT_INTERSECT:
					break;	
			}
		
		} else { // Intersection
			HEAP_DELETE_MIN(struct int_event_t,intQueue,n_intQueue,intEventCmp,HEAP_SWAP);
			struct int_event_t evt;
	//		printf("LineIdx: %d %d\n",evt.lineIdx, evt.type);
			evt = intQueue[n_intQueue+1];

			anglesign = -1;
			printf("EVT_INT\n");
			if (evt.lineIdx == memevt.lineIdx && evt.line2Idx == memevt.line2Idx){
				printf("Duplicit event, skipping\n");
				continue;
			}
			memevt.lineIdx = evt.lineIdx;
			memevt.line2Idx = evt.line2Idx;
			printf("Event lon: ~ %lld lat: ~ %d l1: %d l2: %d\n",
				evt.lon.base+evt.lon.numer/evt.lon.denom,
				evt.lat.base+evt.lon.numer/evt.lon.denom,
				evt.lineIdx,evt.line2Idx);
			lon = evt.lon.base;
			struct line_t l1;
			struct line_t l2;

			l1 = lines[evt.lineIdx];
			l2 = lines[evt.line2Idx];

			if (l1.ended || l2.ended){
				printf("Invalid intersection\n");
				continue;
			}

			struct tree_node_t * higher;
			struct tree_node_t * lower;

			
			if (tree_adjacent(lineNodes[evt.lineIdx],1)==lineNodes[evt.line2Idx]){
				lower = lineNodes[evt.lineIdx];
				higher = lineNodes[evt.line2Idx];

				lineNodes[evt.lineIdx]=higher;
				lineNodes[evt.line2Idx]=lower;
			} else 
			if (tree_adjacent(lineNodes[evt.lineIdx],0)==lineNodes[evt.line2Idx]){
				lower = lineNodes[evt.line2Idx];
				higher = lineNodes[evt.lineIdx];

				lineNodes[evt.lineIdx]=lower;
				lineNodes[evt.line2Idx]=higher;
			} else {
				printf("Problem with adjacency\n");
				continue;
			}

			int tmp;
			tmp = higher->lineIdx;
			higher->lineIdx = lower->lineIdx;
			lower->lineIdx = tmp;

			int lowerIdx;
			int higherIdx;
			lowerIdx = lower->lineIdx;
			higherIdx = higher->lineIdx;
			
			struct tree_node_t * prev;
			struct tree_node_t * next;

			prev = tree_adjacent(lower,0);
			next = tree_adjacent(higher,1);

			struct int_event_t intevt;

			if (prev){
				printf("Int after int\n");
				intevt = makeIntEvent(lines,lowerIdx,prev->lineIdx,evt.lon);
				if (intevt.lineIdx!=-1){
					GARY_PUSH(intQueue);
					HEAP_INSERT(struct int_event_t, intQueue, n_intQueue, intEventCmp, HEAP_SWAP, intevt);
				}
			}
			if (next){
				printf("Int after int\n");
				intevt = makeIntEvent(lines,higherIdx,next->lineIdx,evt.lon);
				if (intevt.lineIdx!=-1){
					GARY_PUSH(intQueue);
					HEAP_INSERT(struct int_event_t, intQueue, n_intQueue, intEventCmp, HEAP_SWAP, intevt);
				}
			}

		}
		
	//	continue;
		// Debug tree order
		struct mixed_num_t memlat;
		memlat = makeMix(0,0,1);
		TREE_FOR_ALL(tree,tree,iternode)
		{
			printf("Lon: %lld ",lon);
			struct line_t l;
			l = lines[iternode->lineIdx];
			struct mixed_num_t lat;
			lat = calcMixLatForLon(l,lon);
			if (mixedCmp(memlat,lat)>0){
				if (memlat.base == lat.base)
					printf("Bad order:");
				else
					printf("Wrong order:");
		//		TREE_BREAK;
			}
			else{ 
				printf("OK:");
			}
			mixedPrint(memlat);
			printf(" vs ");
			mixedPrint(lat);
			printf(" %d/%d\n",l.endlat-l.startlat,l.endlon-l.startlon);
			memlat = lat;
		}
		TREE_END_FOR;
	}
	tree_cleanup(tree);
	
	return lines;
} 


struct map_t addDirectToMap(struct line_t * lines, struct map_t map){
	for (int i=0;i<GARY_SIZE(lines);i++){
		//if (lines[i].broken){
		//	printf("Broken line\n");
		//	continue;
		//}
		Premap__Way * way;
		way = malloc(sizeof(Premap__Way));
		premap__way__init(way);
		way->id = -2000-i;
		way->n_refs=2;
		way->refs = malloc(sizeof(way->refs[0])*2);
		way->refs[0] = lines[i].startid;
		way->refs[1] = lines[i].endid;
		way->has_type = true;
		if (!lines[i].broken && lines[i].isBar==0){
			way->type = OBJTYPE__DIRECT;
			//printf("Writing way %d\n",way->type);
		}
		else
			{
			way->type = OBJTYPE__DIRECT_BAD;
			//printf("Writing bad way %d\n",way->type);
			}
		Premap__Way ** ptr;
		ptr = GARY_PUSH(map.ways);
		*ptr = way;
		map.n_ways++;
	}

	return map;
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
		way->has_type = true;
		Premap__Way ** ptr;
		ptr = GARY_PUSH(map.ways);
		*ptr = way;
		map.n_ways++;
	}

	return map;
}

void checkWayTypes(Premap__Map * pbmap){
	for (int i=0;i<pbmap->n_ways;i++){
		printf("Way: %d, type: %d\n",i,pbmap->ways[i]->type);
	}	

}
uint8_t nodeInPolygon(struct map_t map, Premap__Node * node, Premap__Way * way){
	if (way->refs[0]!=way->refs[way->n_refs-1]){
//		printf("Way is not closed!\n");
		return 0;
	}
	int64_t min_lon;
	min_lon = map.nodes[nodesIdx_find(way->refs[0])->idx]->lon;
	int64_t lat;
	lat = map.nodes[nodesIdx_find(way->refs[0])->idx]->lat;
	for (int i=1;i<way->n_refs;i++)
		min_lon = MIN(min_lon,map.nodes[nodesIdx_find(way->refs[i])->idx]->lon);

	struct line_t nodeLine;
	struct line_t periLine;
	nodeLine = makeLine(node->lon,node->lat,min_lon-100,lat,0,0);

	int crosses;
	crosses = 0;
	for (int i=0;i<way->n_refs-1;i++){
		Premap__Node * n1;
		Premap__Node * n2;
		n1 = map.nodes[nodesIdx_find(way->refs[i])->idx];
		n2 = map.nodes[nodesIdx_find(way->refs[i+1])->idx];
		periLine = makeLine(n1->lon,n1->lat,n2->lon,n2->lat,0,0);

		struct col_t col;
		col = calcMixCollision(nodeLine,periLine);
		if (!col.isCol)
			continue;
		if (!col.atEndpoint){
			crosses++;
			continue;
		}
		// FIXME special cases
		crosses++;
	}
	return crosses%2;
}


struct walk_area_t * findWalkAreas(struct map_t map, struct raster_t raster){
	struct walk_area_t * walkareas;	
	GARY_INIT(walkareas,0);
	for (int i=0;i<map.n_ways;i++){	
		if (!isWalkArea(map.ways[i]))
			continue;
		struct walk_area_t * area;
		area = GARY_PUSH(walkareas);
		area->periIdx = i;

		int64_t * refs;

		refs = map.ways[i]->refs;
		
		int minx;
		int maxx;
		int miny;
		int maxy;
		
		int * box;

		box = getRasterBox(raster,map.nodes[nodesIdx_find(refs[0])->idx]->lon,map.nodes[nodesIdx_find(refs[0])->idx]->lat);

		minx = maxx = box[0];
		miny = maxy = box[1];

		for (int j=0;j<map.ways[i]->n_refs;j++){
			int nidx;
			nidx = nodesIdx_find(refs[j])->idx;
			box = getRasterBox(raster,map.nodes[nidx]->lon, map.nodes[nidx]->lat);
			minx = MIN(minx,box[0]);
			maxx = MAX(maxx,box[0]);
			miny = MIN(miny,box[1]);
			maxy = MAX(maxy,box[1]);
		}

		int64_t * ways;
		GARY_INIT(ways,0);
		GARY_INIT(area->nodeIdxs,0);
		for (int x=minx;x<=maxx;x++){
			for (int y=miny;y<=maxy;y++){
				int * idxs;
				idxs = raster.raster[x][y];
				for (int k=0;k<GARY_SIZE(idxs);k++){
					if (!nodeInPolygon(map,map.nodes[idxs[k]],map.ways[i])){
						continue;
					}
					int * nodeIdx;
					nodeIdx = GARY_PUSH(area->nodeIdxs);
					*nodeIdx = idxs[k];
					int64_t * nways = nodeWays_find(map.nodes[idxs[k]]->id)->ways;
					for (int j=0;j<GARY_SIZE(nways);j++){
						int64_t * way;
						way = GARY_PUSH(ways);
						* way = nways[j]; 
					}
				}
			}
		}

		map.ways[i]->type = OBJTYPE__WALKAREA;
		
		printf("Area has %d squares\n",(maxx-minx)*(maxy-miny));

		area->n_nodes = GARY_SIZE(area->nodeIdxs);
		if (GARY_SIZE(ways)==0){
			area->n_bars=0;
			area->n_ways=0;
			continue;
		} 

		int64_sort(ways,GARY_SIZE(ways));
		
		printf("Ways: %d\n",GARY_SIZE(ways));

		GARY_INIT(area->barIdxs,0);
		GARY_INIT(area->wayIdxs,0);

		int64_t item;
		item = ways[0];
		int64_t memitem;
		memitem = ways[0];
		int index;
		for (int j=0;j<GARY_SIZE(ways);j++){
			if (memitem==ways[j])
				continue;
			//printf("Item\n");
			memitem = ways[j];
			index = waysIdx_find(ways[j])->idx;
			if (isBarrier(map.ways[index])){
			//	printf("Barrier\n");
				int * last;
				last = GARY_PUSH(area->barIdxs);
				*last = index;
				map.ways[index]->type = OBJTYPE__AREABAR;
			}
			if (isWay(map.ways[index])){
			//	printf("Way\n");
				int * last;
				last = GARY_PUSH(area->wayIdxs);
				*last = index;
			//	map.ways[index]->type = OBJTYPE__AREAWAY;
			}
		}
		area->n_bars = GARY_SIZE(area->barIdxs);
		area->n_ways = GARY_SIZE(area->wayIdxs);
	}
	return walkareas;
} 

void prepareForVoronoi(struct map_t map, struct walk_area_t * waklareas){
	
}

struct map_t removeBarriers(struct map_t map){
	Premap__Way ** newways;
	GARY_INIT(newways,0);
	for (int i=0;i<map.n_ways;i++){
		if (isBarrier(map.ways[i])||map.ways[i]->type==OBJTYPE__DIRECT_BAD){
			free(map.ways[i]);
			continue;
		}
		Premap__Way ** way;
		way = GARY_PUSH(newways);
		*way = map.ways[i];
	}
	GARY_FREE(map.ways);
	map.ways = newways;
	map.n_ways = GARY_SIZE(newways);

	Premap__Node ** newnodes;
	GARY_INIT(newnodes,0);

	nodeWays_refresh(map);
	for (int i=0;i<map.n_nodes;i++){
		if (GARY_SIZE(nodeWays_find(map.nodes[i]->id)->ways)==0){
			free(map.nodes[i]);
			continue;
		}
		Premap__Node ** node;
		node = GARY_PUSH(newnodes);
		*node = map.nodes[i];
	}
	GARY_FREE(map.nodes);
	map.nodes=newnodes;
	map.n_nodes = GARY_SIZE(newnodes);

	return map;
}

Premap__Map * loadMap(char * filename){
	FILE * IN;
	IN = fopen(filename,"r");
	if (IN==NULL){
		printf("File opening error\n");	
		return NULL;
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
		return NULL;
	}
	fclose(IN);
	
	Premap__Map *pbmap;
	pbmap = premap__map__unpack(NULL,len,buf);
	return pbmap;
}
	
int saveMap(Premap__Map * pbmap,char * filename){
	int64_t len;
	uint8_t * buf;
	len = premap__map__get_packed_size(pbmap);
	buf = malloc(len);
	premap__map__pack(pbmap,buf);
	FILE * OUT;
	OUT = fopen(filename,"w");
	if (OUT==NULL){
		printf("File opening error\n");	
		return 1;
	}
	fwrite(buf,1,len,OUT);
	fclose(OUT);
	return 0;

}

int saveSearchGraph(Graph__Graph * graph,char * filename){
	int64_t len;
	uint8_t * buf;
	len = graph__graph__get_packed_size(graph);
	buf = malloc(len);
	graph__graph__pack(graph,buf);
	FILE * OUT;
	OUT = fopen(filename,"w");
	if (OUT==NULL){
		printf("File opening error\n");	
		return 1;
	}
	fwrite(buf,1,len,OUT);
	fclose(OUT);
	return 0;

}
Graph__Graph * makeSearchGraph(struct map_t map){
	Graph__Graph * graph;
	graph = malloc(sizeof(Graph__Graph));
	graph__graph__init(graph);
	Graph__Vertex ** vertices;
	vertices = malloc(sizeof(Graph__Vertex *)*map.n_nodes);
	for (int i=0;i<map.n_nodes;i++){
		Graph__Vertex * v;
		v = malloc(sizeof(Graph__Vertex));
		graph__vertex__init(v);
		v->idx = i;
		v->osmid = map.nodes[i]->id;
		v->lat = int2deg(map.nodes[i]->lat);
		v->lon = int2deg(map.nodes[i]->lon);
		vertices[i]=v;
	}
	graph->vertices = vertices;
	graph->n_vertices = map.n_nodes;

	Graph__Edge ** edges;
	GARY_INIT(edges,0);
	int edgecnt;
	edgecnt = 0;
	for (int i=0;i<map.n_ways;i++){
		Premap__Way * way;
		way = map.ways[i];
		for (int j=0;j<(way->n_refs-1);j++){
			int fromIdx;
			int toIdx;
			fromIdx = nodesIdx_find(way->refs[j])->idx;
			toIdx = nodesIdx_find(way->refs[j+1])->idx;
			Graph__Edge * e;
			e = malloc(sizeof(Graph__Edge));
			graph__edge__init(e);
			e->idx = edgecnt++;
			e->osmid = way->id;
			e->vfrom = fromIdx;
			e->vto = toIdx;
			e->type = way->type;
			Graph__Edge ** eptr;
			eptr = GARY_PUSH(edges);
			*eptr = e;

			e = malloc(sizeof(Graph__Edge));
			graph__edge__init(e);
			e->idx = edgecnt++;
			e->osmid = way->id;
			e->vfrom = toIdx;
			e->vto = fromIdx;
			e->type = way->type;
			eptr = GARY_PUSH(edges);
			*eptr = e;
		}
			
	}
	graph->edges = edges;
	graph->n_edges = GARY_SIZE(edges);
	return graph;
	
	
}

void freeNX2Array(int ** array){
	for(int i=0;i<GARY_SIZE(array);i++)
		free(array[i]);
	GARY_FREE(array);	
}

int main (int argc, char ** argv){
	
	Premap__Map * pbmap;
	pbmap = loadMap("../data/praha-union.pbf");
	if (pbmap==NULL){
		printf("Error loading file\n");	
		return 2;
	}
	struct map_t map;
	map = initMap(pbmap);
	printf("Loaded %d nodes, %d ways\n",map.n_nodes,map.n_ways);
	nodesIdx_refresh(map.n_nodes,map.nodes);
	waysIdx_refresh(map.n_ways,map.ways);
	nodeWays_refresh(map);
	struct graph_t graph;
	graph = makeGraph(map);
	struct raster_t raster;
	raster = makeRaster(map);
	int ** candidates;
	candidates = makeDirectCandidates(map,raster,graph.wayGraph,20,2);
	lines = findDirectWays(map,candidates,graph.barGraph);
	freeNX2Array(graph.barGraph);
	freeNX2Array(candidates);
	map = addDirectToMap(lines,map);
	struct walk_area_t * walkareas;
	walkareas = findWalkAreas(map,raster);
	for (int i=0;i<GARY_SIZE(walkareas);i++){
		candidates = makeAreaCandidates(map,walkareas[i],300,2);
	//	map = addCandidatesToMap(candidates,map);
		int ** barGraph = makeAreaBarGraph(map,walkareas[i]);
		lines = findDirectWays(map,candidates,barGraph);
		map = addDirectToMap(lines,map);
		freeNX2Array(barGraph);
		freeNX2Array(candidates);
	}
	map = removeBarriers(map);
	printf("Found %d walk areas\n",GARY_SIZE(walkareas));
	mapToPBF(map,pbmap);
	//checkWayTypes(pbmap);
	if (saveMap(pbmap,"../data/praha-union-c.pbf")!=0)
		printf("Saving map failed\n");
	Graph__Graph * searchGraph;
	searchGraph = makeSearchGraph(map);
	saveSearchGraph(searchGraph,"../data/praha-graph.pbf");
	return 0;
}
