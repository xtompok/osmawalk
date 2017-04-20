#ifndef UTILS_H
#define UTILS_H

#include "types.h"


double calcWeight(Graph__Graph * graph, struct config_t conf, Graph__Edge * edge);

struct bbox_t getMapBBox(struct search_data_t * data);

void printMapBBox(struct search_data_t data);

/*!
 * Converts inplace coordinates from UTM to WGS-84
 * @param data Search data (for projections)
 * @param lon x coordinate
 * @param lat y coordinate
 * @result Result of a PROJ.4 conferting function
 */
int utm2wgs(struct search_data_t data, double * lon, double * lat);
/*!
 * Converts inplace coordinates from WGS-84 to UTM
 * @param data Search data (for projections)
 * @param lon Longitude
 * @param lat Latitude
 * @result Result of a PROJ.4 conferting function
 */
int wgs2utm(struct search_data_t data, double * lon, double * lat);
/*!
 * Calculate time for going through an edge
 * @param graph Search graph
 * @param conf Speeds config
 * @param edge Pointer to an edge
 * @result Time in seconds
 */
double calcTime(Graph__Graph * graph,struct config_t conf,Graph__Edge * edge);

char * stopNameFromOSMId(struct search_data_t * data,uint64_t osmid);

#endif
