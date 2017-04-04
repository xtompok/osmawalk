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

/*! @struct raster_t 
 * @abstract Struct for representing raster over a map
 * @field minlon Longitude of a lower left corner of the raster
 * @field minlon Latitude of a lower left corner of the raster
 * @field lonparts Number of cells in longitude dimension
 * @field latparts Number of cells in latitude dimension
 * @field steplon Width of a cell 
 * @field steplat Height of a cell
 * @field raster Raster array
 */
struct raster_t {
	int64_t minlon;
	int64_t minlat;
	int64_t lonparts;
	int64_t latparts;
	int64_t steplon;
	int64_t steplat;
	int *** raster;
};

/*! @struct tree_node_t
 * @abstract Structure for node in tree for sweeping
 * @field lineIdx Index of a line
 */
struct tree_node_t{
	int lineIdx;
};

/*! @struct walk_area_t
 * @abstract Struct for representig walk areas
 * @field periIdx Index of a line defining perimeter of the area
 * @field n_nodes Number of nodes inside
 * @field nodesIdxs Array of indexes to nodes inside
 * @field n_bars Number of barriers inside
 * @field barIdxs Array of indexes to barrier ways inside
 * @field n_way Number of ways inside
 * @field wayIdxs Array of indexes to ways inside
 */
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
