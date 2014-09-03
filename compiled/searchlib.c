#include <ucw/lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <inttypes.h>

#include <yaml.h>
#include <proj_api.h>
#include <ucw/heap.h>

#include "include/graph.pb-c.h"

#include "searchlib.h"
#include "writegpx.h"

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

double calcTime(Graph__Graph * graph, struct config_t conf,Graph__Edge * edge){
	double speed;
	speed = conf.speeds[edge->type];
	if (speed==0)
		return DBL_MAX;
	int fromHeight;
	int toHeight;
	fromHeight = graph->vertices[edge->vfrom]->height;
	toHeight = graph->vertices[edge->vto]->height;
	int dh = toHeight-fromHeight;
	return edge->dist+abs(dh)*((dh>0?conf.upscale:conf.downscale))/speed;
}


// Loading 
struct config_t parseConfigFile(char * filename){
	struct config_t conf;
	conf.desc = objtype__descriptor;
	conf.maxvalue = conf.desc.values[conf.desc.n_values-1].value;
	conf.speeds = malloc(sizeof(double)*(conf.maxvalue+1));
	conf.ratios = malloc(sizeof(double)*(conf.maxvalue+1));
	for (int i=0;i<conf.maxvalue+1;i++){
		conf.speeds[i]=-1;
		conf.ratios[i]=-1;
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
				printf("Parsing speeds\n");
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
				printf("Parsing ratios\n");
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
			} else if (strcmp((char *)key->data.scalar.value,"heights")==0){
				printf("Parsing heights\n");
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
	printf("Allocated %d MB\n",len>>20);
	size_t read;
	read = fread(buf,1,len,IN);
	if (read!=len){
		printf("Not read all file");
		return NULL;
	}
	fclose(IN);
	
	Graph__Graph * graph;
	graph = graph__graph__unpack(NULL,len,buf);

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
		int64_t dlon;
		int64_t dlat;
		dlon = graph->vertices[edge->vto]->lon-graph->vertices[edge->vfrom]->lon;
		dlat = graph->vertices[edge->vto]->lat-graph->vertices[edge->vfrom]->lat;

		edge->dist = sqrt(dlon*dlon+dlat*dlat);
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
	printf("Point %d: %f, %f\n",minIdx,graph->vertices[minIdx]->lon,graph->vertices[minIdx]->lat);
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


void findWay(struct search_data_t data,struct dijnode_t * dijArray,int fromIdx, int toIdx){
	
	int tmp;
	tmp=fromIdx;
	fromIdx=toIdx;
	toIdx=tmp;

	Graph__Graph * graph;
	graph = data.graph;

	struct nodeways_t * nodeways;
	nodeways = data.nodeWays;

	int * heap;
	int n_heap;
	heap = malloc(sizeof(int)*(graph->n_vertices+1));
	int * heapIndex;
	heapIndex = malloc(sizeof(int)*graph->n_vertices);

	#define DIJ_SWAP(heap,a,b,t) ( \
		heapIndex[heap[a]]=b,heapIndex[heap[b]]=a,\
		t=heap[a],heap[a]=heap[b],heap[b]=t)

	#define DIJ_CMP(x,y) (dijArray[x].dist<dijArray[y].dist)

	dijArray[fromIdx].reached=true;
	dijArray[fromIdx].dist=0;
	
	n_heap = 0;
	heapIndex[fromIdx]=1;
	HEAP_INSERT(int,heap,n_heap,DIJ_CMP,DIJ_SWAP,fromIdx);

	while(n_heap > 0){
		HEAP_DELETE_MIN(int,heap,n_heap,DIJ_CMP,DIJ_SWAP);
		int vIdx;
		vIdx = heap[n_heap+1];
		
		if (!dijArray[vIdx].reached){
			printf("Found unreached vertex, exitting\n");
			return;
		}
		for (int i=0;i<nodeways[vIdx].n_ways;i++){
			Graph__Edge * way;
			way = graph->edges[nodeways[vIdx].ways[i]];
			double len;
			len = calcTime(data.graph,data.conf,way);
			if (dijArray[way->vto].dist <= (dijArray[vIdx].dist+len))
				continue;
			dijArray[way->vto].dist = dijArray[vIdx].dist+len;
			dijArray[way->vto].fromIdx = vIdx;
			dijArray[way->vto].fromEdgeIdx = nodeways[vIdx].ways[i];
			if (dijArray[way->vto].reached){
				HEAP_DECREASE(int,heap,n_heap,DIJ_CMP,DIJ_SWAP,heapIndex[way->vto],way->vto);
			}else {
				dijArray[way->vto].reached = true;
				heapIndex[way->vto]=n_heap+1;
				HEAP_INSERT(int,heap,n_heap,DIJ_CMP,DIJ_SWAP,way->vto);
			}
		}
		dijArray[vIdx].completed=true;
		if (vIdx == toIdx)
			break;
	}
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
	printf("Dist: %f\n",dijArray[fromIdx].dist);
	int idx;
	idx = fromIdx;
	while (idx != toIdx){
		idx = dijArray[idx].fromIdx;
		count++;
	}

	printf("Cnt: %d\n",count);
	*n_points = count;
	
	struct point_t * results;
	results = malloc(sizeof(struct point_t)*(count+1));
	count = 0;
	idx = fromIdx;
	double lat;
	double lon;
	double dist;
	dist = 0;
	while (idx != toIdx){
		lat = graph->vertices[idx]->lat;
		lon = graph->vertices[idx]->lon;
		utm2wgs(data,&lon,&lat);
		results[count].lat = lat;
		results[count].lon = lon;
		results[count].height = graph->vertices[idx]->height;
		results[count].type = graph->edges[dijArray[idx].fromEdgeIdx]->type;
		idx = dijArray[idx].fromIdx;
		count++;
	}
	lat = graph->vertices[idx]->lat;
	lon = graph->vertices[idx]->lon;
	utm2wgs(data,&lon,&lat);
	results[count].lat = lat;
	results[count].lon = lon;
	results[count].height = graph->vertices[idx]->height;
	results[count].type=-1;	
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

struct search_data_t prepareData(char * configName, char * dataName){
	struct search_data_t data;
	data.pj_wgs84 = pj_init_plus("+proj=longlat +datum=WGS84 +no_defs");
	data.pj_utm = pj_init_plus("+proj=utm +zone=33 +ellps=WGS84 +units=m +no_defs");
	data.conf = parseConfigFile(configName);
	data.graph = loadMap(dataName);
	if (data.graph==NULL){
		printf("Error loading map\n");	
		return data;
	}
	data.nodeWays = makeNodeWays(data.graph);
	calcDistances(data.graph);
	return data;
}


struct search_result_t findPath(struct search_data_t data,double fromLat, double fromLon, double toLat, double toLon){
	wgs2utm(data,&fromLon,&fromLat);
	int fromIdx;
	fromIdx = findNearestVertex(data.graph,fromLon,fromLat);

	wgs2utm(data,&toLon,&toLat);
	int toIdx;
	toIdx = findNearestVertex(data.graph,toLon,toLat);

	printf("Searching from %lld(%f,%f,%d) to %lld(%f,%f,%d)\n",data.graph->vertices[fromIdx]->osmid,
			data.graph->vertices[fromIdx]->lat,
			data.graph->vertices[fromIdx]->lon,
			data.graph->vertices[fromIdx]->height,
			data.graph->vertices[toIdx]->osmid,
			data.graph->vertices[toIdx]->lat,
			data.graph->vertices[toIdx]->lon,
			data.graph->vertices[toIdx]->height
			);

	struct dijnode_t * dijArray;
	dijArray = prepareDijkstra(data.graph);

	findWay(data, dijArray,fromIdx, toIdx);
	struct search_result_t result;
	int n_points;
	result.points = resultsToArray(data,dijArray,fromIdx,toIdx,&n_points);
	result.n_points = n_points;
	result.dist = 0;
	if (result.n_points == 0){
		result.time=0;
		return result;
	}
	int idx;
	idx = fromIdx;
	while(idx != toIdx){
		result.dist+=data.graph->edges[dijArray[idx].fromEdgeIdx]->dist;
		idx = dijArray[idx].fromIdx;
	}
	result.time = dijArray[fromIdx].dist;
	return result;	
}


