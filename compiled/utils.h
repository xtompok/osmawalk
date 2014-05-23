#ifndef UTILS_H
#define UTILS_H
#define SCALE 10

#include <math.h>

#include "types.h"
#include "include/premap.pb-c.h" 

/*! Convert internal coordinates to real UTM
 * @param intdeg Internal coordinate
 * @result UTM coordinate
 */
static inline double int2deg(int intdeg){
	return (1.0*intdeg)/SCALE;		
}

/*! Convert real UTM to internal coordinate
 * @param deg UTM coordinate
 * @result Internal coordinate
 */
static inline int deg2int(double deg){
	return (int)(deg*SCALE);	
}
/*! Have two lines same starting point?
 * @param l1 First line
 * @param l2 Second line
 * @result True if they have, else otherwise
 */
static inline unsigned int sameStart(struct line_t l1, struct line_t l2){
	return ((l1.startlon==l2.startlon) && (l1.startlat==l2.startlat));
}
/*! Have two lines same end point?
 * @param l1 First line
 * @param l2 Second line
 * @result True if they have, else otherwise
 */
static inline unsigned int sameEnd(struct line_t l1, struct line_t l2){
	return ((l1.endlon==l2.endlon) && (l1.endlat==l2.endlat)) ;
}
/*! Could be way used for searching direct connections?
 * @param way Way
 * @result True if could, false otherwise
 */
unsigned int isDirectableWay(Premap__Way * way);
/*! Could be node used for searching direct connections?
 * @param way Way
 * @result True if could, false otherwise
 */
unsigned int isDirectableNode(Premap__Node * node);
/*! Is the way barrier?
 * @param way Way
 * @result True if it is, false otherwise
 */
unsigned int isBarrier(Premap__Way * way);
/*! Is the way walk way?
 * @param way Way
 * @result True if it is false otherwise
 */
unsigned int isWay(Premap__Way * way);
/*! Is the way walk area?
 * @param way Way
 * @result True if it is, false otherwise
 */
int isWalkArea(Premap__Way * way);

/*! Calculate eucliedan distance between two nodes
 * @param node1 First node
 * @param node2 Second node
 * @result Euclidean distance between nodes
 */
static inline int distance(Premap__Node * node1, Premap__Node * node2){
	uint64_t dlon;
	uint64_t dlat;
	dlon = node2->lon-node1->lon;
	dlat = node2->lat-node1->lat;
	return (int)(sqrt(dlon*dlon+dlat*dlat)/10);
}

/* Make line 
 * @param startlon Longitude of starting point
 * @param startlat Latitude of starting point
 * @param endlon Longitude of ending point
 * @param endlat Latitude of ending point
 * @param startid Id of a starting point
 * @param endid Id of an ending point
 * @result line structure with passed parameters
 */
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
