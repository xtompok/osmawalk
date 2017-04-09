#ifndef HASHES_C
#define HASHES_C


#include <ucw/lib.h>
#include "types.h"
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

/*!
 * @abstract Hash table osm id -> index in array of stops
 */

#define HASH_NODE struct osmId2sIdxNode
#define HASH_PREFIX(name) osmId2sIdx_##name
#define HASH_KEY_ATOMIC osmid 
#define HASH_ATOMIC_TYPE int64_t
#define HASH_DEFAULT_SIZE 6000
#define HASH_WANT_CLEANUP
#define HASH_WANT_FIND
#define HASH_WANT_NEW
#include <ucw/hashtable.h>


#endif
