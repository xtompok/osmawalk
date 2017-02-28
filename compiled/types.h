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

/*! @struct mixed_num_t
 * @abstract Struct for representing mixed numbers. numer/denom should be always
 * lower than 1
 * @field base Integral part of a number
 * @field numer Numerator of a rational part
 * @field denom Denominator of a rational part
 */
struct mixed_num_t {
	int64_t base;
	int64_t numer;
	int64_t denom;
};

/*! @struct col_t 
 * @abstract Struct for representing collisions
 * @field isCol Is it collision?
 * @field atEndpoint Is the collision at endpoint of both lines?
 * @field lon Longitude of a collision in mixed number
 * @field lat Latitude of a collision in mixed number
 */
struct col_t {
	uint8_t isCol;
	uint8_t atEndpoint;
	struct mixed_num_t lon;
	struct mixed_num_t lat;
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

enum event_type  {EVT_START=2,EVT_END=0,EVT_INTERSECT=1};
/*! @struct event_t
 * @abstract Start or end event
 * @field lon Longitude of an event
 * @field lat Latitude of an event
 * @field dlon Delta of longitude of a line
 * @field dlat Delta of latitude of a line
 * @field lineIdx Index of a line in lines array
 */
struct event_t {
	enum event_type type;
	SWEEP_TYPE lon;
	SWEEP_TYPE lat;
	SWEEP_TYPE dlon;
	SWEEP_TYPE dlat;	
	unsigned int lineIdx;
};
/*! @struct int_event_t
 * @abstract Intersecton event
 * @field lon Longitude of an event
 * @field lat Latitude of an event
 * @field dlon Delta of longitude of a lower line
 * @field dlat Delta of latitude of a lower line
 * @field lineIdx Index of a first line in lines array
 * @field line2Idx Index of a second line in lines array
 */
struct int_event_t {
	enum event_type type;
	struct mixed_num_t lon;
	struct mixed_num_t lat;
	SWEEP_TYPE dlon;
	SWEEP_TYPE dlat;	
	unsigned int lineIdx;
	unsigned int line2Idx;
};

/*! @struct line_t
 * @abstract Struct for representing line
 * @field startlon Longitude of a starting point
 * @field startlat Latitude of a starting point
 * @field endlon Longitude of an end point
 * @field endlat Latitude of an end point
 * @field startid Id of a srating point
 * @field endid Id of a end point
 * @field isBar Is it a barrier line?
 * @field broken Is it a barrier or crossed by it?
 * @field started Has the line started?
 * @field ended Hase the line ended?
 */
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
