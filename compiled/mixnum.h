#ifndef MIXNUM_H
#define MIXNUM_H

#include <ucw/lib.h>
#include <stdint.h>
#include "types.h"

/*!
 * @abstract Library for manipulating with mixed numbers
 *
 * @discussion This file provides several useful functions for manipulating 
 * with mixed numbers.
 *
 * */

/*!
 * @abstract Print mixed number to stdout without newline
 * @param mix Mixed number to print
 */
void mixedPrint(struct mixed_num_t mix);

/*!
 * @abstract Compare two mixed numbers
 *
 * @param x First mixed number
 * @param y Second mixed number
 * @return -1 if first is lower, 1 if second is lower, 0 if equal
 *
 */
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

/*!
 *  @abstract Make mixed number
 *
 *  @discussion Takes three numbers and returns mixed number base+(numer/denom).
 *
 *  @param base Base of the number
 *  @param numer Numerator of the rational part
 *  @param denom Denominator of the rational part
 *  @return Structure representing mixed number
 */
struct mixed_num_t makeMix(int64_t base,int64_t numer,int64_t denom);

#endif
