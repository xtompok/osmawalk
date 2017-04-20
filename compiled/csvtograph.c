#include <ucw/lib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <csv.h>
#include <ucw/gary.h>
#include "include/graph.pb-c.h"
#include "hashes.h"
#include "searchgraph.h"

struct parse_t {
	int state;
	char ok;
	int type;
	char first;
	Graph__Vertex ** vertices;
	Graph__Edge ** edges;
	Graph__Stop ** stops;

	Graph__Vertex * tmpvertex;
	Graph__Edge * tmpedge;
	Graph__Stop * tmpstop;
};


void node_item_cb(void * item,size_t len,void * data){
	struct parse_t * p_struct;
	p_struct = (struct parse_t *)data;
	if (p_struct->first == 1)
		return;
	int64_t aInt;
	double aDouble;
	switch (p_struct->state) {
		case 0: //id
			aInt = strtoll((char *)item,NULL,10);
			p_struct->tmpvertex->osmid = aInt;
			break;
		case 1: //lat
			aDouble = strtod((char *)item,NULL);
			p_struct->tmpvertex->lat = aDouble;
			break;
		case 2: //lon
			aDouble = strtod((char *)item,NULL);
			p_struct->tmpvertex->lon = aDouble;
			break;
		case 3: //height
			aInt = strtoll((char *)item,NULL,10);
			p_struct->tmpvertex->height = aInt;
			break;

	}

	p_struct->state++;
}

void node_line_cb(int ch, void * data){
	struct parse_t * p_struct;
	p_struct = (struct parse_t *)data;
	p_struct->state=0;
	Graph__Vertex ** v;
	v = GARY_PUSH(p_struct->vertices);
	*v = malloc(sizeof(Graph__Vertex));
	graph__vertex__init(*v);
	p_struct->tmpvertex = *v;
	p_struct->first = 0;
}

void stop_item_cb(void * item,size_t len,void * data){
	struct parse_t * p_struct;
	p_struct = (struct parse_t *)data;
	if (p_struct->first == 1)
		return;
	int64_t aInt;
	double aDouble;
	switch (p_struct->state) {
		case 0: //id
			aInt = strtoll((char *)item,NULL,10);
			p_struct->tmpvertex->osmid = aInt;
			p_struct->tmpstop->osmid = aInt;
			break;
		case 1: //stop_id
			p_struct->tmpstop->stop_id = malloc(strlen(item)+1);
			strcpy(p_struct->tmpstop->stop_id,item);
			break;
		case 2: //raptor_id
			aInt = strtoll((char *)item,NULL,10);
			p_struct->tmpstop->raptor_id = aInt;
		case 3: //lat
			aDouble = strtod((char *)item,NULL);
			p_struct->tmpvertex->lat = aDouble;
			break;
		case 4: //lon
			aDouble = strtod((char *)item,NULL);
			p_struct->tmpvertex->lon = aDouble;
			break;
		case 5: //height
			p_struct->tmpvertex->height = -1;
			break;
	}

	p_struct->state++;
}

void stop_line_cb(int ch, void * data){
	struct parse_t * p_struct;
	p_struct = (struct parse_t *)data;
	p_struct->state=0;

	Graph__Vertex ** v;
	v = GARY_PUSH(p_struct->vertices);
	*v = malloc(sizeof(Graph__Vertex));
	graph__vertex__init(*v);
	p_struct->tmpvertex = *v;

	Graph__Stop ** s;
	s = GARY_PUSH(p_struct->stops);
	*s = malloc(sizeof(Graph__Stop));
	graph__stop__init(*s);
	p_struct->tmpstop = *s;

	p_struct->first = 0;
}

void way_item_cb(void * item,size_t len,void * data){
	struct parse_t * p_struct;
	p_struct = (struct parse_t *)data;
	if (p_struct->first == 1)
		return;
	int64_t num;
	num = strtoll((char *)item,NULL,10);
	struct nodesIdxNode * n;
	switch (p_struct->state) {
		case 0: //id
			p_struct->tmpedge->has_osmid = 1;
			p_struct->tmpedge->osmid = num;
			break;
		case 1: //type
			p_struct->tmpedge->type = num;
			break;
		case 2: //from
			n = nodesIdx_find2(num);
			if (!n){
				p_struct->ok=0;
				printf("Wrong from id: %lld\n",num);
				break;
			}
			p_struct->tmpedge->vfrom = n->idx;
			break;
		case 3: //to
			n = nodesIdx_find2(num);
			if (!n){
				p_struct->ok=0;
				printf("Wrong to id: %lld\n",num);
				break;
			}
			p_struct->tmpedge->vto = n->idx;
			break;

	}

	p_struct->state++;
}

void way_line_cb(int ch, void * data){
	struct parse_t * p_struct;
	p_struct = (struct parse_t *)data;
	p_struct->state=0;
	if (p_struct->ok==0){
		p_struct->ok=1;
		return;
	}
	Graph__Edge ** e;
	e = GARY_PUSH(p_struct->edges);
	*e = malloc(sizeof(Graph__Edge));
	graph__edge__init(*e);
	p_struct->tmpedge = *e;
	p_struct->first = 0;
}

void direct_item_cb(void * item,size_t len,void * data){
	struct parse_t * p_struct;
	p_struct = (struct parse_t *)data;
	if (p_struct->first == 1)
		return;
	int64_t num;
	num = strtoll((char *)item,NULL,10);
	switch (p_struct->state) {
		case 0: //from
			if (nodesIdx_find2(num) == NULL){
				printf("Unknown direct node %llu\n",num);
				p_struct->ok=0;
				return;
			}
			p_struct->tmpedge->vfrom = nodesIdx_find2(num)->idx;
			break;
		case 1: //to
			if (nodesIdx_find2(num) == NULL){
				printf("Unknown direct node %llu\n",num);
				p_struct->ok=0;
				return;
			}
			p_struct->tmpedge->vto = nodesIdx_find2(num)->idx;
			break;

	}

	p_struct->state++;
}

void direct_line_cb(int ch, void * data){
	struct parse_t * p_struct;
	p_struct = (struct parse_t *)data;
	p_struct->state=0;
	if (p_struct->ok==0){
		p_struct->ok=1;
		return;
	}
	Graph__Edge ** e;
	e = GARY_PUSH(p_struct->edges);
	*e = malloc(sizeof(Graph__Edge));
	graph__edge__init(*e);
	p_struct->tmpedge = *e;
	p_struct->tmpedge->type = p_struct->type;
	p_struct->first = 0;

}

int parseFile (char * baseDir, char * filename, struct csv_parser * parser,
		struct parse_t * p_struct,
		void (item_cb)(void * item, size_t len, void * data),
		void (line_cb)(int ch,void * data)){
	
	char * path;
	path = malloc(strlen(baseDir)+25);
	sprintf(path,"%s/%s",baseDir,filename);
	int fdin;
	fdin = open(path,O_RDONLY);
	if (fdin < 0){
		printf("Can't open file %s, exitting\n",path);
		return 1;
	}
	free(path);

	struct stat statbuf;
	fstat (fdin,&statbuf);

	char * inbuf;
	inbuf = mmap(NULL,statbuf.st_size,PROT_READ,MAP_PRIVATE,fdin,0);

	p_struct->first = 1;
	csv_parse(parser,inbuf,statbuf.st_size,item_cb,line_cb,p_struct);
	
	munmap(inbuf,statbuf.st_size);
	
	return 0;
}

int main (int argc, char ** argv){
	if (argc != 2){
		printf("Usage: %s <dir with csv files>\n",argv[0]);
		return 1;
	}
	char * basedir;
	if (argv[1][strlen(argv[1])-1]=='/'){
		argv[1][strlen(argv[1])-1]='\0';
	}
	basedir = argv[1];
	
	struct csv_parser parser;
	csv_init(&parser,CSV_APPEND_NULL);
	csv_set_delim(&parser,';');

	struct parse_t * p_struct;
	p_struct = malloc(sizeof(struct parse_t));
	p_struct->state=0;
	p_struct->ok=1;

	GARY_INIT(p_struct->vertices,0);
	GARY_INIT(p_struct->edges,0);
	GARY_INIT(p_struct->stops,0);

	parseFile(basedir,"nodes.csv",&parser,p_struct,node_item_cb,node_line_cb);
	printf("Nodes: %d\n",GARY_SIZE(p_struct->vertices));

	parseFile(basedir,"stops.csv",&parser,p_struct,stop_item_cb,stop_line_cb);
	printf("Nodes: %d\n",GARY_SIZE(p_struct->vertices));

	nodesIdx_refresh(GARY_SIZE(p_struct->vertices),p_struct->vertices);
	
	parseFile(basedir,"ways.csv",&parser,p_struct,way_item_cb,way_line_cb);

	p_struct->type=50;
	parseFile(basedir,"direct.csv",&parser,p_struct,direct_item_cb,direct_line_cb);

	p_struct->type=51;
	parseFile(basedir,"stops-direct.csv",&parser,p_struct,direct_item_cb,direct_line_cb);

	Graph__Graph * graph;
	graph = malloc(sizeof(Graph__Graph));
	graph__graph__init(graph);
	graph->n_edges=GARY_SIZE(p_struct->edges);
	graph->edges=p_struct->edges;
	graph->n_vertices=GARY_SIZE(p_struct->vertices);
	graph->vertices=p_struct->vertices;
	graph->n_stops=GARY_SIZE(p_struct->stops);
	graph->stops=p_struct->stops;

	printf("Created graph with %d edges, %d vertices and %d stops\n",
			graph->n_edges,
			graph->n_vertices,
			graph->n_stops);

	struct vertexedges_t * vedges;
	vedges = makeVertexEdges(graph);
	largestComponent(graph,vedges);
	printf("Created graph with %d edges, %d vertices and %d stops\n",
			graph->n_edges,
			graph->n_vertices,
			graph->n_stops);
	saveSearchGraph(graph,"../data/postgis-graph.pbf");


}
