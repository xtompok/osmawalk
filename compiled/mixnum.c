#include "mixnum.h"
#include <stdlib.h>
#include <stdio.h>

void mixedPrint(struct mixed_num_t mix){
	printf("%lld + %lld/%lld ~ %lld",mix.base,mix.numer,mix.denom,mix.base+mix.numer/mix.denom);
}

struct mixed_num_t makeMix(int64_t base,int64_t numer,int64_t denom){
	if (denom<0){
		numer*=-1;
		denom*=-1;
	}
	struct mixed_num_t mix;
	mix.base = base + (numer/denom);
	mix.numer = numer%denom;
	mix.denom = denom;
	return mix;
}
