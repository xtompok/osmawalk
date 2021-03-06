#include "utils.h"

#include <stdlib.h>
#include <stdio.h>

unsigned int isDirectableWay(Premap__Way * way){
	unsigned int t;
	t = way->type;
	if ((t==OBJTYPE__WAY || 
		t==OBJTYPE__PARK ||
		t==OBJTYPE__GREEN ||
		t==OBJTYPE__FOREST ||
		t==OBJTYPE__PAVED ||
		t==OBJTYPE__UNPAVED ||
		t==OBJTYPE__STEPS ||
		t==OBJTYPE__HIGHWAY
		)&& (
		way->tunnel==0 &&
		way->bridge==0))
	return 1;
	
	return 0;
}

unsigned int isDirectableNode(Premap__Node * node){
	if (node->intunnel==0 &&
		node->onbridge==0)
	return 1;
	
	return 0;
}
unsigned int isBarrier(Premap__Way * way){
	unsigned int t;
	t = way->type;
	if (t==OBJTYPE__WAY || 
		t==OBJTYPE__BARRIER ||
	//	t==OBJTYPE__ROPE ||
		t==OBJTYPE__SOLID 
		)
	return 1;
	
	return 0;
}
unsigned int isWay(Premap__Way * way){
	unsigned int t;
	t = way->type;
	if (t==OBJTYPE__WAY || 
		t==OBJTYPE__PARK ||
		t==OBJTYPE__GREEN ||
		t==OBJTYPE__FOREST ||
		t==OBJTYPE__PAVED ||
		t==OBJTYPE__UNPAVED ||
		t==OBJTYPE__STEPS ||
		t==OBJTYPE__HIGHWAY
		)
	return 1;
	
	return 0;
}
int isWalkArea(Premap__Way * way){
	if (way->area && isWay(way))
		return 1;
	else 
		return 0;

}
