#ifndef PENALTY_H
#define PENALTY_H

#include "types.h"
#include "include/graph.pb-c.h"

//double calcChangePenalty(graph,data.conf,q->vert->fromEdgeType,way);	
//double calcWaitPenalty(graph,data.conf,fromway,toway,point,time
//double calcPointPenalty(graph,data.conf,point

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
