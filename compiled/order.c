#include <ucw/lib.h>
#include <ucw/fastbuf.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <ucw/gary.h>
#include <osm.h>

#include "include/types.pb-c.h"
#include "include/premap.pb-c.h"
#include "order.h"

#define TREE_PREFIX(X) tree_##X 
#define TREE_NODE struct clean_tree_node_t
#define TREE_KEY_ATOMIC id
#define TREE_WANT_CLEANUP
#define TREE_WANT_FIND
#define TREE_WANT_NEW
#define TREE_WANT_LOOKUP
#define TREE_WANT_ITERATOR
#include <ucw/redblack.h>

struct tree_tree * way_tree;
struct tree_tree * node_tree;

char *  node_cb(size_t len,char * inbuf, char * outbuf){
	Premap__Node * node;
	node = premap__node__unpack(NULL,len,inbuf);
	printf("Node: %lld\n",node->id);
	printf("Lat: %lld, lon: %lld\n",node->lat,node->lon);
	if (tree_find(node_tree,node->id)||(node->objtype!=OBJTYPE__NONE)){
		printf("Found\n");
		memcpy(outbuf,&len,sizeof(size_t));
		outbuf+=sizeof(size_t);
		memcpy(outbuf,inbuf,len);
		premap__node__free_unpacked(node,NULL);
		return outbuf+len;
	}
	premap__node__free_unpacked(node,NULL);
	return outbuf;
}

char * way_cb(size_t len,char * inbuf, char * outbuf){
	Premap__Way * way;
	way = premap__way__unpack(NULL,len,inbuf);
	printf("Way: %lld\n",way->id);
	if (tree_find(way_tree,way->id)||(way->type!=OBJTYPE__NONE))
	{
		printf("Found\n");
		for (int i=0;i<way->n_refs;i++){
			tree_lookup(node_tree,way->refs[i]);
		
		}
		memcpy(outbuf,&len,sizeof(size_t));
		outbuf+=sizeof(size_t);
		memcpy(outbuf,inbuf,len);
		premap__way__free_unpacked(way,NULL);
		return outbuf+len;
	}
	premap__way__free_unpacked(way,NULL);
	return outbuf;
}

char * mp_cb(size_t len,char * inbuf, char * outbuf){
	Premap__Multipolygon * mp;
	mp = premap__multipolygon__unpack(NULL,len,inbuf);
	printf("MP: %lld\n",mp->id);
	for (int i=0; i<mp->n_refs;i++){
		tree_lookup(way_tree,mp->refs[i]);
		premap__multipolygon__free_unpacked(mp,NULL);
		memcpy(outbuf,&len,sizeof(size_t));
		outbuf+=sizeof(size_t);
		memcpy(outbuf,inbuf,len);
		return outbuf+len;
	}
	premap__multipolygon__free_unpacked(mp,NULL);
	return outbuf;
}

int loadFile (char * inFilename, char * outFilename, 
		char * (callback)(size_t len, char * inbuf,char * outbuf)){
	int fdin;
	fdin = open(inFilename,O_RDONLY);
	if (fdin < 0){
		printf("Can't open file %s, exitting\n",inFilename);
		return 1;
	}


	struct stat statbuf;
	fstat (fdin,&statbuf);

	char * inbuf;
	inbuf = mmap(NULL,statbuf.st_size,PROT_READ,MAP_PRIVATE,fdin,0);

	char * outbuf;
	outbuf = malloc(statbuf.st_size);

	char * inbufp;
	char * outbufp;
	inbufp = inbuf;
	outbufp = outbuf;
	printf("File size: %d\n",statbuf.st_size);
	while (inbufp<(inbuf+statbuf.st_size)) {
		size_t len;
		memcpy(&len,inbufp,sizeof(size_t));
		inbufp += sizeof(size_t);
		outbufp = callback(len,inbufp,outbufp);
		inbufp+=len;
	}

	int fdout;
	fdout = open(outFilename,O_RDWR|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (fdout < 0){
		printf("Can't open file %s, exitting\n",outFilename);
		return 2;
	}

	write(fdout,outbuf,outbufp-outbuf);
	close(fdout);
	munmap(inbuf,statbuf.st_size);
	munmap(outbuf,statbuf.st_size);
	return 0;
}


int main (int argc, char ** argv){
	char * mpInFilename = "../data/mp-stage1";
	char * mpOutFilename = "../data/mp-stage2";
	char * wayInFilename = "../data/ways-stage1";
	char * wayOutFilename = "../data/ways-stage2";
	char * nodeInFilename = "../data/nodes-stage1";
	char * nodeOutFilename = "../data/nodes-stage2";
	
	way_tree = malloc(sizeof(struct tree_tree));
	node_tree = malloc(sizeof(struct tree_tree));
	tree_init(way_tree);
	tree_init(node_tree);
	loadFile(mpInFilename,mpOutFilename,mp_cb);
	loadFile(wayInFilename,wayOutFilename,way_cb);
	loadFile(nodeInFilename,nodeOutFilename,node_cb);

}
