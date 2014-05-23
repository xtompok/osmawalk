#ifndef TYPES_H
#define TYPES_H
#include <ucw/lib.h>
#include <stdint.h>
#include "include/premap.pb-c.h"

#define SWEEP_TYPE int64_t
/*!
 * @abstract 
 * @discussion
 */

struct nodesIdxNode {
	int64_t key;
	int idx;
};
struct waysIdxNode {
	int64_t key;
	int idx;
};
struct nodeWaysNode {
	int64_t nodeid;
	int64_t * ways;
};

struct mixed_num_t {
	int64_t base;
	int64_t numer;
	int64_t denom;
};

struct col_t {
	uint8_t isCol;
	uint8_t atEndpoint;
	struct mixed_num_t lon;
	struct mixed_num_t lat;
};

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

struct raster_t {
	int64_t minlon;
	int64_t minlat;
	int64_t lonparts;
	int64_t latparts;
	int64_t steplon;
	int64_t steplat;
	int *** raster;
};

enum event_type  {EVT_START=2,EVT_END=0,EVT_INTERSECT=1};
struct event_t {
	enum event_type type;
	SWEEP_TYPE lon;
	SWEEP_TYPE lat;
	SWEEP_TYPE dlon;
	SWEEP_TYPE dlat;	
	unsigned int lineIdx;
};
struct int_event_t {
	enum event_type type;
	struct mixed_num_t lon;
	struct mixed_num_t lat;
	SWEEP_TYPE dlon;
	SWEEP_TYPE dlat;	
	unsigned int lineIdx;
	unsigned int line2Idx;
};

struct line_t {
	SWEEP_TYPE  startlon;
	SWEEP_TYPE  startlat;
	SWEEP_TYPE endlon;
	SWEEP_TYPE endlat;
	int64_t startid;
	int64_t endid;
	bool isBar;
	bool broken;
	bool started;
	bool ended;
};

struct tree_node_t{
	int lineIdx;
};

struct walk_area_t{
	int  periIdx;
	int n_nodes;
	int * nodeIdxs;
	int n_bars;
	int * barIdxs;
	int n_ways;
	int * wayIdxs;
};

#endif
