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

#endif
