#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "hashes.h"
#include <float.h>

int utm2wgs(struct search_data_t data,double * lon, double * lat){
	int res;
	res= pj_transform(data.pj_utm,data.pj_wgs84,1,1,lon,lat,NULL);
	*lon = (*lon * 180)/M_PI;
	*lat = (*lat * 180)/M_PI;
	return res;
}
int wgs2utm(struct search_data_t data,double * lon, double * lat){
	*lon = (*lon/180)*M_PI;
	*lat = (*lat/180)*M_PI;
	return pj_transform(data.pj_wgs84,data.pj_utm,1,1,lon,lat,NULL);
}

double calcWeight(Graph__Graph * graph, struct config_t conf, Graph__Edge * edge){
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


struct bbox_t getMapBBox(struct search_data_t * data){
	Graph__Graph * graph;
	int64_t minlon;
	int64_t minlat;
	int64_t maxlon;
	int64_t maxlat;

	graph = data->graph;
	
	minlon = graph->vertices[0]->lon;
	maxlon = graph->vertices[0]->lon;
	minlat = graph->vertices[0]->lat;
	maxlat = graph->vertices[0]->lat;
	for (int i=0;i<graph->n_vertices;i++){
		int64_t lon;
		int64_t lat;
		lon = graph->vertices[i]->lon;
		lat = graph->vertices[i]->lat;

		minlon=(lon<minlon)?lon:minlon;
		minlat=(lat<minlat)?lat:minlat;
		maxlon=(lon>maxlon)?lon:maxlon;
		maxlat=(lat>maxlat)?lat:maxlat;
	}

	struct bbox_t bbox;
	bbox.minlon=minlon;
	bbox.minlat=minlat;
	bbox.maxlon=maxlon;
	bbox.maxlat=maxlat;
	utm2wgs(*data,&bbox.minlon,&bbox.minlat);
	utm2wgs(*data,&bbox.maxlon,&bbox.maxlat);
	return bbox;
}


void printMapBBox(struct search_data_t data){
	Graph__Graph * graph;
	int64_t minlon;
	int64_t minlat;
	int64_t maxlon;
	int64_t maxlat;

	graph = data.graph;
	
	minlon = graph->vertices[0]->lon;
	maxlon = graph->vertices[0]->lon;
	minlat = graph->vertices[0]->lat;
	maxlat = graph->vertices[0]->lat;
	for (int i=0;i<graph->n_vertices;i++){
		int64_t lon;
		int64_t lat;
		lon = graph->vertices[i]->lon;
		lat = graph->vertices[i]->lat;

		minlon=(lon<minlon)?lon:minlon;
		minlat=(lat<minlat)?lat:minlat;
		maxlon=(lon>maxlon)?lon:maxlon;
		maxlat=(lat>maxlat)?lat:maxlat;
	}

	printf("Bounding box in UTM: (%lld,%lld), (%lld,%lld)\n",minlat,minlon,maxlat,maxlon);
	double fminlon=minlon;
	double fminlat=minlat;
	double fmaxlon=maxlon;
	double fmaxlat=maxlat;
	utm2wgs(data,&fminlon,&fminlat);
	utm2wgs(data,&fmaxlon,&fmaxlat);
	printf("Bounding box in WGS-84: (%lf,%lf), (%lf,%lf)\n",fminlat,fminlon,fmaxlat,fmaxlon);
}

char * stopNameFromOSMId(struct search_data_t * data,uint64_t osmid){
	struct osmId2sIdxNode * stopsNode;
	int raptor_id;
	stopsNode = osmId2sIdx_find2(osmid);
	raptor_id = data->graph->stops[stopsNode->idx]->raptor_id;
	return data->timetable->stops[raptor_id]->name;
}
