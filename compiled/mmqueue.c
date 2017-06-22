#include <ucw/lib.h>

#include <err.h>

#include <ucw/heap.h>
#include <ucw/gary.h>
#include <ucw/mempool.h>

#include "mmqueue.h"
#include "hashes.h"


#define DIJ_CMP(x,y) (x->arrival < y->arrival)

#define MMHEAP_INSERT(elem) \
	HEAP_INSERT(struct mmdijnode_t *,q->heap,q->n_heap,DIJ_CMP,HEAP_SWAP,(elem))
#define MMHEAP_DELETE_MIN() \
	HEAP_DELETE_MIN(struct mmdijnode_t *,q->heap,q->n_heap,DIJ_CMP,HEAP_SWAP)
#define MMHEAP_DECREASE(elem,val) \
	HEAP_DECREASE(q->heap,q->n_heap,DIJ_CMP,HEAP_SWAP,(elem),(val))

void prepareMMNode(struct mmdijnode_t * node){
	node->reached = 1;
	node->completed = 0;
	node->majorized = 0;
/*
	struct mmdijnode_t * prev;
	Graph__Vertex * osmvert;
	struct stop * stop;
	struct edge_t * edge;
	bool reached;
	bool completed;
	bool majorized;
	time_t arrival;
	double penalty;
	*/
} 

void addFirstNodeToQueue(struct mmqueue_t * q,Graph__Vertex * v, Stop * s, uint64_t time){
	struct mmdijnode_t * node;
	node = mp_alloc(q->pool,sizeof(struct mmdijnode_t));

	prepareMMNode(node);
	node->prev = NULL;
	node->osmvert = v;
	node->stop = s;
	node->edge = NULL;
	node->arrival = time;
	node->penalty = 0;
	
	// Add to vertlut
	struct mmdijnode_t ** vertlutItem;
	vertlutItem = GARY_PUSH(q->vertlut[v->idx]);
	*vertlutItem = node;

	// Add to heap
	GARY_PUSH(q->heap);
	MMHEAP_INSERT(node);
	q->vert = q->heap[0];
}
void freeEdge(struct edge_t * e){
	if (e->edge_type == EDGE_TYPE_PT){
		free(e->ptedge);
	}
	free(e);
}

void addNodeToQueue(struct mmqueue_t * q,
		struct mmdijnode_t * prev,
		Graph__Vertex * v,
		Stop * s,
		struct edge_t * e,
		uint64_t arrival,
		double penalty){
	// Check if it is not majorized by another node already in queue
	// Check if it majorizes some nodes in queue and mark them as majorized
	struct mmdijnode_t ** vertnodes;
	vertnodes = q->vertlut[v->idx];
	for (int j=0;j<GARY_SIZE(vertnodes);j++){
		struct mmdijnode_t * anode;
		anode = vertnodes[j];
		if ((arrival >= anode->arrival)&&(penalty >= anode->penalty)){
			free(e);
			return;
		}
		if ((arrival <= anode->arrival)&&(penalty <= anode->penalty)){
			anode->majorized=1;
		}
	}

	struct mmdijnode_t * node;

	// Add to dijArray
	node = mp_alloc(q->pool,sizeof(struct mmdijnode_t));

	prepareMMNode(node);
	node->prev = prev;
	node->osmvert = v;
	node->stop = s;
	node->edge = e;
	node->arrival = arrival;
	node->penalty = penalty;
	
	// Add to vertlut
	struct mmdijnode_t  ** vertlutItem;
	vertlutItem = GARY_PUSH(q->vertlut[v->idx]);
	*vertlutItem = node;

	// Add to heap
	GARY_PUSH(q->heap);
	MMHEAP_INSERT(node);
}

struct mmdijnode_t * getQueueMin(struct mmqueue_t * q){
	if (q->n_heap==0){
		err(2,"Heap empty");
	}

	MMHEAP_DELETE_MIN();
	q->vert = q->heap[q->n_heap+1];
	GARY_POP(q->heap);

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
	q->pool = mp_new(sizeof(struct mmdijnode_t));
	q->graph = graph;
	

	GARY_INIT_SPACE(q->heap,graph->n_vertices/3);
	GARY_PUSH(q->heap);	// First item is unused

	q->vertlut = calloc(sizeof(struct mmdijnode_t **),graph->n_vertices);
	for (int i=0;i<graph->n_vertices;i++){
		GARY_INIT(q->vertlut[i],0);	
	}

	
	//GARY_INIT_SPACE(q->dijArray,graph->n_vertices/3);
	return q;
	
} 
void freeMMQueue(struct mmqueue_t * queue,int n_vertices){
	GARY_FREE(queue->heap);
	for (int i=0;i<n_vertices;i++){
		for (int j=0;j<GARY_SIZE(queue->vertlut[i]);j++){
			struct mmdijnode_t * node;
			node = queue->vertlut[i][j];	
			if (node->edge == NULL){
				continue;	
			}
			if (node->edge->edge_type == EDGE_TYPE_PT){
				free(node->edge->ptedge);	
			}
			free(node->edge);
		}
		GARY_FREE(queue->vertlut[i]);	
	}
	free(queue->vertlut);
	//GARY_FREE(queue->dijArray);
	mp_delete(queue->pool);
	free(queue);
}
