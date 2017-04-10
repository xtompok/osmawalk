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
#include "writegpx.h"
#include "hashes.c"

#include "libraptor.h"

// Utils
int utm2wgs(struct search_data_t data,double * lon, double * lat){
	int res;
	res= pj_transform(data.pj_utm,data.pj_wgs84,1,1,lon,lat,NULL);
	*lon = (*lon * 180)/M_PI;
	*lat = (*lat * 180)/M_PI;
	return res;
}
int wgs2utm(struct search_data_t data,double * lon, double * lat){
	*lon = (*lon/180)*M_PI;
	*lat = (*lat/180)*M_PI;
	return pj_transform(data.pj_wgs84,data.pj_utm,1,1,lon,lat,NULL);
}

double calcWeight(Graph__Graph * graph, struct config_t conf, Graph__Edge * edge){
	return calcTime(graph,conf,edge)*conf.penalties[edge->type];
}

double calcTime(Graph__Graph * graph, struct config_t conf,Graph__Edge * edge){
	double speed;
	speed = conf.speeds[edge->type];
	if (speed==0){
		printf("Speed not defined for %lld (type %d)\n",edge->osmid,edge->type);
		return DBL_MAX;
	}
	int fromHeight;
	int toHeight;
	fromHeight = graph->vertices[edge->vfrom]->height;
	toHeight = graph->vertices[edge->vto]->height;
	int dh = toHeight-fromHeight;
	//return (edge->dist+abs(dh)*(dh>0?conf.upscale:conf.downscale))/speed;
	if (edge->dist > 100){
		printf("Dist too big");
	}
	return (edge->dist)/speed;
}


struct bbox_t getMapBBox(struct search_data_t * data){
	Graph__Graph * graph;
	int64_t minlon;
	int64_t minlat;
	int64_t maxlon;
	int64_t maxlat;

	graph = data->graph;
	
	minlon = graph->vertices[0]->lon;
	maxlon = graph->vertices[0]->lon;
	minlat = graph->vertices[0]->lat;
	maxlat = graph->vertices[0]->lat;
	for (int i=0;i<graph->n_vertices;i++){
		int64_t lon;
		int64_t lat;
		lon = graph->vertices[i]->lon;
		lat = graph->vertices[i]->lat;

		minlon=(lon<minlon)?lon:minlon;
		minlat=(lat<minlat)?lat:minlat;
		maxlon=(lon>maxlon)?lon:maxlon;
		maxlat=(lat>maxlat)?lat:maxlat;
	}

	struct bbox_t bbox;
	bbox.minlon=minlon;
	bbox.minlat=minlat;
	bbox.maxlon=maxlon;
	bbox.maxlat=maxlat;
	utm2wgs(*data,&bbox.minlon,&bbox.minlat);
	utm2wgs(*data,&bbox.maxlon,&bbox.maxlat);
	return bbox;
}


void printMapBBox(struct search_data_t data){
	Graph__Graph * graph;
	int64_t minlon;
	int64_t minlat;
	int64_t maxlon;
	int64_t maxlat;

	graph = data.graph;
	
	minlon = graph->vertices[0]->lon;
	maxlon = graph->vertices[0]->lon;
	minlat = graph->vertices[0]->lat;
	maxlat = graph->vertices[0]->lat;
	for (int i=0;i<graph->n_vertices;i++){
		int64_t lon;
		int64_t lat;
		lon = graph->vertices[i]->lon;
		lat = graph->vertices[i]->lat;

		minlon=(lon<minlon)?lon:minlon;
		minlat=(lat<minlat)?lat:minlat;
		maxlon=(lon>maxlon)?lon:maxlon;
		maxlat=(lat>maxlat)?lat:maxlat;
	}

	printf("Bounding box in UTM: (%lld,%lld), (%lld,%lld)\n",minlat,minlon,maxlat,maxlon);
	double fminlon=minlon;
	double fminlat=minlat;
	double fmaxlon=maxlon;
	double fmaxlat=maxlat;
	utm2wgs(data,&fminlon,&fminlat);
	utm2wgs(data,&fmaxlon,&fmaxlat);
	printf("Bounding box in WGS-84: (%lf,%lf), (%lf,%lf)\n",fminlat,fminlon,fmaxlat,fmaxlon);
}

void nodesIdx_refresh(int n_nodes, Graph__Vertex ** vertices){
	nodesIdx_cleanup();
	nodesIdx_init();
	for (int i=0;i<n_nodes;i++){
		struct nodesIdxNode * val;
		val = nodesIdx_new(vertices[i]->osmid);
		val->idx = i;
	}
	printf("%d nodes refreshed\n",n_nodes);
}

void sId2sIdx_refresh(int n_stops, Graph__Stop ** stops){
	sId2sIdx_cleanup();
	sId2sIdx_init();
	for (int i=0;i<n_stops;i++){
		struct sId2sIdxNode * val;
		val = sId2sIdx_new(stops[i]->stop_id);
		struct nodesIdxNode * nd;
		nd = nodesIdx_find(stops[i]->osmid);
		if (nd == NULL){
			printf("No matching node for node id %lld\n",stops[i]->osmid);
			continue;
		}
		val->idx = nodesIdx_find(stops[i]->osmid)->idx;
	}
	printf("%d stops refreshed\n",n_stops);
}

void osmId2sIdx_refresh(int n_stops, Graph__Stop ** stops){
	osmId2sIdx_cleanup();
	osmId2sIdx_init();
	for (int i=0;i<n_stops;i++){
		struct osmId2sIdxNode * val;
		val = osmId2sIdx_new(stops[i]->osmid);
		val->idx = i;
	}
	printf("%d stops refreshed\n",n_stops);
}


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

Graph__Graph * loadMap(char * filename){
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
	//printf("Allocated %d MB\n",len>>20);
	size_t read;
	read = fread(buf,1,len,IN);
	if (read!=len){
		printf("Not read all file");
		return NULL;
	}
	fclose(IN);
	
	Graph__Graph * graph;
	graph = graph__graph__unpack(NULL,len,buf);
	free(buf);

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

struct pbf_result_t findMMWay(struct search_data_t data, int fromIdx, int toIdx,time_t starttime,
		Timetable * tt){
	Graph__Graph * graph;
	graph = data.graph;


	struct timetable * newtt;
	time_t date;
	date = starttime - (starttime%(24*3600));
	newtt = gen_tt_for_date(tt,date,NULL);

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
		stopsNode = osmId2sIdx_find(osmid);
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
					nd = nodesIdx_find(osmid);
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

	return processFoundMMRoutes(data,tt,queue->dijArray,queue->vertlut,fromIdx,toIdx);
//	free_tt(newtt);

//	return queue->dijArray;
				
		
}
#undef DIJ_CMP
#undef DIJ_SWAP

struct pbf_result_t processFoundMMRoutes(struct search_data_t data, Timetable * tt, struct mmdijnode_t * dijArray, int ** vertlut, int fromIdx, int toIdx){
	int n_routes;
	n_routes = GARY_SIZE(vertlut[toIdx]);
	printf("Found %d routes\n",n_routes);	
	struct search_result_t * routes;
	routes = calloc(sizeof(struct search_result_t),n_routes);
	struct mmdijnode_t * node;
	for (int i=0;i<n_routes;i++){
		node = dijArray+vertlut[toIdx][i];
		routes[i].time = node->time;
		routes[i].dist = node->penalty;
		printf("Route %d: time: %f, penalty: %f\n",i,routes[i].time,routes[i].dist); 
		struct point_t * points;
		GARY_INIT(points,0);
		do {
			struct point_t * point;
			point = GARY_PUSH(points);
			
			double lon;
			double lat;

			lon = data.graph->vertices[node->idx]->lon;
			lat = data.graph->vertices[node->idx]->lat;
			utm2wgs(data,&lon,&lat);
			point->lon = lon;
			point->lat = lat;
			point->height = data.graph->vertices[node->idx]->height;

			point->departure = node->departed; // TODO shift one point left
			point->arrival = node->time;

			point->vertType = data.graph->vertices[node->idx]->type;
			point->vertId = data.graph->vertices[node->idx]->osmid;

			struct osmId2sIdxNode * stopnode;
			stopnode = osmId2sIdx_find(point->vertId);
			if (stopnode != NULL ){
				point->stopIdx = stopnode->idx;
			} else {
				point->stopIdx = -1;	
			}

			//edgeType, wayId, routeIdx
			if (node->fromEdgeType == ROUTE_TYPE_WALK ){
				point->edgeType = data.graph->edges[node->fromEdgeIdx]->type;
				point->wayId = 	data.graph->edges[node->fromEdgeIdx]->osmid;
			} else if (node->fromEdgeType == ROUTE_TYPE_PT){
				point->edgeType = OBJTYPE__PUBLIC_TRANSPORT;
				point->routeIdx = node->fromEdgeIdx; 
			} else {
				point->edgeType = OBJTYPE__NONE;	
			}

			node = dijArray + node->fromIdx;
				
		} while (node->idx != fromIdx);

		routes[i].n_points = GARY_SIZE(points);
		routes[i].points = calloc(sizeof(struct point_t),routes[i].n_points);
		printf("Points: %d\n",routes[i].n_points);
		for (int j=0;j<routes[i].n_points;j++){
			routes[i].points[j]=points[routes[i].n_points-1-j];
		}
		GARY_FREE(points);
	}

	Result__Result * pbRoutes;
	pbRoutes = malloc(sizeof(Result__Result));
	result__result__init(pbRoutes);
	pbRoutes->n_routes = n_routes;			
	pbRoutes->routes = calloc(sizeof(Result__Route *),n_routes);
	for (int i=0;i<n_routes;i++){
		pbRoutes->routes[i] = malloc(sizeof(Result__Route));
		Result__Route * r;
		r = pbRoutes->routes[i];
		result__route__init(r);
		r->time = routes[i].time;
		r->dist = routes[i].dist;
		r->n_points = routes[i].n_points;
		r->points = calloc(sizeof(Result__Point *),r->n_points);	
		for (int j=0; j<r->n_points;j++){
			r->points[j] = malloc(sizeof(Result__Point));
			Result__Point * pbPt;
			struct point_t * pt;
			pt = routes[i].points + j;
			pbPt = r->points[j];
			result__point__init(pbPt);

			pbPt->lat = pt->lat;
			pbPt->lon = pt->lon;
			pbPt->height = pt->height;
			
			pbPt->departure = pt->departure;
			pbPt->arrival = pt->arrival;
			
			pbPt->verttype = pt->vertType;
			pbPt->has_vertid = 1;
			pbPt->vertid = pt->vertId;
			if (pt->stopIdx != -1){
				pbPt->has_stopidx = 1;
				pbPt->stopidx = pt->stopIdx;
			}

			pbPt->edgetype = pt->edgeType;		
			if (pt->edgeType == ROUTE_TYPE_WALK ){
				pbPt->has_wayid = 1;
				pbPt->wayid = pt->wayId;
			} else if (pt->edgeType == ROUTE_TYPE_PT){
				pbPt->has_routeidx = 1;
				pbPt->routeidx = pt->routeIdx;
			}	
				
		}


	}

	struct pbf_result_t pbf_packed;
	pbf_packed.len = result__result__get_packed_size(pbRoutes);
	pbf_packed.data = malloc(pbf_packed.len);
	result__result__pack(pbRoutes,pbf_packed.data);

	printf("Len: %d\n",pbf_packed.len);
	for (int i=0;i<100;i++){
		printf("%d ",pbf_packed.data[i]);	
	}


	for (int i=0;i<n_routes;i++){
		char filename[20];
		sprintf(filename,"track_%d.gpx",i);
		writeGpxFile(routes[i],filename);	
	}
	
	for (int r=0;r<n_routes;r++){
		printf("Route %d:\n",r);
		for (int s=0;s<routes[r].n_points;s++){
			struct point_t * pt;
			pt = routes[r].points+s;
			if (pt->edgeType == OBJTYPE__PUBLIC_TRANSPORT){
				struct osmId2sIdxNode * stopsNode;
				int raptor_id;

				stopsNode = osmId2sIdx_find(routes[r].points[s-1].vertId);
				raptor_id = data.graph->stops[stopsNode->idx]->raptor_id;
				printf("From: %s ",tt->stops[raptor_id]->name);
				stopsNode = osmId2sIdx_find(pt->vertId);
				raptor_id = data.graph->stops[stopsNode->idx]->raptor_id;
				printf("To: %s\n",tt->stops[raptor_id]->name);
					
			}
		}
	}
	return pbf_packed;
}

struct point_t *  resultsToArray(struct search_data_t data, struct dijnode_t * dijArray, int fromIdx, int toIdx, int * n_points){
	Graph__Graph * graph;
	graph = data.graph;

	if (!dijArray[fromIdx].completed){
		printf("Route not found\n");
		*n_points = 0;
		return NULL;
	}

	int count;
	count = 1;
	//printf("Dist: %f\n",dijArray[fromIdx].dist);
	int idx;
	idx = fromIdx;
	while (idx != toIdx){
		idx = dijArray[idx].fromIdx;
		count++;
	}

	//printf("Cnt: %d\n",count);
	*n_points = count;
	
	struct point_t * results;
	results = malloc(sizeof(struct point_t)*(count+1));
	count = 0;
	idx = fromIdx;
	double lat;
	double lon;
	while (idx != toIdx){
		lat = graph->vertices[idx]->lat;
		lon = graph->vertices[idx]->lon;
		utm2wgs(data,&lon,&lat);
		results[count].lat = lat;
		results[count].lon = lon;
		results[count].height = graph->vertices[idx]->height;
		results[count].edgeType = graph->edges[dijArray[idx].fromEdgeIdx]->type;
		results[count].wayId = graph->edges[dijArray[idx].fromEdgeIdx]->osmid;
		idx = dijArray[idx].fromIdx;
		count++;
	}
	lat = graph->vertices[idx]->lat;
	lon = graph->vertices[idx]->lon;
	utm2wgs(data,&lon,&lat);
	results[count].lat = lat;
	results[count].lon = lon;
	results[count].height = graph->vertices[idx]->height;
	results[count].edgeType=-1;	
	return results;
}

void writeGpxFile(struct search_result_t result,char * filename){
	if (result.n_points==0){
		return;
	}
	printf("Writing results to file %s\n",filename);
	FILE * OUT;
	OUT = fopen(filename,"w");
	writeGpxHeader(OUT);
	writeGpxStartTrack(OUT);
	for (int i=0;i<result.n_points;i++){
		struct point_t p;
		p = result.points[i];
		writeGpxTrkpt(OUT,p.lat,p.lon,p.height);
	}
	writeGpxEndTrack(OUT);
	writeGpxFooter(OUT);
	fclose(OUT);
}

struct search_data_t * prepareData(char * configName, char * dataName){
	struct search_data_t * data;
	data = malloc(sizeof(struct search_data_t));
	data->pj_wgs84 = pj_init_plus("+proj=longlat +datum=WGS84 +no_defs");
	data->pj_utm = pj_init_plus("+proj=utm +zone=33 +ellps=WGS84 +units=m +no_defs");
	data->conf = parseConfigFile(configName);
	data->graph = loadMap(dataName);
	if (data->graph==NULL){
		err(1,"Error loading map\n");	
	}
	data->nodeWays = makeNodeWays(data->graph);
	nodesIdx_refresh(data->graph->n_vertices,data->graph->vertices);
	sId2sIdx_refresh(data->graph->n_stops,data->graph->stops);
	osmId2sIdx_refresh(data->graph->n_stops,data->graph->stops);
	calcDistances(data->graph);
	return data;
}

/*
struct search_result_t findTransfer(struct search_data_t * data, char * from, char * to){
	int fromIdx;
	struct sId2sIdxNode * stopsNode;
	struct search_result_t result;
	stopsNode = sId2sIdx_find(from);
	if (stopsNode == NULL){
		printf("Stop %s not found\n",from);
		result.n_points=0;
		return result;
	}
	fromIdx = stopsNode->idx;

	stopsNode = sId2sIdx_find(to);
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

struct pbf_result_t findPath(struct search_data_t * data,double fromLat, double fromLon, double toLat, double toLon){
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
	struct dijnode_t * dijArray;
	dijArray = prepareDijkstra(data->graph);

	//findWay(*data, dijArray,fromIdx, toIdx);
	

	Timetable * timetable;
	timetable = load_timetable("/home/jethro/Programy/diplomka/mmpf/raptor/tt.bin");
	if (timetable == NULL){
		err(1,"Cannot load timetable");
	}



	return findMMWay(*data,fromIdx,toIdx,time(NULL),timetable);
	/*
	printf("Found\n");
	struct search_result_t result;
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


