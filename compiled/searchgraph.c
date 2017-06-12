#include <ucw/lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucw/gary.h>
#include "hashes.h"
#include "searchgraph.h"
#include "searchlib.h"

// Make for each vertex array of edges on which vertex lies
struct vertexedges_t * makeVertexEdges(Graph__Graph * graph){
	struct vertexedges_t * vertexEdges;
	vertexEdges = malloc(sizeof(struct vertexedges_t)*graph->n_vertices);
	printf("Vertices: %d\n",graph->n_vertices);
	for (int i=0;i<graph->n_vertices;i++){
		vertexEdges[i].n_edges=0;
//		printf("%f %f\n",graph->vertices[i]->lat,graph->vertices[i]->lon);
	}
	for (int i=0;i<graph->n_edges;i++){
		vertexEdges[graph->edges[i]->vfrom].n_edges++;
		vertexEdges[graph->edges[i]->vto].n_edges++;
	}
	for (int i=0;i<graph->n_vertices;i++){
		vertexEdges[i].edges = malloc(sizeof(int)*vertexEdges[i].n_edges);
	//	printf("V: %lld %lld E:%d\n",graph->vertices[i]->osmid,graph->vertices[i]->idx,vertexEdges[i].n_edges);
		vertexEdges[i].n_edges = 0;
	}
	for (int i=0;i<graph->n_edges;i++){
		int vfIdx;
		int vtIdx;
		vfIdx = graph->edges[i]->vfrom;
		vtIdx = graph->edges[i]->vto;
		vertexEdges[vfIdx].edges[vertexEdges[vfIdx].n_edges]= i;
		vertexEdges[vfIdx].n_edges++;
		vertexEdges[vtIdx].edges[vertexEdges[vtIdx].n_edges]= i;
		vertexEdges[vtIdx].n_edges++;
	}
	return vertexEdges;
}

// Select largest component
void largestComponent(Graph__Graph * graph, struct vertexedges_t *  vertexEdges){
	uint8_t * seen;
	GARY_INIT_ZERO(seen,graph->n_vertices);
	int * queue;
	GARY_INIT_ZERO(queue,graph->n_vertices);
	int lastIdx;
	int nodescnt;

	nodescnt=0;
	queue[0] = nodesIdx_find2(graph->vertices[0]->osmid)->idx;

	while((nodescnt < graph->n_vertices/2)){
		lastIdx = 0;
		nodescnt = 0;

		// Set mark to 2 to already done components
		for (int i =0;i<GARY_SIZE(seen);i++)
			if (seen[i])
				seen[i]=2;

		nodescnt++;
		seen[queue[0]]=1;
		
		// BFS over component 
		while (lastIdx >= 0){
			int vIdx;
			vIdx = queue[lastIdx--];
			for (int i=0;i<vertexEdges[vIdx].n_edges;i++){
				int neigh;
				neigh = graph->edges[vertexEdges[vIdx].edges[i]]->vto;
				if (neigh == vIdx){
					neigh = graph->edges[vertexEdges[vIdx].edges[i]]->vfrom;
				}
				
				if (seen[neigh])
					continue;
				
				nodescnt++;
				seen[neigh]=1;
				queue[++lastIdx]=neigh;	
			}
		}

		// Find next root for BFS
		lastIdx = 0;
		while(lastIdx<GARY_SIZE(seen)&&seen[lastIdx]){
//			printf("%d\n",seen[lastIdx]);
			lastIdx++;
		}
		queue[0]=lastIdx;
	
		// If none component has more than half vertices, return last
		if (lastIdx==GARY_SIZE(seen)){
			printf("None component larger then V/2 found, using last\n");
			break;
		}
	}

	//printf("Nodes: %d\n",nodescnt);

	Graph__Vertex ** newVertices;
	newVertices = malloc(sizeof(Graph__Vertex *)*nodescnt);
	
	int * newVertIdxs;
	newVertIdxs = malloc(sizeof(int)*graph->n_vertices);
	for (int i=0;i<graph->n_vertices;i++){
		newVertIdxs[i]=-1;
	}
	int oldIdx;
	oldIdx = 0;
	for (int i=0;i<nodescnt;i++){
		while(!(seen[oldIdx]%2))
			oldIdx++;
		newVertices[i]=graph->vertices[oldIdx];
		newVertIdxs[oldIdx]=i;
		oldIdx++;
	}

	// Set new vertices
	graph->n_vertices = nodescnt;
	graph->vertices = newVertices;

	// Update edges
	Graph__Edge ** newEdges;
	GARY_INIT(newEdges,0);
	for (int i=0;i<graph->n_edges;i++){
		if (seen[graph->edges[i]->vfrom]==1){
			Graph__Edge ** ePtr;
			ePtr = GARY_PUSH(newEdges);
			*ePtr = graph->edges[i];
			if (newVertIdxs[(*ePtr)->vfrom]<0){
				printf("%d -> -1\n",(*ePtr)->vfrom);
			}
			if (newVertIdxs[(*ePtr)->vto]<0){
				printf("%d -> -1\n",(*ePtr)->vto);
			}
			(*ePtr)->vfrom=newVertIdxs[(*ePtr)->vfrom];
			(*ePtr)->vto=newVertIdxs[(*ePtr)->vto];
		}
	
	}
	
	graph->n_edges = GARY_SIZE(newEdges);
	graph->edges = newEdges;

	nodesIdx_refresh(graph->n_vertices,graph->vertices);

	// Update stops
	int stopscnt;
	stopscnt = 0;
	for (int i=0;i<graph->n_stops;i++){
		if (nodesIdx_find2(graph->stops[i]->osmid)!=NULL){
			stopscnt++;
		}
	}

	Graph__Stop ** newStops;
	newStops = malloc(sizeof(Graph__Stop *)*stopscnt);
	int newIdx;
	newIdx = 0;
	for (int i=0;i<graph->n_stops;i++){
		Graph__Stop * stop;
		stop = graph->stops[i];
		if (nodesIdx_find2(stop->osmid)==NULL){
			continue;
		}
		newStops[newIdx]=stop;
		newIdx++;
	}
	graph->n_stops = stopscnt;
	graph->stops = newStops;

}
// Save search graph to file
int saveSearchGraph(Graph__Graph * graph,char * filename){
	int64_t len;
	uint8_t * buf;
	len = graph__graph__get_packed_size(graph);
	buf = malloc(len);
	graph__graph__pack(graph,buf);
	FILE * OUT;
	OUT = fopen(filename,"w");
	if (OUT==NULL){
		printf("File opening error\n");	
		return 1;
	}
	fwrite(buf,1,len,OUT);
	fclose(OUT);
	return 0;
}
	
