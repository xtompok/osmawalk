#ifndef UTILS_H
#define UTILS_H
#define SCALE 1000000
static inline double int2deg(int intdeg){
	return 1.0*intdeg/SCALE;		
}

static inline int deg2int(double deg){
	return (int)(deg*SCALE);	
}
#endif
