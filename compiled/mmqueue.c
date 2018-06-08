#include <ucw/lib.h>

#include <err.h>
#include <stdio.h>
#include <assert.h>

#include <ucw/heap.h>
#include <ucw/gary.h>
#include <ucw/mempool.h>

#include "mmqueue.h"
#include "hashes.h"



#define DIJ_CMP(x,y) (x->arrival < y->arrival)
#define DIJ_SWAP(heap,a,b,t) \
	(/*assert(heap[a]->heapidx == a), assert(heap[b]->heapidx == b),*/\
	heap[a]->heapidx=b,heap[b]->heapidx=a,t=heap[a], heap[a]=heap[b], heap[b]=t)

#define MMHEAP_INSERT(elem) \
	HEAP_INSERT(struct mmdijnode_t *,q->heap,q->n_heap,DIJ_CMP,DIJ_SWAP,(elem))
#define MMHEAP_DELETE_MIN() \
	HEAP_DELETE_MIN(struct mmdijnode_t *,q->heap,q->n_heap,DIJ_CMP,DIJ_SWAP)
#define MMHEAP_DELETE(idx) \
	HEAP_DELETE(struct mmdijnode_t *,q->heap,q->n_heap,DIJ_CMP,DIJ_SWAP,(idx))
#define MMHEAP_DECREASE(elem,val) \
	HEAP_DECREASE(q->heap,q->n_heap,DIJ_CMP,DIJ_SWAP,(elem),(val))

void prepareMMNode(struct mmdijnode_t * node){
	node->reached = 1;
	node->completed = 0;
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
	node->state.vehicles = 0;
	
	// Add to vertlut
	struct mmdijnode_t ** vertlutItem;
	vertlutItem = GARY_PUSH(q->vertlut[v->idx]);
	*vertlutItem = node;

	// Add to heap
	GARY_PUSH(q->heap);
	node->heapidx = q->n_heap+1;
	MMHEAP_INSERT(node);
	q->vert = q->heap[1];
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
	if (penalty < 0){
		errx(2,"Penalty %f is negative!");	
	}
	// Check if it is not majorized by another node already in queue
	// Check if it majorizes some nodes in queue and mark them as majorized
	struct mmdijnode_t ** vertnodes;
	vertnodes = q->vertlut[v->idx];
	//printf("VN: %p %d\n",vertnodes,GARY_SIZE(vertnodes));
	for (int j=0;j<GARY_SIZE(vertnodes);j++){
		struct mmdijnode_t * anode;
		anode = vertnodes[j];
		if ((arrival >= anode->arrival)&&(penalty >= anode->penalty)){
			return;
		}
		if ((arrival <= anode->arrival)&&(penalty <= anode->penalty)){
		// Note: probably fixed bug
		//	if (anode->heapidx > q->n_heap || anode->heapidx == 0){
		//		printf("Error, heapidx greater than heap: %d %d\n",anode->heapidx,q->n_heap);
					
		//	}else {
			if (anode->heapidx > 0){
				MMHEAP_DELETE(anode->heapidx);
				anode->heapidx = -1;
				vertnodes[j] = vertnodes[GARY_SIZE(vertnodes)-1];
				GARY_POP(vertnodes);
			}
		//	}
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
	node->state = node->prev->state;
	if (e->edge_type == EDGE_TYPE_PT){
		node->state.vehicles++;
	}
	/*
	for (int j=0;j<GARY_SIZE(vertnodes);j++){
		if (vertnodes[j]->majorized){
			continue;
		}
		if (equivWays(node,vertnodes[j])){
			if (node->penalty < vertnodes[j]->penalty){
				vertnodes[j]->majorized = 1;	
			}else {
				free(e);
				return;
			}
		}
	}
	*/
	// Add to vertlut
	struct mmdijnode_t  ** vertlutItem;
	vertlutItem = GARY_PUSH(q->vertlut[v->idx]);
	*vertlutItem = node;

	// Add to heap
	//printf("ELEM\n");
	GARY_PUSH(q->heap);
	node->heapidx = q->n_heap+1;
	MMHEAP_INSERT(node);
}

struct mmdijnode_t * getQueueMin(struct mmqueue_t * q){
	if (q->n_heap==0){
		err(2,"Heap empty");
	}

	MMHEAP_DELETE_MIN();
	q->vert = q->heap[q->n_heap+1];
	q->vert->heapidx = -1;
	GARY_POP(q->heap);

	if (!q->vert->reached){
		err(1,"Found unreached vertex, exitting\n");
	}
	
	return q->vert;
}

struct mmqueue_t * createMMQueue(Graph__Graph * graph){
	
	struct mmqueue_t * q;
	q = malloc(sizeof(struct mmqueue_t));
	q->n_heap = 0;
	q->pool = mp_new(sizeof(struct mmdijnode_t));
	q->graph = graph;
	

	GARY_INIT_SPACE(q->heap,graph->n_vertices/4);
	GARY_PUSH(q->heap);	// First item is unused

	q->vertlut = calloc(sizeof(struct mmdijnode_t **),graph->n_vertices);
	for (int i=0;i<graph->n_vertices;i++){
		GARY_INIT_SPACE(q->vertlut[i],10);	
	}

	
	//GARY_INIT_SPACE(q->dijArray,graph->n_vertices/3);
	return q;
	
} 
void freeMMQueue(struct mmqueue_t * queue,int n_vertices){
	GARY_FREE(queue->heap);
	for (int i=0;i<n_vertices;i++){
		GARY_FREE(queue->vertlut[i]);	
	}
	free(queue->vertlut);
	//GARY_FREE(queue->dijArray);
	mp_delete(queue->pool);
	free(queue);
}

char equivWays(struct mmdijnode_t * way1, struct mmdijnode_t * way2){
	int topt1;
	int topt2;
	topt1 = 0;
	topt2 = 0;
	struct mmdijnode_t * n1;
	struct mmdijnode_t * n2;
	n1 = way1;
	n2 = way2;
	while ( n1->edge && n1->edge->edge_type == ROUTE_TYPE_WALK){
		topt1++;	
		n1 = n1->prev;
	}
	while ( n2->edge && n2->edge->edge_type == ROUTE_TYPE_WALK){
		topt2++;	
		n2 = n2->prev;
	}
	// One way ends and other changes from public transport
	if (n1->edge && !n2->edge){
		//printf("MHD vs start\n");
		return 0;
	}
	if (!n1->edge && n2->edge){
		//printf("MHD vs start\n");
		return 0;
	}
	// Both ways changes from public transport and connection is different
	if ((n1->edge && n2->edge) && (
		n1->edge->ptedge->route != n2->edge->ptedge->route || 
		n1->edge->ptedge->departure != n2->edge->ptedge->departure)){
		//printf("Different public transport\n");
		return 0;
			
	}
	Graph__Edge ** buf1;
	Graph__Edge ** buf2;
	buf1 = calloc(sizeof(Graph__Edge *),topt1);
	buf2 = calloc(sizeof(Graph__Edge *),topt2);
	n1 = way1;
	n2 = way2;
	for (int i=0;i<topt1;i++)
	{
		buf1[i] = n1->edge->osmedge;	
		n1 = n1->prev;
	}
	for (int i=0;i<topt2;i++)
	{
		buf2[i] = n2->edge->osmedge;	
		n2 = n2->prev;
	}
	while (buf1[topt1-1] == buf2[topt2-1]){
		topt1--;
		topt2--;	
	}
	int frompt1;
	int frompt2;
	frompt1 = 0;
	frompt2 = 0;
	while (buf1[frompt1] == buf2[frompt2] && frompt1 < topt1 && frompt2 < topt2){
		frompt1++;
		frompt2++;
	}

	//printf("From1: %d, to1: %d, from2: %d, to2: %d\n",frompt1,topt1,frompt2, topt2);

	n1 = way1;
	n2 = way2;
	for (int i=0;i<(frompt1 + (topt1-frompt1)/2);i++){
		n1 = n1->prev;	
	}
	for (int i=0;i<(frompt2 + (topt2-frompt2)/2);i++){
		n2 = n2->prev;	
	}
	double dlon = n1->osmvert->lon - n2->osmvert->lon;
	double dlat = n1->osmvert->lat - n2->osmvert->lat;
	double dist = sqrt(dlon*dlon + dlat*dlat);
	free(buf1);
	free(buf2);

	//printf("Distance: %f\n",dist);
	if (dist < 100){
	//	printf("Equivalent\n");
		return 1;	
	}
	//printf("Different\n");
	
	return 0;


}
