#ifndef TYPES_H
#define TYPES_H
#include <ucw/lib.h>
#include <stdint.h>
#include "include/premap.pb-c.h"

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


struct stopsIdxNode {
	char * stop_id;
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

#endif
