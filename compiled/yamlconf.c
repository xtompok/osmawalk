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

#include "yamlconf.h"


int enumValueForName(ProtobufCEnumDescriptor desc,char * name){
	for (int i=0;i<desc.n_value_names;i++){
		if (strcmp(name,desc.values_by_name[i].name)==0){
			return desc.values[desc.values_by_name[i].index].value;
		}	
	}
	return -1;
};



struct tag_t *  addItemToTagList(struct tag_t * list,struct mapConfItem_t item, int type){
	printf("%s: %s -> %d (priority %d)\n",item.key,item.value,type,item.priority);
	for (int i=0;i<GARY_SIZE(list);i++){
		if (strcmp(list[i].key,item.key))
			continue;
		struct value_t * val;
		val = GARY_PUSH(list[i].values);
		val->prio = item.priority;
		val->objtype = type;
		val->name = malloc((strlen(item.value)+1)*sizeof(char));
		strcpy(val->name,item.value);
		return list;

	}
	printf("Creating new key: ");
	struct tag_t * tag;
	tag=GARY_PUSH(list);
	tag->key = malloc((strlen(item.key)+1)*sizeof(char));
	strcpy(tag->key,item.key);
	printf("%s\n",tag->key);
	GARY_INIT(tag->values,1);
	struct value_t * val;
	val = &tag->values[0];
	val->prio = item.priority;
	val->objtype = type;
	val->name = malloc((strlen(item.value)+1)*sizeof(char));
	strcpy(val->name,item.value);
	return list;
}
void addAreaItemToMapConf(struct mapconfig_t *  conf,struct mapConfItem_t item){
	int type;
	if (strcmp(item.name,"yes")==0)
		type = 1;
	else if (strcmp(item.name,"no")==0)
		type = 0;
	else{
		printf("Wrong value: %s\n",item.name);
		return;
	}
	conf->area = addItemToTagList(conf->area,item,type);
	return;
}

void addTunnelItemToMapConf(struct mapconfig_t *  conf,struct mapConfItem_t item){
	int type;
	if (strcmp(item.name,"yes")==0)
		type = 1;
	else if (strcmp(item.name,"no")==0)
		type = 0;
	else{
		printf("Wrong value: %s\n",item.name);
		return;
	}
	conf->tunnel = addItemToTagList(conf->tunnel,item,type);
	return;
}
void addBridgeItemToMapConf(struct mapconfig_t *  conf,struct mapConfItem_t item){
	int type;
	if (strcmp(item.name,"yes")==0)
		type = 1;
	else if (strcmp(item.name,"no")==0)
		type = 0;
	else{
		printf("Wrong value: %s\n",item.name);
		return;
	}
	conf->bridge = addItemToTagList(conf->bridge,item,type);
	return;
}

void addTypeItemToMapConf(struct mapconfig_t *  conf,struct mapConfItem_t item){
	int enum_val;
	enum_val = enumValueForName(conf->desc, item.name);
	conf->type = addItemToTagList(conf->type,item,enum_val);
}

int parseMapConfigFile(char * filename, struct mapconfig_t * conf,
			void (*addItemToMapConf)(struct mapconfig_t * conf,struct mapConfItem_t item )){

	yaml_parser_t parser;
	yaml_document_t document;

	yaml_parser_initialize(&parser);

	FILE * IN;
	IN = fopen(filename,"r");
	if (IN==NULL){
		printf("Config file opening error\n");	
		return 1;
	}
	yaml_parser_set_input_file(&parser,IN);

	if (!yaml_parser_load(&parser,&document)){
		printf("Error while parsing config file");
		return 1;	
	}
	fclose(IN);

	yaml_node_t * root;
	yaml_node_t * anode; // use as temporary storage
	struct mapConfItem_t mapConfItem;

	root = yaml_document_get_root_node(&document);


	if (root->type != YAML_MAPPING_NODE){
		printf("Wrong syntax of configuration file in objtypes\n");
		return 2;
	}
	for (yaml_node_pair_t * type=root->data.mapping.pairs.start;
			type < root->data.mapping.pairs.top; type++){
		anode = yaml_document_get_node(&document,type->key);
		mapConfItem.name = (char *) anode->data.scalar.value;
		printf("%s\n",mapConfItem.name);

		yaml_node_t * priomap;
		priomap = yaml_document_get_node(&document,type->value);
		if (priomap->type != YAML_MAPPING_NODE){
			printf("Wrong syntax of configuration file in priorities\n");
			return 3;
		}
		for (yaml_node_pair_t * prio=priomap->data.mapping.pairs.start;
				prio < priomap->data.mapping.pairs.top; prio++){
			anode = yaml_document_get_node(&document,prio->key);
			mapConfItem.priority=atoi((char *)anode->data.scalar.value);
			printf("	%d\n",mapConfItem.priority);
			yaml_node_t * tagmap;
			tagmap = yaml_document_get_node(&document,prio->value);
			if (tagmap->type != YAML_MAPPING_NODE){
				printf("Wrong syntax of configuration file in tags\n");
				return 4;
			}
			for (yaml_node_pair_t * atag=tagmap->data.mapping.pairs.start;
					atag < tagmap->data.mapping.pairs.top; atag++){
				anode = yaml_document_get_node(&document,atag->key);
				mapConfItem.key =(char *) anode->data.scalar.value;
				printf("		%s\n",mapConfItem.key);
				yaml_node_t * vallist;
				vallist = yaml_document_get_node(&document,atag->value);
				if (vallist->type == YAML_SEQUENCE_NODE){
					for (yaml_node_item_t *  val=vallist->data.sequence.items.start;
							val < vallist->data.sequence.items.top; val++){
						anode = yaml_document_get_node(&document,*val);
						mapConfItem.value=(char *)anode->data.scalar.value;
						printf("			%s\n",mapConfItem.value);
						addItemToMapConf(conf,mapConfItem);
					}
				} else if (vallist->type == YAML_SCALAR_NODE){ 
					mapConfItem.value=(char *)vallist->data.scalar.value;
					printf("			%s\n",mapConfItem.value);
					
					addItemToMapConf(conf,mapConfItem);
				}else {
					printf("Wrong syntax of configuration file in values\n");
					return 5;
				}
			}
		}
	}
	
	yaml_document_delete(&document);		
	yaml_parser_delete(&parser);

	return 0;
}
