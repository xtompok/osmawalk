#include <yaml.h>

#include "config.h"

#define GTFS_ROUTE_TYPES 8

// Loading 
struct config_t parseConfigFile(char * filename){
	struct config_t conf;
	conf.desc = objtype__descriptor;
	conf.maxvalue = conf.desc.values[conf.desc.n_values-1].value;
	conf.speeds = malloc(sizeof(double)*(conf.maxvalue+1));
	conf.ratios = malloc(sizeof(double)*(conf.maxvalue+1));
	conf.penalties = malloc(sizeof(double)*(conf.maxvalue+1));
	for (int i=0;i<conf.maxvalue+1;i++){
		conf.speeds[i]=-1;
		conf.ratios[i]=-1;
		conf.penalties[i]=1;
	}
	conf.pt_fixed_penalties = calloc(sizeof(double),GTFS_ROUTE_TYPES);
	conf.pt_time_penalties = calloc(sizeof(double),GTFS_ROUTE_TYPES);

	conf.pt_max_vehicles = 100;
	conf.pt_geton_penalty = 0;
	conf.pt_min_wait = 0;
	

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

	yaml_node_t * node;
	yaml_node_pair_t * section;

	node = yaml_document_get_root_node(&document);

	if (node->type == YAML_MAPPING_NODE){
		for (section=node->data.mapping.pairs.start;
				section < node->data.mapping.pairs.top; section++){

			yaml_node_t * key;
			key = yaml_document_get_node(&document,section->key);
			printf("Key: %s\n",(char *)key->data.scalar.value);
			if (strcmp((char *)key->data.scalar.value,"speeds")==0){
				//printf("Parsing speeds\n");
				yaml_node_t * speedsMap;
				speedsMap = yaml_document_get_node(&document,section->value);
				if (speedsMap->type != YAML_MAPPING_NODE){
					printf("Speeds are not mapping\n");
					break;
				}
				yaml_node_pair_t * pair;
				for (pair=speedsMap->data.mapping.pairs.start;
						pair < speedsMap->data.mapping.pairs.top;pair++){
					yaml_node_t * key;
					yaml_node_t * value;
					key = yaml_document_get_node(&document,pair->key);
					value = yaml_document_get_node(&document,pair->value);
					for (int i=0;i<conf.desc.n_values;i++){
						if (strcmp((char *)key->data.scalar.value,conf.desc.values[i].name)==0){
							conf.speeds[conf.desc.values[i].value]=atof((char *)value->data.scalar.value);
							break;
						}
					}
				}
				
			} else if (strcmp((char *)key->data.scalar.value,"ratios")==0){
				//printf("Parsing ratios\n");
				yaml_node_t * ratiosMap;
				ratiosMap = yaml_document_get_node(&document,section->value);
				if (ratiosMap->type != YAML_MAPPING_NODE){
					printf("Ratios are not mapping\n");
					break;
				}
				yaml_node_pair_t * pair;
				for (pair=ratiosMap->data.mapping.pairs.start;
						pair < ratiosMap->data.mapping.pairs.top;pair++){
					yaml_node_t * key;
					yaml_node_t * value;
					key = yaml_document_get_node(&document,pair->key);
					value = yaml_document_get_node(&document,pair->value);
					for (int i=0;i<conf.desc.n_values;i++){
						if (strcmp((char *)key->data.scalar.value,conf.desc.values[i].name)==0){
							conf.ratios[conf.desc.values[i].value]=atof((char *)value->data.scalar.value);
							break;
						}
					}
				}
			
			} else if (strcmp((char *)key->data.scalar.value,"penalties")==0){
				//printf("Parsing speeds\n");
				yaml_node_t * penaltyMap;
				penaltyMap = yaml_document_get_node(&document,section->value);
				if (penaltyMap->type != YAML_MAPPING_NODE){
					printf("Penalties are not mapping\n");
					break;
				}
				yaml_node_pair_t * pair;
				for (pair=penaltyMap->data.mapping.pairs.start;
						pair < penaltyMap->data.mapping.pairs.top;pair++){
					yaml_node_t * key;
					yaml_node_t * value;
					key = yaml_document_get_node(&document,pair->key);
					value = yaml_document_get_node(&document,pair->value);
					for (int i=0;i<conf.desc.n_values;i++){
						if (strcmp((char *)key->data.scalar.value,conf.desc.values[i].name)==0){
							conf.penalties[conf.desc.values[i].value]=atof((char *)value->data.scalar.value);
							break;
						}
					}
				}
				
			} else if (strcmp((char *)key->data.scalar.value,"heights")==0){
				//printf("Parsing heights\n");
				yaml_node_t * heightsMap;
				heightsMap = yaml_document_get_node(&document,section->value);
				if (heightsMap->type != YAML_MAPPING_NODE){
					printf("Height are not mapping\n");
					break;
				}
				yaml_node_pair_t * pair;
				for (pair=heightsMap->data.mapping.pairs.start;
						pair < heightsMap->data.mapping.pairs.top;pair++){
					yaml_node_t * key;
					yaml_node_t * value;
					key = yaml_document_get_node(&document,pair->key);
					value = yaml_document_get_node(&document,pair->value);
					if (strcmp((char *)key->data.scalar.value,"upscale")==0){
						conf.upscale = atof((char *)value->data.scalar.value);	
					} else if (strcmp((char *)key->data.scalar.value,"downscale")==0){
						conf.downscale = atof((char *)value->data.scalar.value);	
					} else if (strcmp((char *)key->data.scalar.value,"maxslope")==0){
//						conf.maxslope = atof((char *)value->data.scalar.value);
					} else if (strcmp((char *)key->data.scalar.value,"upslopescale")==0){
//  					conf.upslopescale= atof((char *)value->data.scalar.value);
					} else if (strcmp((char *)key->data.scalar.value,"downslopescale")==0){
//						conf.downslopescale = atof((char *)value->data.scalar.value);
					} 
				}
				
			} else if (strcmp((char *)key->data.scalar.value,"pt-time-penalties")==0){
				printf("Parsing public transport time penalties\n");
				yaml_node_t * map;
				map = yaml_document_get_node(&document,section->value);
				if (map->type != YAML_MAPPING_NODE){
					printf("PT time penalties are not mapping\n");
					break;
				}
				yaml_node_pair_t * pair;
				for (pair=map->data.mapping.pairs.start;
						pair < map->data.mapping.pairs.top;pair++){
					yaml_node_t * key;
					yaml_node_t * value;
					key = yaml_document_get_node(&document,pair->key);
					value = yaml_document_get_node(&document,pair->value);
					if (strcmp((char *)key->data.scalar.value,"tram")==0){
						conf.pt_time_penalties[0] = atof((char *)value->data.scalar.value);	
					} else if (strcmp((char *)key->data.scalar.value,"metro")==0){
						conf.pt_time_penalties[1] = atof((char *)value->data.scalar.value);	
					} else if (strcmp((char *)key->data.scalar.value,"rail")==0){
						conf.pt_time_penalties[2] = atof((char *)value->data.scalar.value);	
					} else if (strcmp((char *)key->data.scalar.value,"bus")==0){
						conf.pt_time_penalties[3] = atof((char *)value->data.scalar.value);	
					} else if (strcmp((char *)key->data.scalar.value,"ferry")==0){
						conf.pt_time_penalties[4] = atof((char *)value->data.scalar.value);	
					} else if (strcmp((char *)key->data.scalar.value,"cable-car")==0){
						conf.pt_time_penalties[5] = atof((char *)value->data.scalar.value);	
					} else if (strcmp((char *)key->data.scalar.value,"gondola")==0){
						conf.pt_time_penalties[6] = atof((char *)value->data.scalar.value);	
					} else if (strcmp((char *)key->data.scalar.value,"funicular")==0){
						conf.pt_time_penalties[7] = atof((char *)value->data.scalar.value);	
					}
				}
			} else if (strcmp((char *)key->data.scalar.value,"pt-fixed-penalties")==0){
				printf("Parsing public transport fixed penalties\n");
				yaml_node_t * map;
				map = yaml_document_get_node(&document,section->value);
				if (map->type != YAML_MAPPING_NODE){
					printf("PT fixed penalties are not mapping\n");
					break;
				}
				yaml_node_pair_t * pair;
				for (pair=map->data.mapping.pairs.start;
						pair < map->data.mapping.pairs.top;pair++){
					yaml_node_t * key;
					yaml_node_t * value;
					key = yaml_document_get_node(&document,pair->key);
					value = yaml_document_get_node(&document,pair->value);
					if (strcmp((char *)key->data.scalar.value,"tram")==0){
						conf.pt_fixed_penalties[0] = atof((char *)value->data.scalar.value);	
					} else if (strcmp((char *)key->data.scalar.value,"metro")==0){
						conf.pt_fixed_penalties[1] = atof((char *)value->data.scalar.value);	
					} else if (strcmp((char *)key->data.scalar.value,"rail")==0){
						conf.pt_fixed_penalties[2] = atof((char *)value->data.scalar.value);	
					} else if (strcmp((char *)key->data.scalar.value,"bus")==0){
						conf.pt_fixed_penalties[3] = atof((char *)value->data.scalar.value);	
					} else if (strcmp((char *)key->data.scalar.value,"ferry")==0){
						conf.pt_fixed_penalties[4] = atof((char *)value->data.scalar.value);	
					} else if (strcmp((char *)key->data.scalar.value,"cable-car")==0){
						conf.pt_fixed_penalties[5] = atof((char *)value->data.scalar.value);	
					} else if (strcmp((char *)key->data.scalar.value,"gondola")==0){
						conf.pt_fixed_penalties[6] = atof((char *)value->data.scalar.value);	
					} else if (strcmp((char *)key->data.scalar.value,"funicular")==0){
						conf.pt_fixed_penalties[7] = atof((char *)value->data.scalar.value);	
					}
				}
			} else if (strcmp((char *)key->data.scalar.value,"line-penalties")==0){
				printf("Parsing public transport line penalties\n");
				yaml_node_t * map;
				map = yaml_document_get_node(&document,section->value);
				if (map->type != YAML_MAPPING_NODE){
					printf("PT line penalties are not mapping\n");
					break;
				}
				conf.pt_n_line_penalties = map->data.mapping.pairs.top-map->data.mapping.pairs.start;
				conf.pt_line_penalties = calloc(sizeof(struct line_config_t),conf.pt_n_line_penalties);
				yaml_node_pair_t * pair;
				int i=0;
				for (pair=map->data.mapping.pairs.start;
						pair < map->data.mapping.pairs.top;pair++){
					yaml_node_t * key;
					yaml_node_t * value;
					key = yaml_document_get_node(&document,pair->key);
					value = yaml_document_get_node(&document,pair->value);
					conf.pt_line_penalties[i].name = malloc(strlen((char *)key->data.scalar.value)+1);
					strcpy(conf.pt_line_penalties[i].name,(char *)key->data.scalar.value);
					if (strcmp((char *)key->data.scalar.value,"inf")==0){
						conf.pt_line_penalties[i].penalty = PENALTY_INFINITY;
					} else {
						conf.pt_line_penalties[i].penalty = atof((char *)value->data.scalar.value);
					}	
					i++;
				}
			} else if (strcmp((char *)key->data.scalar.value,"max-vehicles")==0){
				yaml_node_t * value;
				value = yaml_document_get_node(&document,section->value);
				conf.pt_max_vehicles = atol((char *)value->data.scalar.value);
			} else if (strcmp((char *)key->data.scalar.value,"geton-penalty")==0){
				yaml_node_t * value;
				value = yaml_document_get_node(&document,section->value);
				conf.pt_geton_penalty = atol((char *)value->data.scalar.value);
			} else if (strcmp((char *)key->data.scalar.value,"min-wait")==0){
				yaml_node_t * value;
				value = yaml_document_get_node(&document,section->value);
				conf.pt_min_wait = atol((char *)value->data.scalar.value);
			} else
			{
				printf("Unsupported section: %s\n",key->data.scalar.value);
			}
		}		
	}
	else {
		printf("Unsupported format of configuration\n");
	}
	
	yaml_document_delete(&document);		
	yaml_parser_delete(&parser);

	int wayIdx;
	wayIdx = -1;
	for (int i=0;i<conf.desc.n_values;i++){
		if (strcmp(conf.desc.values[i].name,"WAY")==0){
			wayIdx = i;
			if (conf.speeds[wayIdx]==-1){
				printf("WAY does not have speed\n");
			}
			break;
		}
	}
	for (int i=0;i<(conf.maxvalue+1);i++){
		if (conf.speeds[i]==-1){
			if (conf.ratios[i]==-1)
				conf.speeds[i]=conf.speeds[wayIdx];
			else
				conf.speeds[i]=conf.speeds[wayIdx]*conf.ratios[i];
		}
		conf.speeds[i]/=3.6;
	}
	return conf;
}
