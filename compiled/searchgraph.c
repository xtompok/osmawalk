#include <ucw/lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucw/gary.h>
#include "hashes.c"
#include "utils.h"
#include "searchgraph.h"

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
	queue[0] = nodesIdx_find(graph->vertices[0]->osmid)->idx;

	while((nodescnt < graph->n_vertices/2)){
		lastIdx = 0;
		nodescnt = 0;

		for (int i =0;i<GARY_SIZE(seen);i++)
			if (seen[i])
				seen[i]=2;
		seen[queue[0]]=1;
		
		while (lastIdx >= 0){
			int vIdx;
			vIdx = queue[lastIdx--];
			for (int i=0;i<vertexEdges[vIdx].n_edges;i++){
				int neigh;
				neigh = graph->edges[vertexEdges[vIdx].edges[i]]->vto;
				if (!seen[neigh]){
					nodescnt++;
					seen[neigh]=1;
					queue[++lastIdx]=neigh;
				}
			}
		}

		lastIdx = 0;
		while(lastIdx<GARY_SIZE(seen)&&seen[lastIdx]){
//			printf("%d\n",seen[lastIdx]);
			lastIdx++;
		}
		if (lastIdx==GARY_SIZE(seen))
			break;
		queue[0]=lastIdx;
	}

	//printf("Nodes: %d\n",nodescnt);

	Graph__Vertex ** newVertices;
	newVertices = malloc(sizeof(Graph__Vertex *)*nodescnt);
	
	int * newVertIdxs;
	newVertIdxs = malloc(sizeof(int)*graph->n_vertices);
	int oldIdx;
	oldIdx = 0;
	for (int i=0;i<nodescnt;i++){
		while(!(seen[oldIdx]%2))
			oldIdx++;
		newVertices[i]=graph->vertices[oldIdx];
		newVertIdxs[oldIdx]=i;
	//	printf("%d -> %d\n",oldIdx,i);
		oldIdx++;

	}
	graph->n_vertices = nodescnt;
	graph->vertices = newVertices;

	Graph__Edge ** newEdges;
	GARY_INIT(newEdges,0);
	for (int i=0;i<graph->n_edges;i++){
		if (seen[graph->edges[i]->vfrom]==1){
			Graph__Edge ** ePtr;
			ePtr = GARY_PUSH(newEdges);
			*ePtr = graph->edges[i];
			(*ePtr)->vfrom=newVertIdxs[(*ePtr)->vfrom];
			(*ePtr)->vto=newVertIdxs[(*ePtr)->vto];
		}
	
	}
	
	graph->n_edges = GARY_SIZE(newEdges);
	graph->edges = newEdges;

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

// Make search graph from map
Graph__Graph * makeSearchGraph(struct map_t map){
	Graph__Graph * graph;
	graph = malloc(sizeof(Graph__Graph));
	graph__graph__init(graph);
	Graph__Vertex ** vertices;
	vertices = malloc(sizeof(Graph__Vertex *)*map.n_nodes);
	for (int i=0;i<map.n_nodes;i++){
		Graph__Vertex * v;
		v = malloc(sizeof(Graph__Vertex));
		graph__vertex__init(v);
		v->idx = i;
		v->osmid = map.nodes[i]->id;
		v->lat = int2deg(map.nodes[i]->lat);
		v->lon = int2deg(map.nodes[i]->lon);
		v->height = map.nodes[i]->height;
		v->has_height = true;
		vertices[i]=v;
	}
	graph->vertices = vertices;
	graph->n_vertices = map.n_nodes;

	Graph__Edge ** edges;
	GARY_INIT(edges,0);
	int edgecnt;
	edgecnt = 0;
	for (int i=0;i<map.n_ways;i++){
		Premap__Way * way;
		way = map.ways[i];
		for (int j=0;j<(way->n_refs-1);j++){
			int fromIdx;
			int toIdx;
			fromIdx = nodesIdx_find(way->refs[j])->idx;
			toIdx = nodesIdx_find(way->refs[j+1])->idx;
			if ((fromIdx > graph->n_vertices)||(toIdx > graph->n_vertices))
				printf("Wrong index: %d,%d (of %d)\n",fromIdx,toIdx,graph->n_vertices);
			Graph__Edge * e;
			e = malloc(sizeof(Graph__Edge));
			graph__edge__init(e);
			e->idx = edgecnt++;
			e->osmid = way->id;
			e->vfrom = fromIdx;
			e->vto = toIdx;
			e->type = way->type;
			Graph__Edge ** eptr;
			eptr = GARY_PUSH(edges);
			*eptr = e;

			e = malloc(sizeof(Graph__Edge));
			graph__edge__init(e);
			e->idx = edgecnt++;
			e->osmid = way->id;
			e->vfrom = toIdx;
			e->vto = fromIdx;
			e->type = way->type;
			eptr = GARY_PUSH(edges);
			*eptr = e;
		}
			
	}
	graph->edges = edges;
	graph->n_edges = GARY_SIZE(edges);
	return graph;
	
	
}
