#ifndef SEARCHLIB_H
#define SEARCHLIB_H
#include <ucw/lib.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <inttypes.h>

#include <ucw/heap.h>
#include <yaml.h>
#include "include/graph.pb-c.h"
#include "libraptor.h"

#include "postprocess.h"
#include "utils.h"
#include "types.h"

/*!
 * @abstract Library for searching path
 *
 * @discussion This library containf functions to load saved graph and speeds
 * configuration and repeatedly searching paths.
 */


/*!
 * Parse config file
 * @param filename Filename of the file
 * @result Speeds configuration
 */
struct config_t parseConfigFile(char * filename);
/*! Load graph from file
 *
 * @param filename Filename of the file
 * @result Graph structure
 */
Graph__Graph * loadMap(char * filename);

/*! Calculate distances of all edges
 * @param graph Graph struct, distances are filled in parameter dist
 */
void calcDistances(Graph__Graph * graph);
/*! Make array with way array for each node
 * @param graph Graph struct
 * @result Array with array of ways for each node on which node lies
 */
struct nodeways_t * makeNodeWays(Graph__Graph * graph);

/*! Find nearest vertex for given coordinates
 * @param graph Search graph
 * @param lon Longitude of a point
 * @param lat Latitude of a point
 * @result Index of nearest vertex
 */
int findNearestVertex(Graph__Graph * graph, double lon, double lat);

/*! Prepare data for Dijkstra searching
 * @param graph
 * @result array of additional structs for each vertex
 */
struct dijnode_t *  prepareDijkstra(Graph__Graph * graph);

/*! Prepare all data for searching 
 * @param configName Name of the configuration file
 * @param dataName Name of the file with searching graph
 * @result Search data structure
 */
struct search_data_t * prepareData(char * configName, char * dataName, char * timetableName);

/*! Find path in graph from coorinates to coordinates
 * @param data Search data
 * @param fromLat Starting point latitude
 * @param fromLon Starting point longitude
 * @param toLat End point latitude
 * @param toLon End point longitude
 * @result Structure for handli search result
 */
struct pbf_data_t findPath(struct search_data_t * data,
		double fromLat, double fromLon, double toLat, double toLon);

struct search_result_t findTransfer(struct search_data_t * data,
		char * from, char * to);

struct bbox_t getMapBBox(struct search_data_t * data);
void printMapBBox(struct search_data_t data);

#endif
