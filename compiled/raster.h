#ifndef RASTER_H
#define RASTER_H

#include <stdlib.h>
#include "types.h"
 /*!
  * @abstract Library for make raster of points in plane
  *
  * @discussion This file provides functions for creating raster from map struct
  * and getting coordinates of a box for point
  *
  */

/*!
 * @abstract Make raster from map struct. 
 *
 * @param map Map struct (see definition in types.h)
 * @return Raster structure
 *
 */
struct raster_t makeRaster(struct map_t map);

/*!
 * @abstract Get coordinates of a box in raster for a point
 *
 * @param raster Raster structure
 * @param lon Longitude of a point
 * @param lat Latitude of a point
 * @return Array of two integers, x and y coordinate of the box, in which the point 
 * lays
 */
int * getRasterBox(struct raster_t raster,int64_t lon, int64_t lat);


#endif
