#ifndef POSTPROCESS_H
#define POSTPROCESS_H

#include "types.h"
#include "mmqueue.h"

struct search_result_t processFoundMMRoutes(struct search_data_t data, struct mmqueue_t * queue,int fromIdx, int toIdx);

void writeGPXForResult(struct search_result_t * res);

void printMMRoutes(struct search_data_t * data,struct search_result_t * res);

struct pbf_data_t generatePBF(struct search_result_t * result);

/*! Write searched way to GPX file
 * @param result Structure handlig search result
 * @param filename Filename of the GPX file
 */
void writeGpxFile(struct search_route_t result,char * filename);

#endif
