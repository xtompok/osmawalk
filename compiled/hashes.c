#include <ucw/lib.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "hashes.h"
/*!
 * @abstract Hash table osm id -> index in array for nodes
 */

#define HASH_NODE struct nodesIdxNode
#define HASH_PREFIX(name) nodesIdx_##name
#define HASH_KEY_ATOMIC key
#define HASH_ATOMIC_TYPE int64_t
#define HASH_DEFAULT_SIZE 100000
#define HASH_WANT_CLEANUP
#define HASH_WANT_FIND
#define HASH_WANT_NEW
#include <ucw/hashtable.h>

/*!
 * @abstract Hash table osm id -> index in array for ways
 */

#define HASH_NODE struct waysIdxNode
#define HASH_PREFIX(name) waysIdx_##name
#define HASH_KEY_ATOMIC key
#define HASH_ATOMIC_TYPE int64_t
#define HASH_DEFAULT_SIZE 100000
#define HASH_WANT_CLEANUP
#define HASH_WANT_FIND
#define HASH_WANT_NEW
#include <ucw/hashtable.h>

/*!
 * @abstract Hash table node id -> array of ways, in which node lays
 */
#define HASH_NODE struct nodeWaysNode
#define HASH_PREFIX(name) nodeWays_##name
#define HASH_KEY_ATOMIC nodeid
#define HASH_ATOMIC_TYPE int64_t
#define HASH_DEFAULT_SIZE 100000
#define HASH_WANT_CLEANUP
#define HASH_WANT_FIND
#define HASH_WANT_NEW
#include <ucw/hashtable.h>

/*!
 * @abstract Hash table stop id -> index in array for nodes
 */

#define HASH_NODE struct sId2sIdxNode
#define HASH_PREFIX(name) sId2sIdx_##name
#define HASH_KEY_STRING stop_id
#define HASH_DEFAULT_SIZE 6000
#define HASH_WANT_CLEANUP
#define HASH_WANT_FIND
#define HASH_WANT_NEW
#include <ucw/hashtable.h>


void nodesIdx_refresh(int n_nodes, Graph__Vertex ** vertices){
	nodesIdx_cleanup();
	nodesIdx_init();
	for (int i=0;i<n_nodes;i++){
		struct nodesIdxNode * val;
		val = nodesIdx_new(vertices[i]->osmid);
		val->idx = i;
	}
	printf("%d nodes refreshed\n",n_nodes);
}

void sId2sIdx_refresh(int n_stops, Graph__Stop ** stops){
	sId2sIdx_cleanup();
	sId2sIdx_init();
	for (int i=0;i<n_stops;i++){
		struct sId2sIdxNode * val;
		val = sId2sIdx_new(stops[i]->stop_id);
		struct nodesIdxNode * nd;
		nd = nodesIdx_find(stops[i]->osmid);
		if (nd == NULL){
			printf("No matching node for node id %lld\n",stops[i]->osmid);
			continue;
		}
		val->idx = nodesIdx_find(stops[i]->osmid)->idx;
	}
	printf("%d stops refreshed\n",n_stops);
}

struct nodesIdxNode * nodesIdx_find2(int64_t key){
	return nodesIdx_find(key);	
}

struct waysIdxNode * waysIdx_find2(int64_t key){
	return waysIdx_find(key);	
}

struct nodeWaysNode * nodeWays_find2(int64_t key){
	return nodeWays_find(key);	
}

struct sId2sIdxNode * sId2sIdx_find2(char * stop_id){
	return sId2sIdx_find(stop_id);
}

