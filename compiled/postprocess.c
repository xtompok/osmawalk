#include <ucw/lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucw/gary.h>
#include "postprocess.h"
#include "writegpx.h"
#include "utils.h"
#include "hashes.h"
#include "include/result.pb-c.h"


struct search_result_t processFoundMMRoutes(struct search_data_t data, struct mmqueue_t * queue,int fromIdx, int toIdx){
	struct mmdijnode_t * dijArray;
	int ** vertlut;
	dijArray = queue->dijArray;
	vertlut = queue->vertlut;

	int n_routes;
	n_routes = GARY_SIZE(vertlut[toIdx]);
	printf("Found %d routes\n",n_routes);	
	struct search_route_t * routes;
	routes = calloc(sizeof(struct search_route_t),n_routes);
	struct mmdijnode_t * node;
	for (int i=0;i<n_routes;i++){
		node = dijArray+vertlut[toIdx][i];
		routes[i].time = node->time;
		routes[i].dist = node->penalty;
		printf("Route %d: time: %f, penalty: %f\n",i,routes[i].time,routes[i].dist); 
		struct point_t * points;
		GARY_INIT(points,0);
		int memdepart;
		memdepart = -1;
		do {
			struct point_t * point;
			point = GARY_PUSH(points);
			
			double lon;
			double lat;

			lon = data.graph->vertices[node->idx]->lon;
			lat = data.graph->vertices[node->idx]->lat;
			utm2wgs(data,&lon,&lat);
			point->lon = lon;
			point->lat = lat;
			point->height = data.graph->vertices[node->idx]->height;

			if (memdepart != -1){
				point->departure = memdepart; 
			}
			memdepart = node->departed;
			point->arrival = node->time;

			point->vertType = data.graph->vertices[node->idx]->type;
			point->vertId = data.graph->vertices[node->idx]->osmid;

			struct osmId2sIdxNode * stopnode;
			stopnode = osmId2sIdx_find2(point->vertId);
			if (stopnode != NULL ){
				point->stopIdx = stopnode->idx;
			} else {
				point->stopIdx = -1;	
			}

			//edgeType, wayId, routeIdx
			if (node->fromEdgeType == ROUTE_TYPE_WALK ){
				point->edgeType = data.graph->edges[node->fromEdgeIdx]->type;
				point->wayId = 	data.graph->edges[node->fromEdgeIdx]->osmid;
			} else if (node->fromEdgeType == ROUTE_TYPE_PT){
				point->edgeType = OBJTYPE__PUBLIC_TRANSPORT;
				point->routeIdx = node->fromEdgeIdx; 
			} else {
				point->edgeType = OBJTYPE__NONE;	
			}

			node = dijArray + node->fromIdx;
				
		} while (node->idx != fromIdx);

		routes[i].n_points = GARY_SIZE(points);
		routes[i].points = calloc(sizeof(struct point_t),routes[i].n_points);
		printf("Points: %d\n",routes[i].n_points);
		for (int j=0;j<routes[i].n_points;j++){
			routes[i].points[j]=points[routes[i].n_points-1-j];
		}
		GARY_FREE(points);
	}

	struct search_result_t result;
	result.routes = routes;
	result.n_routes = n_routes;
	return result;
}
void writeGPXForResult(struct search_result_t * res){
	for (int i=0;i<res->n_routes;i++){
		char filename[20];
		sprintf(filename,"track_%d.gpx",i);
		writeGpxFile(res->routes[i],filename);	
			
	}
}
void printMMRoutes(struct search_data_t * data,struct search_result_t * res){
	for (int r=0;r<res->n_routes;r++){
		printf("Route %d:\n",r);
		for (int s=0;s<res->routes[r].n_points;s++){
			struct point_t * pt;
			struct point_t * prevpt;
			pt = res->routes[r].points+s;
			if (s>0){
				prevpt = res->routes[r].points+s-1;
			} else {
				prevpt = NULL;	
			}
			if (pt->edgeType == OBJTYPE__PUBLIC_TRANSPORT){
				char * timestr;
				char * stopstr;
				timestr = prt_time(prevpt->departure);
				stopstr = stopNameFromOSMId(data,prevpt->vertId);
				printf("From: %s at %s\n",stopstr,timestr);
				free(timestr);
				timestr = prt_time(pt->arrival % (24*3600));
				stopstr = stopNameFromOSMId(data,pt->vertId);
				printf("To: %s at %s by %d\n",stopstr,timestr,pt->routeIdx);
				free(timestr);
			}
		}
	}
	
}

struct pbf_data_t generatePBF(struct search_result_t * result){
	struct search_route_t * routes;
	routes = result->routes;
	Result__Result * pbRoutes;
	pbRoutes = malloc(sizeof(Result__Result));
	result__result__init(pbRoutes);
	pbRoutes->n_routes = result->n_routes;			
	pbRoutes->routes = calloc(sizeof(Result__Route *),result->n_routes);
	for (int i=0;i<result->n_routes;i++){
		pbRoutes->routes[i] = malloc(sizeof(Result__Route));
		Result__Route * r;
		r = pbRoutes->routes[i];
		result__route__init(r);
		r->time = routes[i].time;
		r->dist = routes[i].dist;
		r->n_points = routes[i].n_points;
		r->points = calloc(sizeof(Result__Point *),r->n_points);	
		for (int j=0; j<r->n_points;j++){
			r->points[j] = malloc(sizeof(Result__Point));
			Result__Point * pbPt;
			struct point_t * pt;
			pt = routes[i].points + j;
			pbPt = r->points[j];
			result__point__init(pbPt);

			pbPt->lat = pt->lat;
			pbPt->lon = pt->lon;
			pbPt->height = pt->height;
			
			pbPt->departure = pt->departure;
			pbPt->arrival = pt->arrival;
			
			pbPt->verttype = pt->vertType;
			pbPt->has_vertid = 1;
			pbPt->vertid = pt->vertId;
			if (pt->stopIdx != -1){
				pbPt->has_stopidx = 1;
				pbPt->stopidx = pt->stopIdx;
			}

			pbPt->edgetype = pt->edgeType;		
			if (pt->edgeType == ROUTE_TYPE_WALK ){
				pbPt->has_wayid = 1;
				pbPt->wayid = pt->wayId;
			} else if (pt->edgeType == ROUTE_TYPE_PT){
				pbPt->has_routeidx = 1;
				pbPt->routeidx = pt->routeIdx;
			}	
				
		}


	}

	struct pbf_data_t pbf_packed;
	pbf_packed.len = result__result__get_packed_size(pbRoutes);
	pbf_packed.data = malloc(pbf_packed.len);
	result__result__pack(pbRoutes,pbf_packed.data);

	printf("Len: %d\n",pbf_packed.len);
	return pbf_packed;
	
} 

void writeGpxFile(struct search_route_t result,char * filename){
	if (result.n_points==0){
		return;
	}
	printf("Writing results to file %s\n",filename);
	FILE * OUT;
	OUT = fopen(filename,"w");
	writeGpxHeader(OUT);
	writeGpxStartTrack(OUT);
	for (int i=0;i<result.n_points;i++){
		struct point_t p;
		p = result.points[i];
		writeGpxTrkpt(OUT,p.lat,p.lon,p.height);
	}
	writeGpxEndTrack(OUT);
	writeGpxFooter(OUT);
	fclose(OUT);
}
