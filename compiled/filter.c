#include <ucw/lib.h>
#include <ucw/fastbuf.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include <ucw/gary.h>
#include <ucw/heap.h>
#include "include/geodesic.h"

#define SCALE 1000000
#define SWEEP_SCALE 100

#include "types.h"
#include "raster.h"
//#include "tree.h"
#include "hashes.c"
#include "utils.h"

int scale = 1000000;


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

int64_t calcLatForLon(struct line_t line,int64_t lon){

//	if (line.endlon < lon){
//		return line.endlat;
//	}
//	if (line.startlon > lon){
//		return line.startlat;
//	}

//	return line.startlat;

	int64_t minlat = MIN(line.startlat,line.endlat);
	int64_t maxlat = MAX(line.startlat,line.endlat);	

	int64_t b = line.endlon-line.startlon;
	int64_t a = -(line.endlat-line.startlat);
	int64_t c = (a*line.startlon+b*line.startlat);

	int64_t lat;
	if (b!=0)
		lat = (-a*lon+c)/b;
	else
		lat = line.startlat;


	int64_t cit = (lon-line.startlon)*(line.endlat-line.startlat);
	int64_t jm = line.endlon - line.startlon;
	
	//printf("LFL: %d %d\n",line.startlon,line.startlat);
	//printf("LFL: %d %d\n",lon, (cit/jm)+line.startlat);
	//printf("LFL: %d %d\n",line.endlon,line.endlat);
	//printf("LFL: \n");
	
//	printf("Line: (%d,%d)--(%d,%d), lon: %d, lat: %d\n",line.startlon,line.startlat,line.endlon,line.endlat,lon,cit/jm);
	if ((line.endlon < lon)||(line.startlon > lon)){
		printf("Line already ended!, lon:%d\n",lon);
		printf("Line: (%d,%d)--(%d,%d)\n",line.startlon,line.startlat,line.endlon,line.endlat);
	}else
	if ((lat < minlat) || (lat > maxlat)){
		printf("Latitude is terribly wrong\n");
	}

	//return (cit/jm)+line.startlat;
	return lat;
}

int64_t * calcCollision(struct line_t line1, struct line_t line2){
	int64_t lon1 = line1.startlon;
	int64_t lon2 = line1.endlon;
	int64_t lat1 = line1.startlat;
	int64_t lat2 = line1.endlat;
	int64_t lon3 = line2.startlon;
	int64_t lon4 = line2.endlon;
	int64_t lat3 = line2.startlat;
	int64_t lat4 = line2.endlat;
	int64_t cit = (lon1*lat2-lat1*lon2)*(lon3-lon4)-(lon3*lat4-lat3*lon4)*(lon1-lon2);
	int64_t jm = (lon1-lon2)*(lat3-lat4)-(lat1-lat2)*(lon3-lon4);	
	//int64_t cit = (lat2-lat1)*(lon4-lon3)*lon1+(lon2-lon1)*(lon4-lon3)*(lat3-lat1)-(lat4-lat3)*(lon2-lon1)*lon3;
	//int64_t jm = (lat2-lat1)*(lon4-lon3)-(lon2-lon1)*(lat4-lat3);
	if (jm == 0)
		return NULL;
	int64_t lon = cit/jm;
	if (lon1 > lon || lon > lon2 || lon3 > lon || lon > lon4){
		return NULL;    
	}
	cit = (lon1*lat2-lat1*lon2)*(lat3-lat4)-(lon3*lat4-lat3*lon4)*(lat1-lat2);
	int64_t lat = cit/jm;
	//int64_t lat = (lon*(lat2-lat1)+lon1*(lon2-lon1)-lon1*(lat2-lat1))/(lon2-lon1);

	//printf("Line 1: (%d,%d)--(%d,%d)\n",lon1,lat1,lon2,lat2);
	//printf("Line 2: (%d,%d)--(%d,%d)\n",lon3,lat3,lon4,lat4);
	//printf("Intersection: %d,%d\n",lon,lat);
	if (lat1 > lat || lat > lat2 || lat3 > lat || lat > lat4){
		return NULL;
	}
	//return (lon,lat)
	int64_t * result;
	result = malloc(sizeof(int64_t)*2);
	result[0] = lon;
	result[1] = lat;
	return result;
	
}


enum event_type  {EVT_START=2,EVT_END=0,EVT_INTERSECT=1};
struct event_t {
	enum event_type type;
	int64_t lon;
	int64_t lat;
	int64_t dlon;
	int64_t dlat;	
	unsigned int lineIdx;
	unsigned int line2Idx;
};



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

int linesCmp(struct line_t  line1,struct line_t  line2, int64_t lon,int anglesign){
	
	// DEBUG
	if (line1.startid<line2.startid)
		return -1;

	if (line1.startid>line2.startid)
		return 1;

	if (line1.endid<line2.endid)
		return -1;

	if (line1.endid>line2.endid)
		return 1;
	return 0;
	

	int64_t lat1;
	int64_t lat2;
	lat1 = calcLatForLon(line1,lon);
	lat2 = calcLatForLon(line2,lon);
	if (lat1 < lat2)
		return -1;
	if (lat1 > lat2)
		return 1;
	int dlat1;
	int dlon1;
	int dlat2;
	int dlon2;
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
// Sort
#define ASORT_PREFIX(X) lines_##X
#define ASORT_KEY_TYPE struct line_t
#define ASORT_LT(x,y) (((x).startid<(y).startid)||((x).endid<(y).endid)||((x).isBar<(y).isBar))
#include <ucw/sorter/array-simple.h>




int64_t lon;
unsigned char anglesign;

struct line_t * lines;

int tree_cmp(int idx1, int idx2){
	
	struct line_t line1;
	struct line_t line2;
	
	line1 = lines[idx1];
	line2 = lines[idx2];
	
	/*
	// DEBUG
	if (line1.startid<line2.startid)
		return -1;

	if (line1.startid>line2.startid)
		return 1;

	if (line1.endid<line2.endid)
		return -1;

	if (line1.endid>line2.endid)
		return 1;
	return 0;
	*/
//	anglesign=1;
	int64_t lat1;
	int64_t lat2;
	lat1 = calcLatForLon(line1,lon);
	lat2 = calcLatForLon(line2,lon);
	if (lat1 < lat2)
		return -1;
	if (lat1 > lat2)
		return 1;
	int dlat1;
	int dlon1;
	int dlat2;
	int dlon2;
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
#define TREE_WANT_DELETE
#define TREE_WANT_REMOVE
#define TREE_WANT_NEW
#define TREE_WANT_ITERATOR
#define TREE_GIVE_CMP
#include <ucw/redblack.h>

void tree_dezombification(struct tree_tree * tree, struct line_t * lines,int lineIdx, int64_t lon){
	TREE_FOR_ALL(tree,tree,iternode)
	{
		struct line_t l;
		l = lines[iternode->lineIdx];
	//	printf("%11d:(%10d,%10d)--%11d:(%10d,%10d),%d\n",l.startid,l.startlat,l.startlon,l.endid, l.endlat, l.endlon, l.isBar);
	//
		if ((l.endlon < lon)||(lineIdx==iternode->lineIdx)){
			tree_remove(tree,iternode);
			printf("Zombie roemoved");
		}
		
	}
	TREE_END_FOR;

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
			line->startlat = n2->lat*SWEEP_SCALE;
			line->endlon = n1->lon*SWEEP_SCALE;
			line->endlat = n1->lat*SWEEP_SCALE;
			line->startid = n2->id;
			line->endid = n1->id;
		} else {
			line->startlon = n1->lon*SWEEP_SCALE;
			line->startlat = n1->lat*SWEEP_SCALE;
			line->endlon = n2->lon*SWEEP_SCALE;
			line->endlat = n2->lat*SWEEP_SCALE;
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
			line->startlon = n2->lon*SWEEP_SCALE;
			line->startlat = n2->lat*SWEEP_SCALE;
			line->endlon = n1->lon*SWEEP_SCALE;
			line->endlat = n1->lat*SWEEP_SCALE;
			line->startid = n2->id;
			line->endid = n1->id;
		} else {
			line->startlon = n1->lon*SWEEP_SCALE;
			line->startlat = n1->lat*SWEEP_SCALE;
			line->endlon = n2->lon*SWEEP_SCALE;
			line->endlat = n2->lat*SWEEP_SCALE;
			line->startid = n1->id;
			line->endid = n2->id;
		}
		line->isBar = 1;
		line->broken = 0;
	}

	// Remove duplicities
	// Make queue
	int n_queue;
	n_queue = GARY_SIZE(lines)*2;
	struct event_t * queue;
	GARY_INIT_SPACE(queue,n_queue);
	GARY_PUSH(queue);
	for (int i=0;i<n_queue/2;i++){
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
	
	#define EVENT_CMP(x,y) (\
		((x).lon<(y).lon)||\
		(\
			((x).lon==(y).lon)&&\
			(\
				((x).lat<(y).lat)||\
				(\
					((x).lat==(y).lat)&&\
					(\
						(x).type<(y).type||\
						(\
							((x).type==(y).type)&&\
							(((x).dlat*(y).dlon)<((x).dlon*(y).dlat))\
						)\
					)\
				)\
			)\
		)\
	)

	HEAP_INIT(struct event_t,queue,n_queue,EVENT_CMP,HEAP_SWAP);
	struct tree_tree * tree;
	tree = malloc(sizeof(struct tree_tree));
	tree_init(tree);
	while (n_queue >0)
	{
		printf("Tree has %d nodes\n",tree->count);
		printf("Heap has %d events\n",n_queue);
		HEAP_DELETE_MIN(struct event_t,queue,n_queue,EVENT_CMP,HEAP_SWAP);
		struct event_t evt;
		printf("LineIdx: %d %d\n",evt.lineIdx, evt.type);
		evt = queue[n_queue+1];

		printf("Event: %d %d %d %d\n",evt.lon,evt.lat,evt.type,(evt.dlon==0)?-1:evt.dlat/evt.dlon);

		struct line_t line;
		line = lines[evt.lineIdx];
		if (line.startlon == line.endlon){
			printf("Special case\n");
			continue;
		}
		//struct tree_ins_t ins;
		int64_t * col;
		lon = evt.lon;
		//Debug
		//evt.lon = 50091234;
		// /Debug
		struct event_t intevt;

		//struct tree_t * item;
		switch (evt.type){
			case EVT_START:
				anglesign = 1;
				printf("EVT_START\n");
				struct tree_node_t * node;
				node = tree_find(tree,evt.lineIdx);
				if (node){
					printf("Node already present\n");
					continue;
				}
				lines[evt.lineIdx].started=1;
				node = tree_new(tree,evt.lineIdx);
				struct tree_node_t * prev;
				struct tree_node_t * next;
				prev = tree_adjacent(node,0);
				next = tree_adjacent(node,1);

				//ins = tree_insert(tree,lines,evt.lineIdx,evt.lon);
				//item = tree_find(*tree,lines,evt.lineIdx,evt.lon,1);
				//if (item==NULL){
				//	printf("Item lost\n");
				//}
				//printf("Neighs: %p %d %d\n",*tree,prev.lineIdx,next.lineIdx);
				if (prev){
					col = calcCollision(lines[evt.lineIdx],lines[prev->lineIdx]);					
					if (col){
						printf("Intersection\n");
						intevt.type = EVT_INTERSECT;
						intevt.lon = col[0];
						intevt.lat = col[1];
						intevt.lineIdx = evt.lineIdx;
						intevt.line2Idx = prev->lineIdx;
						if (lines[evt.lineIdx].isBar || lines[evt.line2Idx].isBar){
							lines[evt.lineIdx].broken = 1;
							lines[prev->lineIdx].broken = 1;
						}
						HEAP_INSERT(struct event_t, queue, n_queue, EVENT_CMP, HEAP_SWAP, intevt);
					}
					free(col);
				}
				if (next){
					col = calcCollision(lines[evt.lineIdx],lines[next->lineIdx]);
					if (col){
						printf("Intersection\n");
						intevt.type = EVT_INTERSECT;
						intevt.lon = col[0];
						intevt.lat = col[1];
						intevt.lineIdx = evt.lineIdx;
						intevt.line2Idx = next->lineIdx;
						if (lines[evt.lineIdx].isBar || lines[evt.line2Idx].isBar){
							lines[evt.lineIdx].broken = 1;
							lines[next->lineIdx].broken = 1;
						}
						HEAP_INSERT(struct event_t, queue, n_queue, EVENT_CMP, HEAP_SWAP, intevt);
					}
					free(col);
				}
				break;
			case EVT_END:
				anglesign=-1;
				printf("EVT_END\n");
				//item = tree_find(*tree,lines,evt.lineIdx,evt.lon,-1);
				//if (item==NULL){
				//	printf("Node disappeared\n");
				//}
				line = lines[evt.lineIdx];
				lines[evt.lineIdx].ended=1;
				if (!line.started){
					printf("Ended before started!\n");
					printf("%d:(%d,%d)--%d:(%d,%d),%d\n",line.startid, line.startlat, line.startlon, line.endid, line.endlat, line.endlon, line.isBar);
				} else {
					printf("Ended ok\n");
				}
				node = tree_find(tree,evt.lineIdx);
				if (node){
					tree_delete(tree,evt.lineIdx);
				}else
				{
					if (line.started){
						printf("Zobie!\n");
						tree_dezombification(tree,lines,evt.lineIdx,evt.lon);
						printf("Line1: (%d,%d)--(%d,%d)\n",line.startlon,line.startlat,line.endlon,line.endlat);

					}
				}
				break;
			case EVT_INTERSECT:
				anglesign = -1;
				printf("EVT_INT\n");
				printf("Event lon: %d lat: %d\n",evt.lon,evt.lat);
				struct line_t l1;
				struct line_t l2;
				int64_t lat1;
				int64_t lat2;

				l1 = lines[evt.lineIdx];
				l2 = lines[evt.line2Idx];
			
				lat1 = calcLatForLon(lines[evt.lineIdx],evt.lon);
				lat2 = calcLatForLon(lines[evt.line2Idx],evt.lon);
				if ((lat1!=lat2)||(lat1!=evt.lat)||(lat2!=evt.lat)){
					printf("Lat error: l1:%d, l2:%d, evt:%d \n",lat1,lat2,evt.lat);
					printf("Line1: (%d,%d)--(%d,%d)\n",l1.startlon,l1.startlat,l1.endlon,l1.endlat);
					printf("Line2: (%d,%d)--(%d,%d)\n",l2.startlon,l2.startlat,l2.endlon,l2.endlat);
				}
				node = tree_find(tree,evt.lineIdx);
				if (node){
					tree_delete(tree,evt.lineIdx);
				}{
					if (l1.started){
						printf("Zobie!\n");
						tree_dezombification(tree,lines,evt.lineIdx,evt.lon);
						printf("Line1: (%d,%d)--(%d,%d)\n",l1.startlon,l1.startlat,l1.endlon,l1.endlat);
					}
				}
				node = tree_find(tree,evt.line2Idx);
				if (node){
					tree_delete(tree,evt.line2Idx);
				}{
					if (l2.started){
						printf("Zobie!\n");
						tree_dezombification(tree,lines,evt.line2Idx,evt.lon);
						printf("Line1: (%d,%d)--(%d,%d)\n",l2.startlon,l2.startlat,l2.endlon,l2.endlat);
					}
				}

				printf("Deleted\n");
				anglesign = 1;
				if (!lines[evt.lineIdx].ended){
					node = tree_new(tree,evt.lineIdx);
					struct tree_node_t * prev;
					struct tree_node_t * next;
					prev = tree_adjacent(node,0);
					next = tree_adjacent(node,1);

					//ins = tree_insert(tree,lines,evt.lineIdx,evt.lon);
					//item = tree_find(*tree,lines,evt.lineIdx,evt.lon,1);
					//if (item==NULL){
					//	printf("Item lost\n");
					//}
					//printf("Neighs: %p %d %d\n",*tree,prev.lineIdx,next.lineIdx);
					if (prev){
						col = calcCollision(lines[evt.lineIdx],lines[prev->lineIdx]);					
						if ((col)&&(col[0]>evt.lon)){
							printf("Intersection at %d\n",col[0]);
							intevt.type = EVT_INTERSECT;
							intevt.lon = col[0];
							intevt.lat = col[1];
							intevt.lineIdx = evt.lineIdx;
							intevt.line2Idx = prev->lineIdx;
							if (lines[evt.lineIdx].isBar || lines[evt.line2Idx].isBar){
								lines[evt.lineIdx].broken = 1;
								lines[prev->lineIdx].broken = 1;
							}
							HEAP_INSERT(struct event_t, queue, n_queue, EVENT_CMP, HEAP_SWAP, intevt);
						}
						free(col);
					}
					if (next){
						col = calcCollision(lines[evt.lineIdx],lines[next->lineIdx]);
						if (col&&(col[0]>evt.lon)){
							printf("Intersection at %d\n",col[0]);
							intevt.type = EVT_INTERSECT;
							intevt.lon = col[0];
							intevt.lat = col[1];
							intevt.lineIdx = evt.lineIdx;
							intevt.line2Idx = next->lineIdx;
							if (lines[evt.lineIdx].isBar || lines[evt.line2Idx].isBar){
								lines[evt.lineIdx].broken = 1;
								lines[next->lineIdx].broken = 1;
							}
							HEAP_INSERT(struct event_t, queue, n_queue, EVENT_CMP, HEAP_SWAP, intevt);
						}
						free(col);
					}
				}
				if (!lines[evt.line2Idx].ended){
					node = tree_new(tree,evt.line2Idx);
					struct tree_node_t * prev;
					struct tree_node_t * next;
					prev = tree_adjacent(node,0);
					next = tree_adjacent(node,1);

					//ins = tree_insert(tree,lines,evt.lineIdx,evt.lon);
					//item = tree_find(*tree,lines,evt.lineIdx,evt.lon,1);
					//if (item==NULL){
					//	printf("Item lost\n");
					//}
					//printf("Neighs: %p %d %d\n",*tree,prev.lineIdx,next.lineIdx);
					if (prev){
						col = calcCollision(lines[evt.line2Idx],lines[prev->lineIdx]);					
						if (col && (col[0]>evt.lon)){
							printf("Intersection at %d\n",col[0]);
							intevt.type = EVT_INTERSECT;
							intevt.lon = col[0];
							intevt.lat = col[1];
							intevt.lineIdx = evt.line2Idx;
							intevt.line2Idx = prev->lineIdx;
							if (lines[evt.lineIdx].isBar || lines[evt.line2Idx].isBar){
								lines[evt.line2Idx].broken = 1;
								lines[prev->lineIdx].broken = 1;
							}
							HEAP_INSERT(struct event_t, queue, n_queue, EVENT_CMP, HEAP_SWAP, intevt);
						}
						free(col);
					}
					if (next){
						col = calcCollision(lines[evt.line2Idx],lines[next->lineIdx]);
						if (col&&(col[0]>evt.lon)){
							printf("Intersection at %d\n",col[0]);
							intevt.type = EVT_INTERSECT;
							intevt.lon = col[0];
							intevt.lat = col[1];
							intevt.lineIdx = evt.line2Idx;
							intevt.line2Idx = next->lineIdx;
							if (lines[evt.lineIdx].isBar || lines[evt.line2Idx].isBar){
								lines[evt.line2Idx].broken = 1;
								lines[next->lineIdx].broken = 1;
							}
							HEAP_INSERT(struct event_t, queue, n_queue, EVENT_CMP, HEAP_SWAP, intevt);
						}
						free(col);
					}

				}
				break;
		}

		TREE_FOR_ALL(tree,tree,iternode)
		{
			struct line_t l;
			l = lines[iternode->lineIdx];
		//	printf("%11d:(%10d,%10d)--%11d:(%10d,%10d),%d\n",l.startid,l.startlat,l.startlon,l.endid, l.endlat, l.endlon, l.isBar);
			
		}
		TREE_END_FOR;
	//	printf("Lon:%d Lat:%d\n",queue[n_queue+1].lon,queue[n_queue+1].lat);
	}
	printf("Check\n");
	for (int i=0;i<GARY_SIZE(lines);i++){
		if (lines[i].started && !lines[i].ended){
			struct line_t l;
			l = lines[i];
			printf("%d:(%d,%d)--%d:(%d,%d),%d\n",l.startid,l.startlat,l.startlon,l.endid, l.endlat, l.endlon, l.isBar);
		}
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
		if (!lines[i].broken){
			way->type = OBJTYPE__DIRECT;
			printf("Writing way %d\n",way->type);
		}
		else
			{
			way->type = OBJTYPE__DIRECT_BAD;
			printf("Writing bad way %d\n",way->type);
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
	struct line_t * lines;
	lines = findDirectWays(map,candidates,graph.barGraph);
	map = addDirectToMap(lines,map);
	mapToPBF(map,pbmap);
	checkWayTypes(pbmap);
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
