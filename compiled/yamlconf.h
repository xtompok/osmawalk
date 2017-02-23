#include <ucw/lib.h>
#include <ucw/fastbuf.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <yaml.h>
#include <ucw/gary.h>
#include <osm.h>

#include "include/types.pb-c.h"
#include "include/premap.pb-c.h"

struct value_t {
	char * name;
	int prio;
	int objtype;	
};

struct tag_t {
	char * key;
	struct value_t * values;
};	

struct mapconfig_t {
	ProtobufCEnumDescriptor desc;
	struct tag_t * type;
	struct tag_t * tunnel;
	struct tag_t * bridge;
	struct tag_t * area;
	struct tag_t * stop_pos;
};

struct mapConfItem_t {
	char * key;
	char * value;
	int priority;
	char * name;
};

int enumValueForName(ProtobufCEnumDescriptor desc,char * name);



struct tag_t *  addItemToTagList(struct tag_t * list,struct mapConfItem_t item, int type);

void addBoolItemToMapConf(struct mapconfig_t * conf, struct mapConfItem_t item,struct tag_t ** confitem); 
void addAreaItemToMapConf(struct mapconfig_t *  conf,struct mapConfItem_t item);
void addTunnelItemToMapConf(struct mapconfig_t *  conf,struct mapConfItem_t item);
void addBridgeItemToMapConf(struct mapconfig_t *  conf,struct mapConfItem_t item);
void addTypeItemToMapConf(struct mapconfig_t *  conf,struct mapConfItem_t item);
void addStopPosItemToMapConf(struct mapconfig_t *  conf,struct mapConfItem_t item);

int parseMapConfigFile(char * filename, struct mapconfig_t * conf,
			void (*addItemToMapConf)(struct mapconfig_t * conf,
			struct mapConfItem_t item ));
