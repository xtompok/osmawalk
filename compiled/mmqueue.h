#ifndef MMQUEUE_H
#define MMQUEUE_H

#include "include/graph.pb-c.h"
#include <time.h>
#include <ucw/mempool.h>
#include "types.h"

struct ptedge_t{
	uint64_t departure;
	Route * route;
};

#define EDGE_TYPE_WALK 1
#define EDGE_TYPE_PT 2
struct edge_t {
	char edge_type;
	union {
		Graph__Edge * osmedge;
		struct ptedge_t * ptedge;
	};
};

struct search_state_t{
	int vehicles;
};

struct mmdijnode_t {
	struct mmdijnode_t * prev;
	struct mmdijnode_t * nextlut;
	Graph__Vertex * osmvert;
	Stop * stop;
	struct edge_t * edge;
	struct search_state_t state;
	int heapidx;
	bool reached;
	bool completed;
	time_t arrival;
	double penalty;
};

struct mmqueue_t {
	struct mempool * pool;
	Graph__Graph * graph;
	//struct mmdijnode_t * dijArray;
	struct mmdijnode_t * vert;
	struct mmdijnode_t *** vertlut;
	struct mmdijnode_t ** heap;
	int n_heap;		
	
};

void prepareMMNode(struct mmdijnode_t * node);
void addFirstNodeToQueue(struct mmqueue_t * q,Graph__Vertex * v, Stop * s, uint64_t time);
void addNodeToQueue(struct mmqueue_t * q,
		struct mmdijnode_t * prev,
		Graph__Vertex * v,
		Stop * s,
		struct edge_t * e,
		uint64_t arrival,
		double penalty);

struct mmdijnode_t * getQueueMin(struct mmqueue_t * q);
struct mmqueue_t * createMMQueue(Graph__Graph * graph);
void freeMMQueue(struct mmqueue_t * queue,int n_vertices);
char equivWays(struct mmdijnode_t * way1, struct mmdijnode_t * way2);

#endif
