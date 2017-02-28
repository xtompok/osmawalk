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
#include <yaml.h>
#include "include/graph.pb-c.h"

/*!
 * @abstract Library for searching path
 *
 * @discussion This library containf functions to load saved graph and speeds
 * configuration and repeatedly searching paths.
 */

/*!
 * @struct config_t
 * @abstract Struct for representing configuration file.
 * @field desc Protocol buffer descriptor for decoding name of th item -> tag of
 * the item
 * @field maxvalue Maximum tag
 * @field speeds Array of speeds, speed for tag i is on the position i
 * @field ratios Array of speed ratios, ratio for tag i is on the position i
 * @field upscale How many horizontal meters equals one vertical meter whe going
 * up
 * @field downscale How many horizontal meters equals one vertical meter whe going
 * down
 */
struct config_t {
	ProtobufCEnumDescriptor desc;
	int maxvalue;
	double * speeds;
	double * ratios;
	double * penalties;
	double upscale;
	double downscale;
//	double maxslope;
//	double upslopescale;
//	double downslopescale;
};

/*! @struct nodeways_t
 * @abstract Struct representing array of ways on which node lies
 * @field n_ways Number of ways
 * @field ways Array of indexes to the array of ways
 */
struct nodeways_t{
	int n_ways;
	int * ways;
};

/*! @struct dijnode_t
 * @abstract Struct fo representing additional information for vertex when
 * searching with Dijkstra algorithm
 * @field fromIdx From which index we came
 * @field fromEdgeIdx From which edge we came
 * @field reached The vertex was reached
 * @field completed The vertex was completed
 * @filed dist Distance from starting point
 */
struct dijnode_t {
	int fromIdx;
	int fromEdgeIdx;
	bool reached;
	bool completed;
	double dist;
};
/*! @struct point_t
 * @abstract Struct for representing point in searched path
 * @field lat Latitude of a point
 * @field lon Longitude of a point
 * @field height Height of a point
 * @field type Type of edge on searched path from this point
 */
struct point_t {
	double lat;
	double lon;
	int height;
	int type;	
	uint64_t wayid;
};

/* @struct search_result_t
 * @abstract Struct for handling found path
 * @field n_points Number of points on path
 * @field points Points on path
 * @field dist Travelled distance
 * @field time Time needed for it
 */
struct search_result_t {
	int n_points;
	struct point_t * points;
	double dist;
	double time;

};

/* @struct search_data_t
 * @abstract Struct for handling loaded graph and speeds configuration
 * @field pj_wgs84 PROJ.4 structure for WGS-84 projection
 * @field pj_utm PROJ.4 structutre for UTM projection
 * @field conf Searching configuration
 * @field graph Searching graph
 * @field nodeWays Array of ways for each vertex, on which vertex lies
 */
struct search_data_t{
	projPJ pj_wgs84;
	projPJ pj_utm;
	struct config_t conf;
	Graph__Graph * graph;
	struct nodeways_t * nodeWays;	
};

/* @struct bbox_t
 * @abstract Struct for bounding box
 * @field minlon Minimal longitude
 * @field minlat Minimal latitude
 * @field maxlon Maximal longitude
 * @field maxlat Maximal latitude
 */
struct bbox_t{
	double minlon;
	double minlat;
	double maxlon;
	double maxlat;		
};

/*!
 * Converts inplace coordinates from UTM to WGS-84
 * @param data Search data (for projections)
 * @param lon x coordinate
 * @param lat y coordinate
 * @result Result of a PROJ.4 conferting function
 */
int utm2wgs(struct search_data_t data, double * lon, double * lat);
/*!
 * Converts inplace coordinates from WGS-84 to UTM
 * @param data Search data (for projections)
 * @param lon Longitude
 * @param lat Latitude
 * @result Result of a PROJ.4 conferting function
 */
int wgs2utm(struct search_data_t data, double * lon, double * lat);
/*!
 * Calculate time for going through an edge
 * @param graph Search graph
 * @param conf Speeds config
 * @param edge Pointer to an edge
 * @result Time in seconds
 */
double calcTime(Graph__Graph * graph,struct config_t conf,Graph__Edge * edge);

void nodesIdx_refresh(int n_nodes, Graph__Vertex ** vertices);
void stopsIdx_refresh(int n_stops, Graph__Stop ** stops);

/*!
 * Parse config file
 * @param filename Filename of the file
 * @result Speeds configuration
 */
struct config_t parseConfigFile(char * filename);
/*! Load graph from file
 *
 * @param filename Filename of the file
 * @result Graph structure
 */
Graph__Graph * loadMap(char * filename);

/*! Calculate distances of all edges
 * @param graph Graph struct, distances are filled in parameter dist
 */
void calcDistances(Graph__Graph * graph);
/*! Make array with way array for each node
 * @param graph Graph struct
 * @result Array with array of ways for each node on which node lies
 */
struct nodeways_t * makeNodeWays(Graph__Graph * graph);

/*! Find nearest vertex for given coordinates
 * @param graph Search graph
 * @param lon Longitude of a point
 * @param lat Latitude of a point
 * @result Index of nearest vertex
 */
int findNearestVertex(Graph__Graph * graph, double lon, double lat);

/*! Prepare data for Dijkstra searching
 * @param graph
 * @result array of additional structs for each vertex
 */
struct dijnode_t *  prepareDijkstra(Graph__Graph * graph);

/*! Find way using Dijkstra
 * @param data Search data 
 * @param dijArray Array with aditional structs
 * @param fromIdx Start vertex index
 * @param toIdx End vertex index
 */
void findWay(struct search_data_t data, 
		struct dijnode_t * dijArray,
		int fromIdx, int toIdx);
/*! Convert search results from additions for vertices to array
 * @param data Search data
 * @param dijArray Array with additional structs
 * @param fromIdx Start vertex index
 * @param toIdx End vertex index
 * @param n_points Save number of points of the way to this pointer
 * @result Array of points on the way
 */
struct point_t *  resultsToArray(struct search_data_t data, 
		struct dijnode_t * dijArray, 
		int fromIdx, int toIdx, int * n_points);
/*! Write searched way to GPX file
 * @param result Structure handlig search result
 * @param filename Filename of the GPX file
 */
void writeGpxFile(struct search_result_t result,char * filename);

/*! Prepare all data for searching 
 * @param configName Name of the configuration file
 * @param dataName Name of the file with searching graph
 * @result Search data structure
 */
struct search_data_t * prepareData(char * configName, char * dataName);

/*! Find path in graph from coorinates to coordinates
 * @param data Search data
 * @param fromLat Starting point latitude
 * @param fromLon Starting point longitude
 * @param toLat End point latitude
 * @param toLon End point longitude
 * @result Structure for handli search result
 */
struct search_result_t findPath(struct search_data_t * data,
		double fromLat, double fromLon, double toLat, double toLon);

struct search_result_t findTransfer(struct search_data_t * data,
		char * from, char * to);

struct bbox_t getMapBBox(struct search_data_t * data);
void printMapBBox(struct search_data_t data);

#endif
