#include <ucw/lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <inttypes.h>
#include <time.h>
#include <err.h>

#include <proj_api.h>
#include <ucw/heap.h>
#include <ucw/gary.h>

#include "include/graph.pb-c.h"
#include "include/result.pb-c.h"

#include "searchlib.h"
#include "hashes.h"
#include "utils.h"
#include "postprocess.h"
#include "mmqueue.h"
#include "config.h"
#include "penalty.h"

#include "libraptor.h"

#define DAY_SECS (24*3600)


struct pbf_data_t readFile(char * filename){
	FILE * IN;
	IN = fopen(filename,"r");
	struct pbf_data_t data;
	data.len = 0;
	if (IN==NULL){
		printf("File opening error (%s)\n",filename);	
		return data;
	}
	fseek(IN,0,SEEK_END);

	data.len = ftell(IN);
	fseek(IN,0,SEEK_SET);
	data.data = (uint8_t *)malloc(data.len);
	size_t read;
	read = fread(data.data,1,data.len,IN);
	if (read!=data.len){
		printf("Not read all file %s\n",filename);
		data.len = 0;
		free(data.data);
	}
	fclose(IN);
	return data;
}

void freeMap(Graph__Graph * graph){
	graph__graph__free_unpacked(graph,NULL);	
}

Graph__Graph * loadMap(char * filename){
	struct pbf_data_t data;
	data = readFile(filename);
	if (data.len == 0){
		return NULL;
	}	
	Graph__Graph * graph;
	graph = graph__graph__unpack(NULL,data.len,data.data);
	free(data.data);

	return graph;
}


void calcDistances(Graph__Graph * graph){
	for (int i=0;i<graph->n_edges;i++){
		Graph__Edge * edge;
		edge = graph->edges[i];
		if (edge->vfrom >= graph->n_vertices){
			printf("Wrong point %lld\n",edge->vfrom);	
			edge->dist = DBL_MAX;
			continue;
		}
		if (edge->vto >= graph->n_vertices){
			printf("Wrong point %lld\n",edge->vto);	
			edge->dist = DBL_MAX;
			continue;
		}
		double dlon;
		double dlat;
		dlon = graph->vertices[edge->vto]->lon-graph->vertices[edge->vfrom]->lon;
		dlat = graph->vertices[edge->vto]->lat-graph->vertices[edge->vfrom]->lat;

		edge->dist = sqrt(dlon*dlon+dlat*dlat);
		if (edge->dist > 100){
			printf("Dist too long: dlat: %f, dlon: %f\n",dlat,dlon);
		}
	}
}

void freeNodeWays(struct nodeways_t * nodeways,int n_vertices){
	for (int i=0;i<n_vertices;i++){
		free(nodeways[i].ways);	
	}
	free(nodeways);
}

struct nodeways_t * makeNodeWays(Graph__Graph * graph){
	struct nodeways_t * nodeways;
	nodeways = malloc(sizeof(struct nodeways_t)*graph->n_vertices);
	for (int i=0;i<graph->n_vertices;i++){
		nodeways[i].n_ways=0;
	}
	for (int i=0;i<graph->n_edges;i++){
		nodeways[graph->edges[i]->vfrom].n_ways++;
		nodeways[graph->edges[i]->vto].n_ways++;
	}
	for (int i=0;i<graph->n_vertices;i++){
		nodeways[i].ways = malloc(sizeof(int)*nodeways[i].n_ways);
		nodeways[i].n_ways = 0;
	}
	for (int i=0;i<graph->n_edges;i++){
		int vfIdx;
		int vtIdx;
		vfIdx = graph->edges[i]->vfrom;
		vtIdx = graph->edges[i]->vto;
		nodeways[vfIdx].ways[nodeways[vfIdx].n_ways]= i;
		nodeways[vfIdx].n_ways++;
		nodeways[vtIdx].ways[nodeways[vtIdx].n_ways]= i;
		nodeways[vtIdx].n_ways++;
	}
	return nodeways;
}

int findNearestVertex(Graph__Graph * graph, double lon, double lat){
	double minDist;
	int minIdx;
	minDist = DBL_MAX;
	minIdx = -1;
	for (int i=0;i<graph->n_vertices;i++){
		double dist;
		double dlon;
		double dlat;
		dlon = graph->vertices[i]->lon-lon;
		dlat = graph->vertices[i]->lat-lat;
	
		dist = dlon*dlon+dlat*dlat;
		if (dist < minDist){
			minDist = dist;
			minIdx = i;
		}
	}
	printf("Min dist: %f\n",sqrt(minDist));
	printf("Point %d\n",minIdx);
//	printf("%f\n",graph->vertices[minIdx]->lon);
//	printf("%f\n",graph->vertices[minIdx]->lat);
	return minIdx;
}


struct dijnode_t *  prepareDijkstra(Graph__Graph * graph){
	struct dijnode_t * dijArray;
	dijArray = malloc(sizeof(struct dijnode_t)*graph->n_vertices);
	for (int i=0;i<graph->n_vertices;i++){
		dijArray[i].fromIdx=-1;
		dijArray[i].fromEdgeIdx=-1;
		dijArray[i].reached=false;
		dijArray[i].completed=false;
		dijArray[i].dist = DBL_MAX;
	}
	return dijArray;
}


struct mmqueue_t * findMMWay(struct search_data_t data, int fromIdx, int toIdx,time_t starttime){
	Graph__Graph * graph;
	graph = data.graph;


	struct timetable * curtt;
	struct timetable * nexttt;
	time_t date;
	date = starttime - (starttime%DAY_SECS);
	curtt = gen_tt_for_date(data.timetable,date,NULL);
	nexttt = gen_tt_for_date(data.timetable,date+DAY_SECS,curtt);

	uint64_t curdate = date;
	
	struct nodeways_t * nodeways;
	nodeways = data.nodeWays;
	
	struct mmqueue_t * queue;
	queue = createMMQueue(graph);

	addFirstNodeToQueue(queue,graph->vertices[fromIdx],NULL,starttime); //TODO: pridat i zastavku, pokud je

	int best_time;
	best_time = -1;

	while(queue->n_heap > 0){
		if (getQueueMin(queue) == NULL){
			continue;
		}


		/*if (queue->n_heap%500==0){
			printf("Heap: %d\n",queue->n_heap);
		}*/

		/*char * strtime;
		strtime = prt_time(queue->vert->arrival%(24*3600));
		//printf("Vertex: %d, time: %s, penalty: %f\n",queue->vert->idx,strtime,queue->vert->penalty);
		free(strtime);*/
		//printf("DijArray: %d\n",GARY_SIZE(dijArray));

		// Process public transport connections
		if (queue->vert->stop != NULL){
			/*char * timestr;
			timestr = prt_time((queue->vert->arrival)%(24*3600));
			printf("Stop %d found at %s\n",stopsNode->idx,timestr);
			free(timestr);*/

			struct stop_conns * conns;
		//	printf("%p\n",queue);
		//	printf("Id: \n",queue->vert->stop->id);
			conns = search_stop_conns(curtt,queue->vert->stop->id,queue->vert->arrival-date);
			for (int ridx = 0; ridx < conns->n_routes; ridx++){
				struct stop_route * r;
				r = conns->routes+ridx;
				//timestr = prt_time(r->departure);
				//printf("Departure at %s\n",timestr);
				//free(timestr);
				for (int sidx = 0;sidx < r->n_stops;sidx++){
					int64_t osmid;
					time_t arrival;

					osmid = data.raptor2osm[r->stops[sidx].to->id];
					
					// Stop not in map area
					if(osmid == 0){
						continue;
					}

					arrival = r->stops[sidx].arrival;
					double penalty;
					penalty = 0;
					
					penalty += calcTransportPenalty(graph,data.conf,r,arrival);
					
					struct nodesIdxNode * nd;
					nd = nodesIdx_find2(osmid);
					if (nd == NULL){
						printf("No matching node for node id %lld\n",osmid);
						continue;
					}
					struct edge_t * e;
					e = malloc(sizeof(struct edge_t));
					e->edge_type = EDGE_TYPE_PT;
					e->ptedge = malloc(sizeof(struct ptedge_t));
					e->ptedge->route = r->pbroute;
					e->ptedge->departure = r->departure;
					// TODO: Add stop properties

					penalty += calcPointPenalty(graph,data.conf,graph->vertices[nd->idx]);
					int wait;
					wait = r->departure - (queue->vert->arrival-date);
					penalty += calcChangePenalty(graph,data.conf,e->ptedge,queue->vert,wait);	
					if (penalty < PENALTY_INFINITY){ 
						addNodeToQueue(queue,queue->vert,graph->vertices[nd->idx],r->stops[sidx].to,e,date+arrival,queue->vert->penalty + penalty);
					} else {
						free(e);
					}
				}
					
			}
			free_conns(conns);
			// If after midnight, process connections for the next day
			if (queue->vert->arrival - date > DAY_SECS){
				conns = search_stop_conns(nexttt,queue->vert->stop->id,queue->vert->arrival%DAY_SECS);
				for (int ridx = 0; ridx < conns->n_routes; ridx++){
					struct stop_route * r;
					r = conns->routes+ridx;
					//timestr = prt_time(r->departure);
					//printf("Departure at %s\n",timestr);
					//free(timestr);
					for (int sidx = 0;sidx < r->n_stops;sidx++){
						int64_t osmid;
						time_t arrival;

						osmid = data.raptor2osm[r->stops[sidx].to->id];
						
						// Stop not in map area
						if(osmid == 0){
							continue;
						}

						arrival = r->stops[sidx].arrival;
						double penalty;
						penalty = 0;
						
						penalty += calcTransportPenalty(graph,data.conf,r,arrival);
						
						struct nodesIdxNode * nd;
						nd = nodesIdx_find2(osmid);
						if (nd == NULL){
							printf("No matching node for node id %lld\n",osmid);
							continue;
						}
						struct edge_t * e;
						e = malloc(sizeof(struct edge_t));
						e->edge_type = EDGE_TYPE_PT;
						e->ptedge = malloc(sizeof(struct ptedge_t));
						e->ptedge->route = r->pbroute;
						e->ptedge->departure = r->departure;
						// TODO: Add stop properties

						penalty += calcPointPenalty(graph,data.conf,graph->vertices[nd->idx]);
						int wait;
						wait = r->departure - (queue->vert->arrival%DAY_SECS);
						penalty += calcChangePenalty(graph,data.conf,e->ptedge,queue->vert,wait);	
						addNodeToQueue(queue,queue->vert,graph->vertices[nd->idx],r->stops[sidx].to,e,
							date+DAY_SECS+arrival,queue->vert->penalty + penalty);
					}
						
				}
				free_conns(conns);
			}
		}

		// Process walk connections	
		for (int i=0;i<nodeways[queue->vert->osmvert->idx].n_ways;i++){	
			Graph__Edge * way;
			way = graph->edges[nodeways[queue->vert->osmvert->idx].ways[i]];
			int wayend;
			if (way->vfrom == queue->vert->osmvert->idx){
				wayend = way->vto;	
			} else {
				wayend = way->vfrom;
			}
			double penalty;
			penalty = 0;
			
			penalty += calcPointPenalty(graph,data.conf,graph->vertices[wayend]);
			penalty += calcWalkPenalty(graph,data.conf,way);
			//penalty += calcChangePenalty(graph,data.conf,q->vert->fromEdgeType,way);	


			int time;
			time = calcTime(graph,data.conf,way);


			struct edge_t * e;
			e = malloc(sizeof(struct edge_t));
			e->edge_type = EDGE_TYPE_WALK;
			e->osmedge = way;

			Stop * stop;
			if (graph->vertices[wayend]->has_stop_id){
				stop = data.timetable->stops[graph->vertices[wayend]->stop_id];		
			}else{
				stop = NULL;
			}
			/*printf("queue: %p\n",queue);
			
			if (stop != NULL){
				printf("stop id: %d\n",stop->id);			
			}*/
			addNodeToQueue(queue,queue->vert,graph->vertices[wayend],stop,e,queue->vert->arrival + time,queue->vert->penalty + penalty);

		}


		queue->vert->completed = true;
		if (best_time == -1 && queue->vert->osmvert->idx == toIdx){
			printf("Found path");
			best_time = queue->vert->arrival-starttime;
		}

		// Break if connection is too long
		if ((best_time != -1) && 
			((queue->vert->arrival-starttime) > 1.5*best_time)){
			printf("Path too long, exitting.");
			
			break;
		}
	}

	free_tt(curtt);
	return queue;


				
		
}
#undef DIJ_CMP
#undef DIJ_SWAP

void freeData(struct search_data_t * data){
	free_timetable(data->timetable);
	pj_free(data->pj_wgs84);
	pj_free(data->pj_utm);
	freeNodeWays(data->nodeWays,data->graph->n_vertices);
	freeMap(data->graph);
	free(data->raptor2osm);
	free(data->raptor2idx);

	
	free(data);	
}

struct search_data_t * prepareData(char * configName, char * dataName, char * timetableName){
	struct search_data_t * data;
	data = malloc(sizeof(struct search_data_t));
	data->pj_wgs84 = pj_init_plus("+proj=longlat +datum=WGS84 +no_defs");
	data->pj_utm = pj_init_plus("+proj=utm +zone=33 +ellps=WGS84 +units=m +no_defs");
	data->conf = parseConfigFile(configName);
	data->graph = loadMap(dataName);
	if (data->graph==NULL){
		err(1,"Error loading map\n");	
	}
	data->timetable = load_timetable(timetableName);
	if (data->timetable == NULL){
		err(1,"Error loading timetable");
	}

	int maxraptorid;
	maxraptorid = 0;
	for (int i=0;i < data->graph->n_stops;i++){
		if (data->graph->stops[i]->raptor_id > maxraptorid){
			maxraptorid = data->graph->stops[i]->raptor_id;	
		}
	}
	data->raptor2osm = calloc(sizeof(uint64_t),maxraptorid+1);
	data->raptor2idx = calloc(sizeof(uint64_t),maxraptorid+1);
	for (int i=0;i < data->graph->n_stops;i++){
		data->raptor2osm[data->graph->stops[i]->raptor_id] = data->graph->stops[i]->osmid;
		data->raptor2idx[data->graph->stops[i]->raptor_id] = i;
	}
	
	data->nodeWays = makeNodeWays(data->graph);
	nodesIdx_refresh(data->graph->n_vertices,data->graph->vertices);
	sId2sIdx_refresh(data->graph->n_stops,data->graph->stops);
	calcDistances(data->graph);
	return data;
}

/*
struct search_route_t findTransfer(struct search_data_t * data, char * from, char * to){
	int fromIdx;
	struct sId2sIdxNode * stopsNode;
	struct search_route_t result;
	stopsNode = sId2sIdx_find2(from);
	if (stopsNode == NULL){
		printf("Stop %s not found\n",from);
		result.n_points=0;
		return result;
	}
	fromIdx = stopsNode->idx;

	stopsNode = sId2sIdx_find2(to);
	if (stopsNode == NULL){
		printf("Stop %s not found\n",to);
		result.n_points=0;
		return result;
	}
	int toIdx;
	toIdx = stopsNode->idx;

	printf("Searching from %lld(%lld,%lld,%lld) to %lld(%lld,%lld,%lld)\n",data->graph->vertices[fromIdx]->osmid,
			data->graph->vertices[fromIdx]->lat,
			data->graph->vertices[fromIdx]->lon,
			data->graph->vertices[fromIdx]->height,
			data->graph->vertices[toIdx]->osmid,
			data->graph->vertices[toIdx]->lat,
			data->graph->vertices[toIdx]->lon,
			data->graph->vertices[toIdx]->height
			);

	struct dijnode_t * dijArray;
	dijArray = prepareDijkstra(data->graph);

	findWay(*data, dijArray,fromIdx, toIdx);
	int n_points;
	result.points = resultsToArray(*data,dijArray,fromIdx,toIdx,&n_points);
	result.n_points = n_points;
	result.dist = 0;
	result.time = 0;
	if (result.n_points == 0){
		return result;
	}
	int idx;
	idx = fromIdx;
	while(idx != toIdx){
		result.dist+=data->graph->edges[dijArray[idx].fromEdgeIdx]->dist;
		result.time+=calcTime(data->graph,data->conf,data->graph->edges[dijArray[idx].fromEdgeIdx]);
		idx = dijArray[idx].fromIdx;
	}
	free(dijArray);
	return result;	

}*/

struct pbf_data_t findPath(struct search_data_t * data,double fromLat, double fromLon, double toLat, double toLon){
	wgs2utm(*data,&fromLon,&fromLat);
	int fromIdx;
	fromIdx = findNearestVertex(data->graph,fromLon,fromLat);

	wgs2utm(*data,&toLon,&toLat);
	int toIdx;
	toIdx = findNearestVertex(data->graph,toLon,toLat);

/*	printf("Searching from %lld(%f,%f,%d) to %lld(%f,%f,%d)\n",data->graph->vertices[fromIdx]->osmid,
			data->graph->vertices[fromIdx]->lat,
			data->graph->vertices[fromIdx]->lon,
			data->graph->vertices[fromIdx]->height,
			data->graph->vertices[toIdx]->osmid,
			data->graph->vertices[toIdx]->lat,
			data->graph->vertices[toIdx]->lon,
			data->graph->vertices[toIdx]->height
			);
*/

	//findWay(*data, dijArray,fromIdx, toIdx);

	struct mmqueue_t * queue;
	queue = findMMWay(*data,fromIdx,toIdx,time(NULL)+tz_offset_second(time(NULL)));
	printf("Equivalences: \n");
	for (int i=0;i< GARY_SIZE(queue->vertlut[toIdx]);i++){
		equivWays(queue->vertlut[toIdx][0],queue->vertlut[toIdx][i]);
			
	}
	struct search_result_t result;
	result = processFoundMMRoutes(*data,queue,fromIdx,toIdx);
	writeGPXForResult(&result);
	printMMRoutes(data,&result);

	struct pbf_data_t pbfRes;
	pbfRes = generatePBF(&result);	
	
	freeMMQueue(queue,data->graph->n_vertices);
	return pbfRes;
	/*
	printf("Found\n");
	struct search_route_t result;
	int n_points;
	result.points = resultsToArray(*data,dijArray,fromIdx,toIdx,&n_points);
	result.n_points = n_points;
	result.dist = 0;
	result.time = 0;
	if (result.n_points == 0){
		return result;
	}
	int idx;
	idx = fromIdx;
	while(idx != toIdx){
		result.dist+=data->graph->edges[dijArray[idx].fromEdgeIdx]->dist;
		result.time+=calcTime(data->graph,data->conf,data->graph->edges[dijArray[idx].fromEdgeIdx]);
		idx = dijArray[idx].fromIdx;
	}
	free(dijArray);
	return result;	
	*/
}


