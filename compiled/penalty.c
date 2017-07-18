#include <float.h>
#include <stdio.h>

#include "include/types.pb-c.h"
#include "penalty.h"

double calcChangePenalty(Graph__Graph * graph, struct config_t conf, struct ptedge_t * toEdge, struct mmdijnode_t * point,uint64_t time){

	if (time < conf.pt_min_wait){
		return PENALTY_INFINITY;	
	} 
	if (point->state.vehicles == conf.pt_max_vehicles){
		return PENALTY_INFINITY;	
	}
	double penalty;
	penalty = time;
	penalty += conf.pt_geton_penalty;
	if (point->edge->edge_type == EDGE_TYPE_WALK){
		if (point->edge->osmedge->type == OBJTYPE__DIRECT_UNDERGROUND){
			penalty+=180;	
		}
	}
	return penalty;	
}
double calcPointPenalty(Graph__Graph * graph,struct config_t conf, Graph__Vertex * vert){
	if (vert->type == OBJTYPE__BARRIER){
		return PENALTY_INFINITY;	
	}
	if (vert->type == OBJTYPE__TRAFFIC_LIGHTS){
		return 40;	
	}
	return 1;
	
}
double calcTransportPenalty(Graph__Graph * graph, struct config_t conf, struct stop_route * r,time_t arrival){
//	printf("Transport penalty:(%ld %ld %f\n",arrival,r->departure,(arrival-r->departure));
	double penalty = 0;
	for (int i=0;i<conf.pt_n_line_penalties;i++){
		if (strcmp(r->pbroute->name, conf.pt_line_penalties[i].name)==0){
			penalty = conf.pt_line_penalties[i].penalty;
		}
	}
	penalty += conf.pt_fixed_penalties[r->pbroute->type];
	penalty += conf.pt_time_penalties[r->pbroute->type]*(arrival-r->departure);
	return penalty;
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
