#include <ucw/lib.h>
#include <ucw/fastbuf.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <inttypes.h>

#include <ucw/gary.h>
#include <ucw/heap.h>
#include "include/geodesic.h"

#include "include/graph.pb-c.h"

#include <yaml.h>

#define SWEEP_TYPE int64_t
#include "types.h"
#include "raster.h"
//#include "tree.h"
//#include "hashes.c"
#include "utils.h"
#include "writegpx.h"

int scale = 1000000;



struct config_t {
	ProtobufCEnumDescriptor desc;
	int maxvalue;
	double * speeds;
	double * ratios;
};

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
			} else {
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
	//	printf("Item: %d, speed: %f, ratio: %f\n",i,conf.speeds[i],conf.ratios[i]);
		if (conf.speeds[i]==-1){
			if (conf.ratios[i]==-1)
				conf.speeds[i]=conf.speeds[wayIdx];
			else
				conf.speeds[i]=conf.speeds[wayIdx]*conf.ratios[i];
		}
		conf.speeds[i]/=3.6;
	//	printf("Item: %d, speed: %f\n",i,conf.speeds[i],conf.ratios[i]);
	}
	return conf;



}
void calcDistances(Graph__Graph * graph){
	for (int i=0;i<graph->n_edges;i++){
		Graph__Edge * edge;
		edge = graph->edges[i];
		double g_a = 6378137, g_f = 1/298.257223563; /* WGS84 */
		struct geod_geodesic geod;
		geod_init(&geod, g_a, g_f);
		double dist;
		double tmp1;
		double tmp2;
		geod_inverse(&geod,graph->vertices[edge->vfrom]->lat,
				graph->vertices[edge->vfrom]->lon,
				graph->vertices[edge->vto]->lat,
				graph->vertices[edge->vto]->lon,&dist,&tmp1,&tmp2);
		edge->dist = dist;
	}
}

struct nodeways_t{
	int n_ways;
	int * ways;
};

struct nodeways_t * makeNodeWays(Graph__Graph * graph){
	struct nodeways_t * nodeways;
	nodeways = malloc(sizeof(struct nodeways_t)*graph->n_vertices);
	for (int i=0;i<graph->n_vertices;i++){
		nodeways[i].n_ways=0;
//		printf("%f %f\n",graph->vertices[i]->lat,graph->vertices[i]->lon);
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

double calcTime(Graph__Graph * graph,struct config_t conf,Graph__Edge * edge){
	double speed;
	speed = conf.speeds[edge->type];
	if (speed==0)
		return DBL_MAX;
	/*double g_a = 6378137, g_f = 1/298.257223563; // WGS84 
	struct geod_geodesic geod;
	geod_init(&geod, g_a, g_f);
	double dist;
	double tmp1;
	double tmp2;
	geod_inverse(&geod,graph->vertices[edge->vfrom]->lat,
			graph->vertices[edge->vfrom]->lon,
			graph->vertices[edge->vto]->lat,
			graph->vertices[edge->vto]->lon,&dist,&tmp1,&tmp2);
	*/
	return edge->dist/speed;
}


struct dijnode_t {
	int fromIdx;
	int fromEdgeIdx;
	bool reached;
	bool completed;
	double dist;
};

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


void findWay(Graph__Graph * graph, struct config_t conf, struct nodeways_t * nodeways,struct dijnode_t * dijArray,int fromIdx, int toIdx){
	dijArray[fromIdx].reached=true;
	dijArray[fromIdx].dist=0;
	
/*	int tmp;
	tmp=fromIdx;
	fromIdx=toIdx;
	toIdx=tmp;
*/
	int * heap;
	int n_heap;
	n_heap = graph->n_vertices;
	heap = malloc(sizeof(int)*(graph->n_vertices+1));
	int * heapIndex;
	heapIndex = malloc(sizeof(int)*graph->n_vertices);
	//for (int i=0;i<graph->n_vertices;i++){
	//	heapIndex[i]=i+1;
	//	heap[i+1]=i;
	//}
	//heap[1]=fromIdx;

	#define DIJ_SWAP(heap,a,b,t) ( \
		heapIndex[heap[a]]=b,heapIndex[heap[b]]=a,\
		t=heap[a],heap[a]=heap[b],heap[b]=t)

	#define DIJ_CMP(x,y) (dijArray[x].dist<dijArray[y].dist)
	
	n_heap = 0;

	//HEAP_INIT(int,heap,n_heap,DIJ_CMP,DIJ_SWAP);
	
	heapIndex[fromIdx]=1;
	HEAP_INSERT(int,heap,n_heap,DIJ_CMP,DIJ_SWAP,fromIdx);

	while(!dijArray[toIdx].completed){
	//	printf("Heap has %d vertices\n", n_heap);
		HEAP_DELETE_MIN(int,heap,n_heap,DIJ_CMP,DIJ_SWAP);
		int vIdx;
		vIdx = heap[n_heap+1];
	//	printf("Dist: %f\n",dijArray[vIdx].dist);
		
		if (!dijArray[vIdx].reached){
			printf("Found unreached vertex, exitting\n");
			return;
		
		}
		if (dijArray[vIdx].completed){
			continue;
		}
		//printf("%d %f\n",vIdx,dijArray[vIdx].dist);
		for (int i=0;i<nodeways[vIdx].n_ways;i++){
			Graph__Edge * way;
			way = graph->edges[nodeways[vIdx].ways[i]];
			double len;
			len = calcTime(graph,conf,way);
		//	printf("Neigh len %f\n",len);
			if (dijArray[way->vto].dist <= (dijArray[vIdx].dist+len))
				continue;
//			printf("Replacing\n");
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
//			printf("id %d idx: %d\n",way->vto,heapIndex[way->vto]);
//			printf("id:%d, dist:\n",heap[heapIndex[way->vto]]);
//			printf("id:%d, dist:\n",heap[heapIndex[way->vto]]);
		
		}
		HEAP_INIT(int,heap,n_heap,DIJ_CMP,DIJ_SWAP);
		dijArray[vIdx].completed=true;
	}
}

void printResults(Graph__Graph * graph, struct config_t conf, struct dijnode_t * dijArray, int fromIdx, int toIdx){
	if (!dijArray[toIdx].completed){
		printf("Route not found\n");
		return;
	}
	printf("D1: %f, D2:%f\n",dijArray[fromIdx].dist,dijArray[toIdx].dist);
	int idx;
	idx = toIdx;
	while (dijArray[idx].fromIdx!=-1){
		printf("%f,%f,%d\n",graph->vertices[idx]->lat,graph->vertices[idx]->lon,graph->edges[dijArray[idx].fromEdgeIdx]->type);
		idx = dijArray[idx].fromIdx;
	}
	printf("%f,%f,\n",graph->vertices[idx]->lat, graph->vertices[idx]->lon);	
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
	
int findNearestVertex(Graph__Graph * graph, double lon, double lat){
	printf("Lat: %f, lon: %f\n",lat,lon);
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
	printf("Min dist: %f\n",minDist*1000000);
	printf("Point %d: %f, %f\n",minIdx,graph->vertices[minIdx]->lon,graph->vertices[minIdx]->lat);
	return minIdx;
}

void usage(void){
	printf("Usage: ./search fromlat fromlon tolat tolon\n");		
}


void writeGpxFile(Graph__Graph * graph, struct config_t conf, struct dijnode_t * dijArray,char * filename, int fromIdx, int toIdx){
	if (!dijArray[toIdx].completed){
		return;
	}
	printf("Writing results to file %s\n",filename);
	FILE * OUT;
	OUT = fopen(filename,"w");
	writeGpxHeader(OUT);
	writeGpxStartTrack(OUT);
	int idx;
	idx = toIdx;
	while (dijArray[idx].fromIdx!=-1){
		writeGpxTrkpt(OUT,graph->vertices[idx]->lat,graph->vertices[idx]->lon,0);
		//printf("%f,%f,%d\n",graph->vertices[idx]->lat,graph->vertices[idx]->lon,graph->edges[dijArray[idx].fromEdgeIdx]->type);
		idx = dijArray[idx].fromIdx;
	}
	printf("%f,%f,\n",graph->vertices[idx]->lat, graph->vertices[idx]->lon);	
	writeGpxEndTrack(OUT);
	writeGpxFooter(OUT);
	fclose(OUT);
}

int main (int argc, char ** argv){
	if (argc<5){
		usage();
		return 1;
	}
	struct config_t conf;
	conf = parseConfigFile("../config/speeds.yaml");
	
	Graph__Graph * graph;
	graph = loadMap("../data/praha-graph.pbf");
	if (graph==NULL){
		printf("Error loading map\n");	
		return 2;
	}

	printf("Graph has %d vertices and %d edges\n",graph->n_vertices,graph->n_edges);
	double lon;
	double lat;
	lat = atof(argv[1]);
	lon = atof(argv[2]);
	int fromIdx;
	fromIdx = findNearestVertex(graph,lon,lat);

	lat = atof(argv[3]);
	lon = atof(argv[4]);
	int toIdx;
	toIdx = findNearestVertex(graph,lon,lat);

	calcDistances(graph);

	printf("Searching from %lld(%f,%f) to %lld(%f,%f)\n",graph->vertices[fromIdx]->osmid,
			graph->vertices[fromIdx]->lat,
			graph->vertices[fromIdx]->lon,
			graph->vertices[toIdx]->osmid,
			graph->vertices[toIdx]->lat,
			graph->vertices[toIdx]->lon
			);

	struct dijnode_t * dijArray;
	dijArray = prepareDijkstra(graph);

	struct nodeways_t * nodeWays;
	nodeWays = makeNodeWays(graph);

	findWay(graph, conf, nodeWays, dijArray,fromIdx, toIdx);
	printResults(graph, conf, dijArray, fromIdx, toIdx);
	writeGpxFile(graph, conf, dijArray,"track.gpx", fromIdx, toIdx);

	return 0;
}
