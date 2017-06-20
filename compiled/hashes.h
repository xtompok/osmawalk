#ifndef HASHES_H
#define HASHES_H

#include "types.h"
/*!
 * @abstract Hash table osm id -> index in array for nodes
 */
struct nodesIdxNode * nodesIdx_find2(int64_t key);

/*!
 * @abstract Hash table osm id -> index in array for ways
 */
struct waysIdxNode * waysIdx_find2(int64_t key);

/*!
 * @abstract Hash table node id -> array of ways, in which node lays
 */
struct nodeWaysNode * nodeWays_find2(int64_t key);

/*!
 * @abstract Hash table stop id -> index in array for nodes
 */

struct sId2sIdxNode * sId2sIdx_find2(char * stop_id);


void nodesIdx_refresh(int n_nodes, Graph__Vertex ** vertices);

void sId2sIdx_refresh(int n_stops, Graph__Stop ** stops);

#endif
