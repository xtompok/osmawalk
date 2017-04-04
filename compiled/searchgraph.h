#include "include/graph.pb-c.h"
#include "types.h"
struct vertexedges_t{
	int n_edges;
	int * edges;
};
// Make for each vertex array of edges on which vertex lies
struct vertexedges_t * makeVertexEdges(Graph__Graph * graph);
// Select largest component
void largestComponent(Graph__Graph * graph, struct vertexedges_t *  vertexEdges);
// Save search graph to file
int saveSearchGraph(Graph__Graph * graph,char * filename);
