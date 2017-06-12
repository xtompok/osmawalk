#include <ucw/lib.h>

#include <err.h>

#include <ucw/heap.h>
#include <ucw/gary.h>

#include "mmqueue.h"
#include "hashes.h"


#define DIJ_CMP(x,y) (q->dijArray[x].time < q->dijArray[y].time)

#define MMHEAP_INSERT(elem) \
	HEAP_INSERT(int,q->heap,q->n_heap,DIJ_CMP,HEAP_SWAP,(elem))
#define MMHEAP_DELETE_MIN() \
	HEAP_DELETE_MIN(int,q->heap,q->n_heap,DIJ_CMP,HEAP_SWAP)
#define MMHEAP_DECREASE(elem,val) \
	HEAP_DECREASE(q->heap,q->n_heap,DIJ_CMP,HEAP_SWAP,(elem),(val))

void prepareMMNode(struct mmdijnode_t * node,int idx,int fromDijIdx,int fromEdgeIdx,Objtype fromEdgeType,time_t time,time_t departed,double penalty){
	node->idx = idx;
	node->fromIdx = fromDijIdx;
	node->fromEdgeIdx = fromEdgeIdx;
	node->fromEdgeType = fromEdgeType;
	node->time = time;
	node->departed = departed;
	node->penalty = penalty;
	node->reached = 1;
	node->completed = 0;
	node->majorized = 0;
} 

void addFirstNodeToQueue(struct mmqueue_t * q,int idx,time_t time){
	struct mmdijnode_t * node;
	node = GARY_PUSH(q->dijArray);

	prepareMMNode(node,idx,-1,-1,ROUTE_TYPE_NONE,time,0,0);
	
	// Add to vertlut
	int * vertlutItem;
	vertlutItem = GARY_PUSH(q->vertlut[idx]);
	*vertlutItem = node-q->dijArray;

	// Add to heap
	GARY_PUSH(q->heap);
	MMHEAP_INSERT(node - q->dijArray);
	q->vert = q->dijArray+q->heap[0];
}
void addNodeToQueue(struct mmqueue_t * q,
		int idx,
		int fromEdgeIdx,
		char fromEdgeType,
		time_t time,
		time_t departed,
		double penalty){
	// Check if it is not majorized by another node already in queue
	// Check if it majorizes some nodes in queue and mark them as majorized
	for (int j=0;j<GARY_SIZE(q->vertlut[idx]);j++){
		struct mmdijnode_t * anode;
		anode = q->dijArray + q->vertlut[idx][j];
		if ((time >= anode->time)&&(penalty >= anode->penalty)){
			return;
		}
		if ((time <= anode->time)&&(penalty <= anode->penalty)){
			anode->majorized=1;
		}
	}

	struct mmdijnode_t * node;

	// Add to dijArray
	int vertIdx;	// Preserve vert pointer through reallocation
	vertIdx = q->vert-q->dijArray;
	node = GARY_PUSH(q->dijArray);
	q->vert = q->dijArray+vertIdx;

	prepareMMNode(node,idx,vertIdx,fromEdgeIdx,fromEdgeType,time,departed,penalty);
	
	// Add to vertlut
	int * vertlutItem;
	vertlutItem = GARY_PUSH(q->vertlut[idx]);
	*vertlutItem = node-q->dijArray;

	// Add to heap
	GARY_PUSH(q->heap);
	MMHEAP_INSERT(node - q->dijArray);
}

struct mmdijnode_t * getQueueMin(struct mmqueue_t * q){
	if (q->n_heap==0){
		err(2,"Heap empty");
	}

	MMHEAP_DELETE_MIN();
	int vIdx;
	vIdx = q->heap[q->n_heap+1];
	GARY_POP(q->heap);
	q->vert = q->dijArray + vIdx;

	if (!q->vert->reached){
		err(1,"Found unreached vertex, exitting\n");
	}

	if (q->vert->majorized){
		return NULL;	
	}

	return q->vert;
}

struct mmqueue_t * createMMQueue(Graph__Graph * graph){
	
	struct mmqueue_t * q;
	q = malloc(sizeof(struct mmqueue_t));
	q->n_heap = 0;
	GARY_INIT_SPACE(q->heap,graph->n_vertices/3);
	GARY_PUSH(q->heap);	// First item is unused

	q->vertlut = calloc(sizeof(int*),graph->n_vertices);
	for (int i=0;i<graph->n_vertices;i++){
		GARY_INIT(q->vertlut[i],0);	
	}

	GARY_INIT_SPACE(q->dijArray,graph->n_vertices/3);
	return q;
	
} 
void freeMMQueue(struct mmqueue_t * queue,int n_vertices){
	GARY_FREE(queue->heap);
	for (int i=0;i<n_vertices;i++){
		GARY_FREE(queue->vertlut[i]);	
	}
	free(queue->vertlut);
	GARY_FREE(queue->dijArray);
	free(queue);
}
