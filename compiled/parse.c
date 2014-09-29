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
#include "parse.h"

uint64_t wanted_id = 0;
int debug = 0;
int type;
char *user = NULL, *tag = NULL, *value = NULL;
char *file;
int use_rel = 0, use_way = 0, use_node = 0;
int file_type = OSM_FTYPE_UNKNOWN;
int write_gpx = 0;
OSM_BBox *bbox = NULL;


int enumValueForName(ProtobufCEnumDescriptor desc,char * name){
	for (int i=0;i<desc.n_value_names;i++){
		if (strcmp(name,desc.values_by_name[i].name)==0){
			return desc.values[desc.values_by_name[i].index].value;
		}	
	}
	return -1;
};

void addItemToMapConf(struct mapconfig_t *  conf,struct mapConfItem_t item){
	printf("%s: %s -> %d (priority %d)\n",item.key,item.value,item.enum_val,item.priority);
	for (int i=0;i<GARY_SIZE(conf->tags);i++){
		if (strcmp(conf->tags[i].key,item.key))
			continue;
		struct value_t * val;
		val = GARY_PUSH(conf->tags[i].values);
		val->prio = item.priority;
		val->objtype = item.enum_val;
		val->name = malloc((strlen(item.value)+1)*sizeof(char));
		strcpy(val->name,item.value);
		return;

	}
	printf("Creating new key: ");
	struct tag_t * tag;
	tag=GARY_PUSH(conf->tags);
	tag->key = malloc((strlen(item.key)+1)*sizeof(char));
	strcpy(tag->key,item.key);
	printf("%s\n",tag->key);
	GARY_INIT(tag->values,1);
	struct value_t val;
	val = tag->values[1];
	val.prio = item.priority;
	val.objtype = item.enum_val;
	val.name = malloc((strlen(item.value)+1)*sizeof(char));
	strcpy(val.name,item.value);
	return;
}

// Loading 
struct mapconfig_t parseMapConfigFile(char * filename){
	struct mapconfig_t conf;
	conf.desc = objtype__descriptor;
	GARY_INIT(conf.tags,0);

	yaml_parser_t parser;
	yaml_document_t document;

	yaml_parser_initialize(&parser);

	FILE * IN;
	IN = fopen(filename,"r");
	if (IN==NULL){
		printf("Config file opening error\n");	
		return conf;
	}
	yaml_parser_set_input_file(&parser,IN);

	yaml_parser_load(&parser,&document);
	fclose(IN);

	yaml_node_t * root;
	yaml_node_t * anode; // use as temporary storage
	struct mapConfItem_t mapConfItem;

	root = yaml_document_get_root_node(&document);

	if (root->type != YAML_MAPPING_NODE){
		printf("Wrong syntax of configuration file in objtypes\n");
		return conf;
	}
	for (yaml_node_pair_t * type=root->data.mapping.pairs.start;
			type < root->data.mapping.pairs.top; type++){
		anode = yaml_document_get_node(&document,type->key);
		mapConfItem.enum_val = enumValueForName(conf.desc,anode->data.scalar.value);
		printf("%s (%d)\n",anode->data.scalar.value,mapConfItem.enum_val);

		yaml_node_t * priomap;
		priomap = yaml_document_get_node(&document,type->value);
		if (priomap->type != YAML_MAPPING_NODE){
			printf("Wrong syntax of configuration file in priorities\n");
			return conf;
		}
		for (yaml_node_pair_t * prio=priomap->data.mapping.pairs.start;
				prio < priomap->data.mapping.pairs.top; prio++){
			anode = yaml_document_get_node(&document,prio->key);
			mapConfItem.priority=atoi(anode->data.scalar.value);
			printf("	%d\n",mapConfItem.priority);
			yaml_node_t * tagmap;
			tagmap = yaml_document_get_node(&document,prio->value);
			if (tagmap->type != YAML_MAPPING_NODE){
				printf("Wrong syntax of configuration file in tags\n");
				return conf;
			}
			for (yaml_node_pair_t * atag=tagmap->data.mapping.pairs.start;
					atag < tagmap->data.mapping.pairs.top; atag++){
				anode = yaml_document_get_node(&document,atag->key);
				mapConfItem.key = anode->data.scalar.value;
				printf("		%s\n",mapConfItem.key);
				yaml_node_t * vallist;
				vallist = yaml_document_get_node(&document,atag->value);
				if (vallist->type == YAML_SEQUENCE_NODE){
					for (yaml_node_item_t *  val=vallist->data.sequence.items.start;
							val < vallist->data.sequence.items.top; val++){
						anode = yaml_document_get_node(&document,*val);
						mapConfItem.value=anode->data.scalar.value;
						printf("			%s\n",mapConfItem.value);
						addItemToMapConf(&conf,mapConfItem);
					}
				} else if (vallist->type == YAML_SCALAR_NODE){ 
					mapConfItem.value=vallist->data.scalar.value;
					printf("			%s\n",mapConfItem.value);
					
					addItemToMapConf(&conf,mapConfItem);
				}else {
					printf("Wrong syntax of configuration file in values\n");
					return conf;
				}
			}
		}
	}
	
	yaml_document_delete(&document);		
	yaml_parser_delete(&parser);

	return conf;
}


int relation(OSM_Relation *r) {
    printf("Relation: %d\n",r->id);
    for (int i=0;i<r->tags->num;i++){
    	printf("	%s: %s\n",r->tags->data[i].key,r->tags->data[i].val);
    }
    return 0;
}

int way(OSM_Way *w) {
    printf("Way:%d\n",w->id);
    return 0;
}

int node(OSM_Node *n) {
    printf("Node:%d\n",n->id);
    return 0;
}


int tag_node(OSM_Node *n) {
    int i;
    if (n->tags == NULL)
        return 0;
    if (tag == NULL)
        return 1;
    if (value != NULL) {
        for (i=0; i<n->tags->num; i++) {
            char *key = n->tags->data[i].key;
            char *val = n->tags->data[i].val;
            if (*key && *val)
            {
                if (strcmp(tag, key) == 0 && strcmp(value, val) == 0)
                    return 1;
            }
        }
    }
    else {
        for (i=0; i<n->tags->num; i++) {
            if (*n->tags->data[i].key) {
                char *key = n->tags->data[i].key;
                if (strcmp(tag, key) == 0)
                    return 1;
            }
        }
    }
    return 0;
}

int tag_way(OSM_Way *n) {
    int i;
    if (n->tags == NULL)
        return 0;
    if (tag == NULL)
        return 1;
    if (value != NULL) {
        for (i=0; i<n->tags->num; i++) {
            char *key = n->tags->data[i].key;
            char *val = n->tags->data[i].val;
            if (*key && *val)
            {
                if (strcmp(tag, key) == 0 && strcmp(value, val) == 0)
                    return 1;
            }
        }
    }
    else {
        for (i=0; i<n->tags->num; i++) {
            if (*n->tags->data[i].key) {
                char *key = n->tags->data[i].key;
                if (strcmp(tag, key) == 0)
                    return 1;
            }
        }
    }
    return 0;
}

int tag_rel(OSM_Relation *n) {
    int i;
    if (n->tags == NULL)
        return 0;
    if (tag == NULL)
        return 1;
    if (value != NULL) {
        for (i=0; i<n->tags->num; i++) {
            char *key = n->tags->data[i].key;
            char *val = n->tags->data[i].val;
            if (*key && *val)
            {
                if (strcmp(tag, key) == 0 && strcmp(value, val) == 0)
                    return 1;
            }
        }
    }
    else {
        for (i=0; i<n->tags->num; i++) {
            if (*n->tags->data[i].key) {
                char *key = n->tags->data[i].key;
                if (strcmp(tag, key) == 0)
                    return 1;
            }
        }
    }
    return 0;
}


int main(int argc, char **argv) {
    int i;
    OSM_File *F;
    OSM_Data *O;

    file_type = OSM_FTYPE_XML;

    parseMapConfigFile(argv[2]);

    osm_init();

    F = osm_open(argv[1], file_type);
    if (F == NULL)
        return 1;
    
    O = osm_parse(F, OSMDATA_REL, NULL, node, way, relation);    
    osm_close(F);
    return 0;
}
