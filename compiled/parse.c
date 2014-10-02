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
#include "parse.h"
#include "yamlconf.h"

// TODO
// - tunely, mosty, plochy
// - vysky
// - zahazovani nepotrebneho

uint64_t wanted_id = 0;
int debug = 0;
int type;
char *user = NULL, *tag = NULL, *value = NULL;
char *file;
int use_rel = 0, use_way = 0, use_node = 0;
int file_type = OSM_FTYPE_UNKNOWN;
int write_gpx = 0;
OSM_BBox *bbox = NULL;

struct mapconfig_t conf;
FILE * wayFile;
FILE * nodeFile;
FILE * mpFile;

struct height_map_t heights;

struct height_map_t loadHeights(char * filename){
	long len;
	struct height_map_t map;
	FILE * IN;
	IN = fopen(filename,"r");
	if (IN == NULL){
		printf("File %s not found", filename);
		return map;
	}
	fseek(IN,0,SEEK_END);
	len = ftell(IN);
	rewind(IN);
	fread(&map.minlon,1,sizeof(int),IN);
	fread(&map.minlat,1,sizeof(int),IN);
	fread(&map.maxlon,1,sizeof(int),IN);
	fread(&map.maxlat,1,sizeof(int),IN);
	
	printf("Heights from N %d E %d to N %d E %d\n",map.minlat,map.minlon,map.maxlat,map.maxlon);

	map.map = malloc(sizeof(int)*(len/2));
	for (int i=0;i<len/2;i++){
		map.map[i]=0;
		fread(map.map+i,1,2,IN);
	}
	return map;
}

int calcHeight(struct height_map_t map, double lat, double lon){
	if ((lat < map.minlat) || (lat >= map.maxlat))
		return 0;
	if ((lon < map.minlon) || (lon >= map.maxlon))
		return 0;
	lat -= map.minlat;
	lon -= map.minlon;
	lat *=1200;
	lon *=1200;
	int rowlen;
	int coord;
	rowlen = (map.maxlon-map.minlon)*1200;
	coord = ((int)lat)*rowlen+((int)lon);
	return map.map[coord];
}


int classify(OSM_Tag_List * tags,struct tag_t * classifier){
	int priority = -1;
	int objtype = 0;
	for (int i=0;i<tags->num;i++){
		OSM_Tag tag;
		tag = tags->data[i];
		for (int k=0;k<GARY_SIZE(classifier);k++){
			if (strcmp(classifier[k].key,tag.key))
				continue;
			struct tag_t tag;
			tag = classifier[k];
			for (int v=0;v<GARY_SIZE(tag.values);v++){
				if ((tag.values[v].name[0]=='*') && 
				    (tag.values[v].prio>priority)){
					objtype = tag.values[v].objtype;
					priority = tag.values[v].prio;
				}
				else if (strcmp(tag.values[v].name,tag.key))
					continue;
				if (tag.values[v].prio>priority){
					objtype = tag.values[v].objtype;
					priority = tag.values[v].prio;
				}
				
			}
		}	
	}
	return objtype;
}

void dumpMultipol(OSM_Relation *mp, struct obj_attr attr){
	Premap__Multipolygon * pbMp;
	pbMp = malloc(sizeof(Premap__Multipolygon));
	premap__multipolygon__init(pbMp);
	pbMp->id = mp->id;
	pbMp->has_type = true;
	pbMp->type = attr.objtype;
//	pbMp->has_tunnel = true;
//	pbMp->tunnel = attr.tunnel;
//	pbMp->has_bridge = true;
//	pbMp->bridge = attr.bridge;

	// Write refs etc.
	pbMp->n_refs = mp->member->num;
	pbMp->n_roles = mp->member->num;
	pbMp->refs = malloc(sizeof(pbMp->refs[0])*pbMp->n_refs);
	pbMp->roles = malloc(sizeof(pbMp->roles[0])*pbMp->n_refs);
	for (int i=0;i<pbMp->n_refs;i++){
		OSM_Rel_Member memb;
		memb = mp->member->data[i];
		if (memb.type != OSM_REL_MEMBER_TYPE_WAY){
			fprintf(stderr,"Non-way member in multipolygon %d\n",mp->id);
			free(pbMp->roles);
			free(pbMp->refs);
			free(pbMp);
			return;
		}
		if (strcmp(memb.role,"inner")==0){
			pbMp->roles[i]=PREMAP__MULTIPOLYGON__ROLE__INNER;
		}
		else if (strcmp(memb.role,"outer")==0){
			pbMp->roles[i]=PREMAP__MULTIPOLYGON__ROLE__OUTER;
		} else{
			fprintf(stderr,"Role non inner nor outer in multipolygon %d\n",mp->id);
			free(pbMp);
			free(pbMp->roles);
			free(pbMp->refs);
			return;
		}
		pbMp->refs[i] = memb.ref;
	}
	
	size_t len;
	len = premap__multipolygon__get_packed_size(pbMp);
	uint8_t * buf;
	buf = malloc(len);
	premap__multipolygon__pack(pbMp,buf);
	fwrite(&len,1,sizeof(size_t),mpFile);
	fwrite(buf,1,len,mpFile);
	free(buf);
	free(pbMp->roles);
	free(pbMp->refs);
	free(pbMp);

}
void dumpWay(OSM_Way * way, struct obj_attr attr){
	Premap__Way * pbWay;
	pbWay = malloc(sizeof(Premap__Way));
	premap__way__init(pbWay);
	pbWay->id = way->id;
	pbWay->has_type = true;
	pbWay->type = attr.objtype;
	pbWay->has_tunnel = true;
	pbWay->tunnel = attr.tunnel;
	pbWay->has_bridge = true;
	pbWay->bridge = attr.bridge;
	pbWay->has_area = true;
	pbWay->area = attr.area;
	int n_refs;
	n_refs = 0;
	while (way->nodes[n_refs]!=0)
		n_refs++;
	pbWay->n_refs=n_refs;
	pbWay->refs = malloc(sizeof(pbWay->refs[0])*n_refs);
	for (int i=0;i<n_refs;i++){
		pbWay->refs[i] = way->nodes[i];
	}
	size_t len;
	len = premap__way__get_packed_size(pbWay);
	uint8_t * buf = malloc(len);
	premap__way__pack(pbWay,buf);
	fwrite(&len,1,sizeof(size_t),wayFile);
	fwrite(buf,1,len,wayFile);
	free(buf);
	free(pbWay->refs);
	free(pbWay);

}

void dumpNode(OSM_Node * node, struct obj_attr attr){
	Premap__Node * pbNode;
	pbNode = malloc(sizeof(Premap__Node));
	premap__node__init(pbNode);
	pbNode->id = node->id;
	pbNode->has_objtype = true;
	pbNode->objtype = attr.objtype;
	pbNode->has_intunnel = true;
	pbNode->intunnel = attr.tunnel;
	pbNode->has_onbridge = true;
	pbNode->onbridge = attr.bridge;
	pbNode->has_height = true;
	pbNode->height = calcHeight(heights,node->lat,node->lon);
	pbNode->lat = node->lat;
	pbNode->lon = node->lon;
	size_t len;
	len = premap__node__get_packed_size(pbNode);
	uint8_t * buf;
	buf = malloc(len);
	premap__node__pack(pbNode,buf);
	fwrite(&len,1,sizeof(size_t),nodeFile);
	fwrite(buf,1,len,nodeFile);
	free(buf);
	free(pbNode);
}


int relation(OSM_Relation *r) {
    	printf("Relation: %d\n",r->id);
	OSM_Tag * tags;
	tags = r->tags->data;
	for (int i=0;i<r->tags->num;i++){
		if ((strcmp(tags[i].key,"type")==0) &&
		    (strcmp(tags[i].val,"multipolygon")!=0))
			return 0;
		printf("	%s: %s\n",r->tags->data[i].key,r->tags->data[i].val);
    	}
	struct obj_attr attr;
	attr.objtype = classify(r->tags,conf.type);
	attr.tunnel  = classify(r->tags,conf.tunnel);
	attr.bridge = classify(r->tags,conf.bridge);
	dumpMultipol(r,attr);
    	return 0;
}

int way(OSM_Way *w) {
	struct obj_attr attr;
	attr.objtype = classify(w->tags,conf.type);
	attr.area = classify(w->tags,conf.area);
	attr.tunnel  = classify(w->tags,conf.tunnel);
	attr.bridge = classify(w->tags,conf.bridge);
//	if (objtype==-1)
//		return;
	dumpWay(w,attr);
	return 0;
}

int node(OSM_Node *n) {
	struct obj_attr attr;
	attr.objtype = classify(n->tags,conf.type);
	attr.tunnel  = classify(n->tags,conf.tunnel);
	attr.bridge = classify(n->tags,conf.bridge);
//	if (objtype==-1)
//		return;
	dumpNode(n,attr);
	return 0;
}

int main(int argc, char **argv) {
	int i;
	OSM_File *F;
	OSM_Data *O;
	
	file_type = OSM_FTYPE_XML;

	conf.desc = objtype__descriptor;
	GARY_INIT(conf.type,0);
	GARY_INIT(conf.area,0);
	GARY_INIT(conf.bridge,0);
	GARY_INIT(conf.tunnel,0);

	parseMapConfigFile("../config/types.yaml",&conf,addTypeItemToMapConf);
	parseMapConfigFile("../config/area.yaml",&conf,addAreaItemToMapConf);
	parseMapConfigFile("../config/bridge.yaml",&conf,addBridgeItemToMapConf);
	parseMapConfigFile("../config/tunnel.yaml",&conf,addTunnelItemToMapConf);

	heights = loadHeights("../osm/heights.bin");



	wayFile = fopen("../data/ways-stage1","w");
	nodeFile = fopen("../data/nodes-stage1","w");
	mpFile = fopen("../data/mp-stage1","w");

	osm_init();

	F = osm_open(argv[1], file_type);
	if (F == NULL)
		return 1;

	O = osm_parse(F, OSMDATA_REL, NULL, node, way, relation);    
	osm_close(F);
	
	fclose(wayFile);
	fclose(nodeFile);
	fclose(mpFile);
	
	return 0;
}
