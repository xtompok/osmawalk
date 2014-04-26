#ifndef TYPES_H
#define TYPES_H
#include <stdint.h>
#include "include/premap.pb-c.h"

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

struct line_t {
	double  startlon;
	double  startlat;
	double endlon;
	double endlat;
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
	int n_bars;
	int * barIdxs;
	int n_ways;
	int * wayIdxs;
	int  periIdx;
};

#endif
