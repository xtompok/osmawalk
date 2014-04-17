#ifndef RASTER_H
#define RASTER_H

#include <stdlib.h>

#include "types.h"


struct raster_t makeRaster(struct map_t map);

int * getRasterBox(struct raster_t raster,int64_t lon, int64_t lat);


#endif
