#ifndef SEARCHLIB_H
#define SEARCHLIB_H

#include <ucw/lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <inttypes.h>
#include <proj_api.h>
#include <ucw/heap.h>
#include "include/graph.pb-c.h"
#include <yaml.h>


//projPJ pj_wgs84;
//projPJ pj_utm;

struct config_t {
	ProtobufCEnumDescriptor desc;
	int maxvalue;
	double * speeds;
	double * ratios;
};
struct nodeways_t{
	int n_ways;
	int * ways;
};
struct dijnode_t {
	int fromIdx;
	int fromEdgeIdx;
	bool reached;
	bool completed;
	double dist;
};
struct point_t {
	double lat;
	double lon;
	int type;	
};
struct search_result_t {
	int n_points;
	struct point_t * points;
	double dist;
	double time;

};

struct search_data_t{
	projPJ pj_wgs84;
	projPJ pj_utm;
	struct config_t conf;
	Graph__Graph * graph;
	struct nodeways_t * nodeWays;	
};
int utm2wgs(struct search_data_t data, double * lon, double * lat);
int wgs2utm(struct search_data_t data, double * lon, double * lat);

struct config_t parseConfigFile(char * filename);
Graph__Graph * loadMap(char * filename);

void calcDistances(Graph__Graph * graph);
struct nodeways_t * makeNodeWays(Graph__Graph * graph);

static inline  double calcTime(struct config_t conf,Graph__Edge * edge){
	double speed;
	speed = conf.speeds[edge->type];
	if (speed==0)
		return DBL_MAX;
	return edge->dist/speed;
}

struct dijnode_t *  prepareDijkstra(Graph__Graph * graph);
void findWay(struct search_data_t data, 
		struct dijnode_t * dijArray,
		int fromIdx, int toIdx);
struct point_t *  resultsToArray(struct search_data_t data, 
		struct dijnode_t * dijArray, 
		int fromIdx, int toIdx, int * n_points);
int findNearestVertex(Graph__Graph * graph, double lon, double lat);
void writeGpxFile(struct search_data_t data,struct dijnode_t * dijArray,char * filename, int fromIdx, int toIdx);
struct search_result_t findPath(struct search_data_t data,double fromLat, double fromLon, double toLat, double toLon);
struct search_data_t prepareData(char * configName, char * dataName);

#endif
