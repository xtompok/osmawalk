#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "include/types.pb-c.h"
#include "include/premap.pb-c.h"


char node_cb(size_t len,uint8_t * inbuf){
	Premap__Node * node;
	node = premap__node__unpack(NULL,len,inbuf);
//	fprintf(stderr,"Node: %lld\n",node->id);
//	fprintf(stderr,"Lat: %lld, lon: %lld\n",node->lat,node->lon);

//	id,lat,lon,height,objtype,inside,intunnel,onbridge,loc,sq1,sq2
	printf("%lld\t%f\t%f\t%d\t%d\t%d\t%d\t%d\t%d\t%s\tSRID=3065;POINT(%f %f %d)\t%d\t%d\n",
//		id    lat lon hei typ ins int onb stp ref 
		node->id,
		node->lat,
		node->lon,
		node->height,
		node->objtype,
		node->inside,
		node->intunnel,
		node->onbridge,
		node->stop,
		node->stop?(node->ref?node->ref:""):"",
		node->lon,
		node->lat,
		node->height,
		node->square1,
		node->square2);
	premap__node__free_unpacked(node,NULL);
	return 1;
}

char way_cb(size_t len,uint8_t * inbuf){
	Premap__Way * way;
	way = premap__way__unpack(NULL,len,inbuf);
//	fprintf(stderr,"Way: %lld\n",way->id);

//	id,area,barrier,type,bridge,tunnel,wayidx
	printf("%lld\t%d\t%d\t%d\t%d\t%d\t%d\n",
		way->id,
		way->area,
		way->barrier,
		way->type,
		way->bridge,
		way->tunnel,
		way->wayidx);
	premap__way__free_unpacked(way,NULL);
	return 1;
}

char way_refs_cb(size_t len,uint8_t * inbuf){
	Premap__Way * way;
	way = premap__way__unpack(NULL,len,inbuf);
//	fprintf(stderr,"Way: %lld\n",way->id);
//	id,ref,order
	for (int i=0;i<way->n_refs;i++){
		printf("%lld\t%lld\t%d\n",way->id,way->refs[i],i);
	}
	premap__way__free_unpacked(way,NULL);
	return 1;
}

char mp_cb(size_t len,uint8_t * inbuf){
	Premap__Multipolygon * mp;
	mp = premap__multipolygon__unpack(NULL,len,inbuf);
//	fprintf(stderr,"MP: %lld\n",mp->id);
//	id,type
	printf("%lld\t%d\n",
		mp->id,
		mp->type);
	premap__multipolygon__free_unpacked(mp,NULL);
	return 1;
}
char mp_refs_cb(size_t len,uint8_t * inbuf){
	Premap__Multipolygon * mp;
	mp = premap__multipolygon__unpack(NULL,len,inbuf);
//	fprintf(stderr,"MP: %lld\n",mp->id);
//	id,ref,role
	for (int i=0;i<mp->n_refs;i++){
		printf("%lld\t%lld\t%d\n",
			mp->id,
			mp->refs[i],
			mp->roles[i]);
	}
	premap__multipolygon__free_unpacked(mp,NULL);
	return 1;
}

int loadFile (char * inFilename,  
		char (callback)(size_t len, uint8_t * inbuf)){
	int fdin;
	fdin = open(inFilename,O_RDONLY);
	if (fdin < 0){
		fprintf(stderr,"Can't open file %s, exitting\n",inFilename);
		return 1;
	}

/*	int fdout;
	fdout = open(outFilename,O_RDWR|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (fdout < 0){
		printf("Can't open file %s, exitting\n",outFilename);
		return 2;
	}
*/
	struct stat statbuf;
	fstat (fdin,&statbuf);
	
//	lseek(fdout, statbuf.st_size - 1, SEEK_SET);
//	write(fdout,"",1);

	uint8_t * inbuf;
	inbuf = mmap(NULL,statbuf.st_size,PROT_READ,MAP_PRIVATE,fdin,0);

//	char * outbuf;
//	outbuf = mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fdout,0);

	uint8_t * inbufp;
	inbufp = inbuf;
//	outbufp = outbuf;
	fprintf(stderr,"File size: %d\n",statbuf.st_size);
	while (inbufp<(inbuf+statbuf.st_size)) {
		size_t len;
		memcpy(&len,inbufp,sizeof(size_t));
		inbufp += sizeof(size_t);
		callback(len,inbufp);
		inbufp+=len;
	}
	munmap(inbuf,statbuf.st_size);
//	munmap(outbuf,statbuf.st_size);
	return 0;
}


int main (int argc, char ** argv){
	char * mpInFilename = "../data/mp-stage1";
	char * wayInFilename = "../data/ways-stage1";
	char * nodeInFilename = "../data/nodes-stage1";

//	PGconn * conn;
//	conn = connectToDb();

	switch (argv[1][0]){
		case 'n':
			loadFile(nodeInFilename,node_cb);
		break;
		case 'w':
			loadFile(wayInFilename,way_cb);
		break;
		case 'v':
			loadFile(wayInFilename,way_refs_cb);
		break;
		case 'm':
			loadFile(mpInFilename,mp_cb);
		break;
		case 'l':
			loadFile(mpInFilename,mp_refs_cb);
		break;
			
	}

}
