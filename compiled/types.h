#ifndef TYPES_H
#define TYPES_H
#include <ucw/lib.h>
#include <stdint.h>
#include <proj_api.h>
#include "include/premap.pb-c.h"
#include "include/graph.pb-c.h"
#include "libraptor.h"

#define SWEEP_TYPE int64_t
/*!
 * @abstract Structures for preparing data 
 * @discussion This file contains almost all structures used when preparing data
 */

/*! @struct nodesIdxNode 
 * @field key Node id
 * @field idx Index in nodes array
 */
struct nodesIdxNode {
	int64_t key;
	int idx;
};
/*! @struct waysIdxNode 
 * @field key Way id
 * @field Index in ways array
 */
struct waysIdxNode {
	int64_t key;
	int idx;
};
/*! @struct nodeWaysNode
 * @field nodeid Node id
 * @field ways Array of ways on which node lies
 */
struct nodeWaysNode {
	int64_t nodeid;
	int64_t * ways;
};


struct sId2sIdxNode {
	char * stop_id;
	int idx;
};

struct osmId2sIdxNode {
	int64_t	osmid;
	int idx;
};


/*! @struct map_t
 * @abstract Struct for representing map
 * @field n_nodes Number of nodes
 * @field nodes Array of nodes
 * @field n_ways Number of ways
 * @field ways Array of ways
 * @field n_relations Number of relations
 * @field relations Array of relations
 * @field n_multipols Number of multipolygons
 * @field multipols Array of multipolygons
 */
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

#define ROUTE_TYPE_NONE 0
#define ROUTE_TYPE_WALK 1
#define ROUTE_TYPE_PT 2

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
	// Location
	double lat;
	double lon;
	int height;
	// Time
	time_t departure;
	time_t arrival;
	// Penalty (for debugging)
	double penalty;
	// Vertex features
	Graph__Vertex * osmvert;
	Stop * stop;
	// Edge features
	struct edge_t * edge;
};

/* @struct search_result_t
 * @abstract Struct for handling found path
 * @field n_points Number of points on path
 * @field points Points on path
 * @field dist Travelled distance
 * @field time Time needed for it
 */
struct search_route_t {
	int n_points;
	struct point_t * points;
	double dist;
	double time;
	double penalty;

};

struct search_result_t {
	int n_routes;
	struct search_route_t * routes;	
};

struct pbf_data_t {
	long int len;
	uint8_t * data;	
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
	Timetable * timetable;
	uint64_t * raptor2osm;
	uint64_t * raptor2idx;
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

#endif
