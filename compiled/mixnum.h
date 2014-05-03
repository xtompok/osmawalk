#ifndef MIXNUM_H
#define MIXNUM_H

#include <ucw/lib.h>
#include <stdint.h>
#include "types.h"

void mixedPrint(struct mixed_num_t mix);

static inline int mixedCmp(struct mixed_num_t x, struct mixed_num_t y){
	if (x.base < y.base)
		return -1;
	if (x.base > y.base)
		return 1;
	if (x.numer*y.denom < x.denom*y.numer)
		return -1;	
	if (x.numer*y.denom > x.denom*y.numer)
		return 1;
	return 0;
}

struct mixed_num_t makeMix(int64_t base,int64_t numer,int64_t denom);

#endif
