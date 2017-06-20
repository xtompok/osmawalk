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
	int n_routes;
	n_routes = GARY_SIZE(queue->vertlut[toIdx]);
	printf("Found %d routes\n",n_routes);	
	struct search_route_t * routes;
	routes = calloc(sizeof(struct search_route_t),n_routes);
	struct mmdijnode_t * node;
	for (int i=0;i<n_routes;i++){
		node = queue->vertlut[toIdx][i];
		routes[i].time = node->arrival;
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

			lon = node->osmvert->lon;
			lat = node->osmvert->lat;
			utm2wgs(data,&lon,&lat);
			point->lon = lon;
			point->lat = lat;
			point->height = node->osmvert->height;

			point->arrival = node->arrival;
			if (memdepart != -1){
				point->departure = memdepart; 
			}else{
				point->departure = point->arrival;
			}
			if (node->edge && node->edge->edge_type == EDGE_TYPE_PT){
				memdepart = node->edge->ptedge->departure;	
			}

			point->penalty = node->penalty;

			point->osmvert = node->osmvert;
			point->stop = node->stop;
			point->edge = node->edge;

			node = node->prev;
				
		} while (node != NULL);

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
		printf("------------------------");
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
			if (pt->edge && pt->edge->edge_type == EDGE_TYPE_PT){
				char * timestr;
				char * stopstr;
				timestr = prt_time(prevpt->departure);
				//stopstr = stopNameFromOSMId(data,prevpt->vertId);
				//printf("From: %s at %s\n",stopstr,timestr);
				printf("From: %s at %s\n",prevpt->stop->name,timestr);
				free(timestr);
				timestr = prt_time(pt->arrival % (24*3600));
				//stopstr = stopNameFromOSMId(data,pt->vertId);
				//printf("To: %s at %s by %d\n",stopstr,timestr,pt->routeIdx);
				printf("To: %s at %s by %s\n",pt->stop->name,timestr,pt->edge->ptedge->route->name);
				free(timestr);
			}
			printf("Time: %lld, penalty: %f\n",pt->arrival,pt->penalty);
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
				
			pbPt->osmvert = malloc(sizeof(Graph__Vertex));
			graph__vertex__init(pbPt->osmvert);

			pbPt->osmvert->osmid = pt->osmvert->osmid ;
			pbPt->osmvert->lat = pt->osmvert->lat ;
			pbPt->osmvert->lon = pt->osmvert->lon ;
			pbPt->osmvert->type = pt->osmvert->type ;
			pbPt->osmvert->height = pt->osmvert->height ;
			pbPt->osmvert->idx = pt->osmvert->idx ;

			if (pt->stop != NULL){
				pbPt->stop = malloc(sizeof(Result__Stop));
				result__stop__init(pbPt->stop);
				pbPt->stop->name = malloc(strlen(pt->stop->name)+1);
				strcpy(pbPt->stop->name,pt->stop->name);
				// TODO stop id 
			}
			if (pt->edge && pt->edge->edge_type == EDGE_TYPE_WALK){
				pbPt->edgetype = RESULT__EDGE_TYPE__WALK;
				pbPt->walkedge = malloc(sizeof(Graph__Edge));
				graph__edge__init(pbPt->walkedge);
				pbPt->walkedge->idx = pt->edge->osmedge->idx ;
				pbPt->walkedge->osmid = pt->edge->osmedge->osmid ;
				pbPt->walkedge->crossing = pt->edge->osmedge->crossing ;
				pbPt->walkedge->type = pt->edge->osmedge->type ;
				// TODO other properties
			
			}else if (pt->edge && pt->edge->edge_type == EDGE_TYPE_PT){
				pbPt->edgetype = RESULT__EDGE_TYPE__PT;	
				pbPt->ptedge = malloc(sizeof(Result__PTEdge));
				result__ptedge__init(pbPt->ptedge);
				pbPt->ptedge->name = pt->edge->ptedge->route->name;
				//pbPr->ptedge->id = pt->edge->ptedge->id;
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