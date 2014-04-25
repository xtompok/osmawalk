#ifndef UTILS_H
#define UTILS_H
#define SCALE 1000000

#include "include/premap.pb-c.h" 
static inline double int2deg(int intdeg){
	return 1.0*intdeg/SCALE;		
}

static inline int deg2int(double deg){
	return (int)(deg*SCALE);	
}
unsigned int isDirectable(Premap__Way * way);
unsigned int isBarrier(Premap__Way * way);
unsigned int isWay(Premap__Way * way);
int isWalkArea(Premap__Way * way);
#endif
