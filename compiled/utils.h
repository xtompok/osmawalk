#ifndef UTILS_H
#define UTILS_H
#define SCALE 10

#include <math.h>

#include "types.h"
#include "include/premap.pb-c.h" 

static inline double int2deg(int intdeg){
	return (1.0*intdeg)/SCALE;		
}

static inline int deg2int(double deg){
	return (int)(deg*SCALE);	
}
static inline unsigned int sameStart(struct line_t l1, struct line_t l2){
	return ((l1.startlon==l2.startlon) && (l1.startlat==l2.startlat));
}
static inline unsigned int sameEnd(struct line_t l1, struct line_t l2){
	return ((l1.endlon==l2.endlon) && (l1.endlat==l2.endlat)) ;
}
unsigned int isDirectableWay(Premap__Way * way);
unsigned int isDirectableNode(Premap__Node * node);
unsigned int isBarrier(Premap__Way * way);
unsigned int isWay(Premap__Way * way);
int isWalkArea(Premap__Way * way);

static inline int distance(Premap__Node * node1, Premap__Node * node2){
	uint64_t dlon;
	uint64_t dlat;
	dlon = node2->lon-node1->lon;
	dlat = node2->lat-node1->lat;
	return (int)(sqrt(dlon*dlon+dlat*dlat)/10);
}

static inline struct line_t makeLine(int64_t startlon,int64_t startlat, int64_t endlon, int64_t endlat, int64_t startid, int64_t endid){
	struct line_t line;
	line.startlon = startlon;
	line.startlat = startlat;
	line.endlon = endlon;
	line.endlat = endlat;
	line.startid = startid;
	line.endid = endid;
	return line;
}

#endif
