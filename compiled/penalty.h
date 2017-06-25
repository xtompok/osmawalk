#ifndef PENALTY_H
#define PENALTY_H

#include "types.h"
#include "include/graph.pb-c.h"
#include "mmqueue.h"

//double calcChangePenalty(graph,data.conf,q->vert->fromEdgeType,way);	
//double calcWaitPenalty(graph,data.conf,fromway,toway,point,time)
double calcChangePenalty(Graph__Graph * graph, struct config_t conf, struct ptedge_t * toEdge, struct mmdijnode_t * point, uint64_t time);
double calcPointPenalty(Graph__Graph * graph,struct config_t conf, Graph__Vertex *vert);

double calcWalkPenalty(Graph__Graph * graph, struct config_t conf, Graph__Edge * edge);
double calcTransportPenalty(Graph__Graph * graph, struct config_t conf, struct stop_route * r,time_t arrival);

/*!
 * Calculate time for going through an edge
 * @param graph Search graph
 * @param conf Speeds config
 * @param edge Pointer to an edge
 * @result Time in seconds
 */
double calcTime(Graph__Graph * graph,struct config_t conf,Graph__Edge * edge);

#endif
