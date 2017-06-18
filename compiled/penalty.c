#include <float.h>
#include <stdio.h>

#include "penalty.h"
double calcTransportPenalty(Graph__Graph * graph, struct config_t conf, struct stop_route * r,time_t arrival){
//	printf("Transport penalty:(%ld %ld %f\n",arrival,r->departure,(arrival-r->departure));
	return (arrival-r->departure);
}

double calcWalkPenalty(Graph__Graph * graph, struct config_t conf, Graph__Edge * edge){
//	printf("Walk penalty: %f\n", calcTime(graph,conf,edge)*conf.penalties[edge->type]);
	return calcTime(graph,conf,edge)*conf.penalties[edge->type];
}

double calcTime(Graph__Graph * graph, struct config_t conf,Graph__Edge * edge){
	double speed;
	speed = conf.speeds[edge->type];
	if (speed==0){
		printf("Speed not defined for %lld (type %d)\n",edge->osmid,edge->type);
		return DBL_MAX;
	}
	int fromHeight;
	int toHeight;
	fromHeight = graph->vertices[edge->vfrom]->height;
	toHeight = graph->vertices[edge->vto]->height;
	int dh = toHeight-fromHeight;
	//return (edge->dist+abs(dh)*(dh>0?conf.upscale:conf.downscale))/speed;
	if (edge->dist > 100){
		printf("Dist too big");
	}
	return (edge->dist)/speed;
}
