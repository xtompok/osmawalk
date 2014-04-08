#include <ucw/lib.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "include/premap.pb-c.h"

//#define ARRAY_TYPE int
//#define ARRAY_PREFIX(name) intarray_##name
//#include <array.h>

struct nodesIdxNode {
	int64_t key;
	int idx;
};
struct waysIdxNode {
	int64_t key;
	int idx;
};

#define HASH_NODE struct nodesIdxNode
#define HASH_PREFIX(name) nodesIdx_##name
#define HASH_KEY_ATOMIC key
#define HASH_ATOMIC_TYPE int64_t
#define HASH_DEFAULT_SIZE 100000
#define HASH_WANT_CLEANUP
#define HASH_WANT_FIND
#define HASH_WANT_NEW
#include <ucw/hashtable.h>

#define HASH_NODE struct waysIdxNode
#define HASH_PREFIX(name) waysIdx_##name
#define HASH_KEY_ATOMIC key
#define HASH_ATOMIC_TYPE int64_t
#define HASH_DEFAULT_SIZE 100000
#define HASH_WANT_CLEANUP
#define HASH_WANT_FIND
#define HASH_WANT_NEW
#include <ucw/hashtable.h>

void nodesIdx_refresh(int n_nodes, Premap__Node ** nodes){
	nodesIdx_cleanup();
	nodesIdx_init();
	for (int i=0;i<n_nodes;i++){
		struct nodesIdxNode * val;
		val = nodesIdx_new(nodes[i]->id);
		val->idx = i;
	}
}

void waysIdx_refresh(int n_ways, Premap__Way ** ways){
	waysIdx_cleanup();
	waysIdx_init();
	for (int i=0;i<n_ways;i++){
		struct waysIdxNode * val;
		val = waysIdx_new(ways[i]->id);
		val->idx = i;
	}
}

int main (int argc, char ** argv){
	
	FILE * IN;
	IN = fopen("../scripts/filter/praha-union.pbf","r");
	if (IN==NULL){
		printf("File opening error\n");	
		return 1;
	}
	fseek(IN,0,SEEK_END);
	long int len;
	len = ftell(IN);
	fseek(IN,0,SEEK_SET);
	uint8_t * buf;
	buf = (uint8_t *)malloc(len);
	printf("Allocated %d memory\n",len);
	fread(buf,1,len,IN);
	fclose(IN);
	
	Premap__Map *map;
	map = premap__map__unpack(NULL,len,buf);
	if (map==NULL){
		printf("Error unpacking protocolbuffer\n");	
		return 2;
	}
	printf("Loaded %d nodes, %d ways\n",map->n_nodes,map->n_ways);
	nodesIdx_refresh(map->n_nodes,map->nodes);
	waysIdx_refresh(map->n_ways,map->ways);
	printf("%d",map->nodes[nodesIdx_find(1132352548)->idx]->id);
	
	return 0;
}
