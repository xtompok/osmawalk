#ifndef MMQUEUE_H
#define MMQUEUE_H

#include "include/graph.pb-c.h"
#include <time.h>
#include "types.h"

void prepareMMNode(struct mmdijnode_t * node,int idx,int fromDijIdx,int fromEdgeIdx,Objtype fromEdgeType,time_t time,time_t departed,double penalty);
void addFirstNodeToQueue(struct mmqueue_t * q,int idx,time_t time);
void addNodeToQueue(struct mmqueue_t * q,
		int idx,
		int fromEdgeIdx,
		char fromEdgeType,
		time_t time,
		time_t departed,
		double penalty);

struct mmdijnode_t * getQueueMin(struct mmqueue_t * q);
struct mmqueue_t * createMMQueue(Graph__Graph * graph);
void freeMMQueue(struct mmqueue_t * queue,int n_vertices);

#endif
