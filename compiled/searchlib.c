#include <ucw/lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <inttypes.h>
#include <time.h>
#include <err.h>

#include <yaml.h>
#include <proj_api.h>
#include <ucw/heap.h>
#include <ucw/gary.h>

#include "include/graph.pb-c.h"
#include "include/result.pb-c.h"

#include "searchlib.h"
#include "hashes.h"
#include "utils.h"
#include "postprocess.h"

#include "libraptor.h"

// Loading 
struct config_t parseConfigFile(char * filename){
	struct config_t conf;
	conf.desc = objtype__descriptor;
	conf.maxvalue = conf.desc.values[conf.desc.n_values-1].value;
	conf.speeds = malloc(sizeof(double)*(conf.maxvalue+1));
	conf.ratios = malloc(sizeof(double)*(conf.maxvalue+1));
	conf.penalties = malloc(sizeof(double)*(conf.maxvalue+1));
	for (int i=0;i<conf.maxvalue+1;i++){
		conf.speeds[i]=-1;
		conf.ratios[i]=-1;
		conf.penalties[i]=1;
	}
	

	yaml_parser_t parser;
	yaml_document_t document;

	yaml_parser_initialize(&parser);

	FILE * IN;
	IN = fopen(filename,"r");
	if (IN==NULL){
		printf("Config file opening error\n");	
		return conf;
	}
	yaml_parser_set_input_file(&parser,IN);
	

	yaml_parser_load(&parser,&document);
	fclose(IN);

	yaml_node_t * node;
	yaml_node_pair_t * section;

	node = yaml_document_get_root_node(&document);

	if (node->type == YAML_MAPPING_NODE){
		for (section=node->data.mapping.pairs.start;
				section < node->data.mapping.pairs.top; section++){

			yaml_node_t * key;
			key = yaml_document_get_node(&document,section->key);
			if (strcmp((char *)key->data.scalar.value,"speeds")==0){
				//printf("Parsing speeds\n");
				yaml_node_t * speedsMap;
				speedsMap = yaml_document_get_node(&document,section->value);
				if (speedsMap->type != YAML_MAPPING_NODE){
					printf("Speeds are not mapping\n");
					break;
				}
				yaml_node_pair_t * pair;
				for (pair=speedsMap->data.mapping.pairs.start;
						pair < speedsMap->data.mapping.pairs.top;pair++){
					yaml_node_t * key;
					yaml_node_t * value;
					key = yaml_document_get_node(&document,pair->key);
					value = yaml_document_get_node(&document,pair->value);
					for (int i=0;i<conf.desc.n_values;i++){
						if (strcmp((char *)key->data.scalar.value,conf.desc.values[i].name)==0){
							conf.speeds[conf.desc.values[i].value]=atof((char *)value->data.scalar.value);
							break;
						}
					}
				}
				
			} else if (strcmp((char *)key->data.scalar.value,"ratios")==0){
				//printf("Parsing ratios\n");
				yaml_node_t * ratiosMap;
				ratiosMap = yaml_document_get_node(&document,section->value);
				if (ratiosMap->type != YAML_MAPPING_NODE){
					printf("Speeds are not mapping\n");
					break;
				}
				yaml_node_pair_t * pair;
				for (pair=ratiosMap->data.mapping.pairs.start;
						pair < ratiosMap->data.mapping.pairs.top;pair++){
					yaml_node_t * key;
					yaml_node_t * value;
					key = yaml_document_get_node(&document,pair->key);
					value = yaml_document_get_node(&document,pair->value);
					for (int i=0;i<conf.desc.n_values;i++){
						if (strcmp((char *)key->data.scalar.value,conf.desc.values[i].name)==0){
							conf.ratios[conf.desc.values[i].value]=atof((char *)value->data.scalar.value);
							break;
						}
					}
				}
			
			} else if (strcmp((char *)key->data.scalar.value,"penalties")==0){
				//printf("Parsing speeds\n");
				yaml_node_t * penaltyMap;
				penaltyMap = yaml_document_get_node(&document,section->value);
				if (penaltyMap->type != YAML_MAPPING_NODE){
					printf("Speeds are not mapping\n");
					break;
				}
				yaml_node_pair_t * pair;
				for (pair=penaltyMap->data.mapping.pairs.start;
						pair < penaltyMap->data.mapping.pairs.top;pair++){
					yaml_node_t * key;
					yaml_node_t * value;
					key = yaml_document_get_node(&document,pair->key);
					value = yaml_document_get_node(&document,pair->value);
					for (int i=0;i<conf.desc.n_values;i++){
						if (strcmp((char *)key->data.scalar.value,conf.desc.values[i].name)==0){
							conf.penalties[conf.desc.values[i].value]=atof((char *)value->data.scalar.value);
							break;
						}
					}
				}
				
			} else if (strcmp((char *)key->data.scalar.value,"heights")==0){
				//printf("Parsing heights\n");
				yaml_node_t * heightsMap;
				heightsMap = yaml_document_get_node(&document,section->value);
				if (heightsMap->type != YAML_MAPPING_NODE){
					printf("Height are not mapping\n");
					break;
				}
				yaml_node_pair_t * pair;
				for (pair=heightsMap->data.mapping.pairs.start;
						pair < heightsMap->data.mapping.pairs.top;pair++){
					yaml_node_t * key;
					yaml_node_t * value;
					key = yaml_document_get_node(&document,pair->key);
					value = yaml_document_get_node(&document,pair->value);
					if (strcmp((char *)key->data.scalar.value,"upscale")==0){
						conf.upscale = atof((char *)value->data.scalar.value);	
					} else if (strcmp((char *)key->data.scalar.value,"downscale")==0){
						conf.downscale = atof((char *)value->data.scalar.value);	
					} else if (strcmp((char *)key->data.scalar.value,"maxslope")==0){
//						conf.maxslope = atof((char *)value->data.scalar.value);
					} else if (strcmp((char *)key->data.scalar.value,"upslopescale")==0){
//  					conf.upslopescale= atof((char *)value->data.scalar.value);
					} else if (strcmp((char *)key->data.scalar.value,"downslopescale")==0){
//						conf.downslopescale = atof((char *)value->data.scalar.value);
					} 
				}
				
			} else
			{
				printf("Unsupported section: %s\n",key->data.scalar.value);
			}
		}		
	}
	else {
		printf("Unsupported format of configuration\n");
	}
	
	yaml_document_delete(&document);		
	yaml_parser_delete(&parser);

	int wayIdx;
	wayIdx = -1;
	for (int i=0;i<conf.desc.n_values;i++){
		if (strcmp(conf.desc.values[i].name,"WAY")==0){
			wayIdx = i;
			if (conf.speeds[wayIdx]==-1){
				printf("WAY does not have speed\n");
			}
			break;
		}
	}
	for (int i=0;i<(conf.maxvalue+1);i++){
		if (conf.speeds[i]==-1){
			if (conf.ratios[i]==-1)
				conf.speeds[i]=conf.speeds[wayIdx];
			else
				conf.speeds[i]=conf.speeds[wayIdx]*conf.ratios[i];
		}
		conf.speeds[i]/=3.6;
	}
	return conf;
}

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

#define DIJ_CMP(x,y) (q->dijArray[x].time < q->dijArray[y].time)

#define MMHEAP_INSERT(elem) \
	HEAP_INSERT(int,q->heap,q->n_heap,DIJ_CMP,HEAP_SWAP,(elem))
#define MMHEAP_DELETE_MIN() \
	HEAP_DELETE_MIN(int,q->heap,q->n_heap,DIJ_CMP,HEAP_SWAP)
#define MMHEAP_DECREASE(elem,val) \
	HEAP_DECREASE(q->heap,q->n_heap,DIJ_CMP,HEAP_SWAP,(elem),(val))

void prepareMMNode(struct mmdijnode_t * node,int idx,int fromDijIdx,int fromEdgeIdx,Objtype fromEdgeType,time_t time,time_t departed,double penalty){
	node->idx = idx;
	node->fromIdx = fromDijIdx;
	node->fromEdgeIdx = fromEdgeIdx;
	node->fromEdgeType = fromEdgeType;
	node->time = time;
	node->departed = departed;
	node->penalty = penalty;
	node->reached = 1;
	node->completed = 0;
	node->majorized = 0;
} 

void addFirstNodeToQueue(struct mmqueue_t * q,int idx,time_t time){
	struct mmdijnode_t * node;
	node = GARY_PUSH(q->dijArray);

	prepareMMNode(node,idx,-1,-1,ROUTE_TYPE_NONE,time,0,0);
	
	// Add to vertlut
	int * vertlutItem;
	vertlutItem = GARY_PUSH(q->vertlut[idx]);
	*vertlutItem = node-q->dijArray;

	// Add to heap
	GARY_PUSH(q->heap);
	MMHEAP_INSERT(node - q->dijArray);
	q->vert = q->dijArray+q->heap[0];
}
void addNodeToQueue(struct mmqueue_t * q,
		int idx,
		int fromEdgeIdx,
		char fromEdgeType,
		time_t time,
		time_t departed,
		double penalty){
	// Check if it is not majorized by another node already in queue
	// Check if it majorizes some nodes in queue and mark them as majorized
	for (int j=0;j<GARY_SIZE(q->vertlut[idx]);j++){
		struct mmdijnode_t * anode;
		anode = q->dijArray + q->vertlut[idx][j];
		if ((time >= anode->time)&&(penalty >= anode->penalty)){
			return;
		}
		if ((time <= anode->time)&&(penalty <= anode->penalty)){
			anode->majorized=1;
		}
	}

	struct mmdijnode_t * node;

	// Add to dijArray
	int vertIdx;	// Preserve vert pointer through reallocation
	vertIdx = q->vert-q->dijArray;
	node = GARY_PUSH(q->dijArray);
	q->vert = q->dijArray+vertIdx;

	prepareMMNode(node,idx,vertIdx,fromEdgeIdx,fromEdgeType,time,departed,penalty);
	
	// Add to vertlut
	int * vertlutItem;
	vertlutItem = GARY_PUSH(q->vertlut[idx]);
	*vertlutItem = node-q->dijArray;

	// Add to heap
	GARY_PUSH(q->heap);
	MMHEAP_INSERT(node - q->dijArray);
}

struct mmdijnode_t * getQueueMin(struct mmqueue_t * q){
	if (q->n_heap==0){
		err(2,"Heap empty");
	}

	MMHEAP_DELETE_MIN();
	int vIdx;
	vIdx = q->heap[q->n_heap+1];
	GARY_POP(q->heap);
	q->vert = q->dijArray + vIdx;

	if (!q->vert->reached){
		err(1,"Found unreached vertex, exitting\n");
	}

	if (q->vert->majorized){
		return NULL;	
	}

	return q->vert;
}

struct mmqueue_t * createMMQueue(Graph__Graph * graph){
	
	struct mmqueue_t * q;
	q = malloc(sizeof(struct mmqueue_t));
	q->n_heap = 0;
	GARY_INIT_SPACE(q->heap,graph->n_vertices/3);
	GARY_PUSH(q->heap);	// First item is unused

	q->vertlut = calloc(sizeof(int*),graph->n_vertices);
	for (int i=0;i<graph->n_vertices;i++){
		GARY_INIT(q->vertlut[i],0);	
	}

	GARY_INIT_SPACE(q->dijArray,graph->n_vertices/3);
	return q;
	
} 
void freeMMQueue(struct mmqueue_t * queue,int n_vertices){
	GARY_FREE(queue->heap);
	for (int i=0;i<n_vertices;i++){
		GARY_FREE(queue->vertlut[i]);	
	}
	free(queue->vertlut);
	GARY_FREE(queue->dijArray);
	free(queue);
}

struct mmqueue_t * findMMWay(struct search_data_t data, int fromIdx, int toIdx,time_t starttime){
	Graph__Graph * graph;
	graph = data.graph;


	struct timetable * newtt;
	time_t date;
	date = starttime - (starttime%(24*3600));
	newtt = gen_tt_for_date(data.timetable,date,NULL);

	uint64_t * stopslut;
	int maxraptorid;
	maxraptorid = 0;
	for (int i=0;i < graph->n_stops;i++){
		if (graph->stops[i]->raptor_id > maxraptorid){
			maxraptorid = graph->stops[i]->raptor_id;	
		}
	}
	stopslut = calloc(sizeof(uint64_t),maxraptorid+1);
	for (int i=0;i < graph->n_stops;i++){
		stopslut[graph->stops[i]->raptor_id] = graph->stops[i]->osmid;
	}
	
	struct nodeways_t * nodeways;
	nodeways = data.nodeWays;
	
	struct mmqueue_t * queue;
	queue = createMMQueue(graph);

	addFirstNodeToQueue(queue,fromIdx,starttime);

	int best_time;
	best_time = -1;

	while(queue->n_heap > 0){
		if (getQueueMin(queue) == NULL){
			continue;
		}

		if (queue->n_heap%500==0){
			printf("Heap: %d\n",queue->n_heap);
		}

		//printf("Vertex: %d, time: %d, penalty: %f\n",vert->idx,vert->time,vert->penalty);
		//printf("DijArray: %d\n",GARY_SIZE(dijArray));

		// Process public transport connections
		struct osmId2sIdxNode * stopsNode;
		int64_t osmid;
		osmid = graph->vertices[queue->vert->idx]->osmid;
		stopsNode = osmId2sIdx_find2(osmid);
		if (stopsNode != NULL){
			char * timestr;
			timestr = prt_time((queue->vert->time)%(24*3600));
			printf("Stop %d found at %s\n",stopsNode->idx,timestr);
			free(timestr);

			struct stop_conns * conns;
			conns = search_stop_conns(newtt,graph->stops[stopsNode->idx]->raptor_id,queue->vert->time%(24*3600));
			for (int ridx = 0; ridx < conns->n_routes; ridx++){
				struct stop_route * r;
				r = conns->routes+ridx;
				timestr = prt_time(r->departure);
				printf("Departure at %s\n",timestr);
				free(timestr);
				for (int sidx = 0;sidx < r->n_stops;sidx++){
					int64_t osmid;
					time_t arrival;
					double penalty;

					osmid = stopslut[r->stops[sidx].to];
					
					// Stop not in map area
					if(osmid == 0){
						continue;
					}

					arrival = r->stops[sidx].arrival;
					penalty = 0;
					
					struct nodesIdxNode * nd;
					nd = nodesIdx_find2(osmid);
					if (nd == NULL){
						printf("No matching node for node id %lld\n",osmid);
						continue;
					}
					addNodeToQueue(queue,nd->idx,r->id,ROUTE_TYPE_PT,arrival,r->departure,queue->vert->penalty + penalty);
				}
					
			}
			free_conns(conns);
		}

		// Process walk connections	
		for (int i=0;i<nodeways[queue->vert->idx].n_ways;i++){	
			Graph__Edge * way;
			way = graph->edges[nodeways[queue->vert->idx].ways[i]];
			int wayend;
			if (way->vfrom == queue->vert->idx){
				wayend = way->vto;	
			} else {
				wayend = way->vfrom;
			}

			double penalty;
			penalty = calcWeight(graph,data.conf,way);

			int time;
			time = calcTime(graph,data.conf,way);

			addNodeToQueue(queue,wayend,nodeways[queue->vert->idx].ways[i],ROUTE_TYPE_WALK,
				queue->vert->time + time,0,queue->vert->penalty + penalty);
		}


		queue->vert->completed = true;
		if (best_time == -1 && queue->vert->idx == toIdx){
			printf("Found path");
			best_time = queue->vert->time-starttime;
		}

		// Break if connection is too long
		if ((best_time != -1) && 
			((queue->vert->time-starttime) > 1.5*best_time)){
			printf("Path too long, exitting.");
			
			break;
		}
	}
	free_tt(newtt);
	return queue;


				
		
}
#undef DIJ_CMP
#undef DIJ_SWAP

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

	data->nodeWays = makeNodeWays(data->graph);
	nodesIdx_refresh(data->graph->n_vertices,data->graph->vertices);
	sId2sIdx_refresh(data->graph->n_stops,data->graph->stops);
	osmId2sIdx_refresh(data->graph->n_stops,data->graph->stops);
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
	queue = findMMWay(*data,fromIdx,toIdx,time(NULL));
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


