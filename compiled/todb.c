#include <ucw/lib.h>
#include <ucw/fastbuf.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <libpq-fe.h>

#include <ucw/gary.h>
#include <osm.h>

#include "include/types.pb-c.h"
#include "include/premap.pb-c.h"
#include "order.h"


char node_cb(size_t len,char * inbuf, PGconn * conn){
	Premap__Node * node;
	node = premap__node__unpack(NULL,len,inbuf);
	printf("Node: %lld\n",node->id);
	printf("Lat: %lld, lon: %lld\n",node->lat,node->lon);
/*	if (tree_find(node_tree,node->id)||(node->objtype!=OBJTYPE__NONE)){
		printf("Found\n");
		memcpy(outbuf,&len,sizeof(size_t));
		outbuf+=sizeof(size_t);
		memcpy(outbuf,inbuf,len);
		premap__node__free_unpacked(node,NULL);
		return outbuf+len;
	}*/
	char query[1000];
	snprintf(query,1000,"INSERT INTO nodes (id,lat,lon,height,loc) VALUES (%d,%d,%d,%d,ST_GeomFromText('POINT(%d %d)',3065));",
		node->id,
		node->lat,
		node->lon,
		node->height,
		node->lat,
		node->lon);
	PGresult * result;
	result = PQexec(conn,query);
	if (PQresultStatus(result)==PGRES_COMMAND_OK){
		printf("OK\n");
	}else
	{
		printf("Error: %s\n",PQresStatus(PQresultStatus(result)));
	}
	premap__node__free_unpacked(node,NULL);
	return 1;
}

char way_cb(size_t len,char * inbuf, PGconn * conn){
	Premap__Way * way;
	way = premap__way__unpack(NULL,len,inbuf);
	printf("Way: %lld\n",way->id);
/*	if (tree_find(way_tree,way->id)||(way->type!=OBJTYPE__NONE))
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
	}*/
	premap__way__free_unpacked(way,NULL);
	return 1;
}

char mp_cb(size_t len,char * inbuf, PGconn * conn){
	Premap__Multipolygon * mp;
	mp = premap__multipolygon__unpack(NULL,len,inbuf);
	printf("MP: %lld\n",mp->id);
/*	for (int i=0; i<mp->n_refs;i++){
		tree_lookup(way_tree,mp->refs[i]);
		premap__multipolygon__free_unpacked(mp,NULL);
		memcpy(outbuf,&len,sizeof(size_t));
		outbuf+=sizeof(size_t);
		memcpy(outbuf,inbuf,len);
		return outbuf+len;
	}*/
	premap__multipolygon__free_unpacked(mp,NULL);
	return 1;
}

int loadFile (char * inFilename, PGconn * conn, 
		char (callback)(size_t len, char * inbuf,PGconn * conn)){
	int fdin;
	fdin = open(inFilename,O_RDONLY);
	if (fdin < 0){
		printf("Can't open file %s, exitting\n",inFilename);
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

	char * inbuf;
	inbuf = mmap(NULL,statbuf.st_size,PROT_READ,MAP_PRIVATE,fdin,0);

//	char * outbuf;
//	outbuf = mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fdout,0);

	char * inbufp;
	char status;
	inbufp = inbuf;
//	outbufp = outbuf;
	printf("File size: %d\n",statbuf.st_size);
	while (inbufp<(inbuf+statbuf.st_size)) {
		size_t len;
		memcpy(&len,inbufp,sizeof(size_t));
		inbufp += sizeof(size_t);
		status = callback(len,inbufp,conn);
		inbufp+=len;
	}
	munmap(inbuf,statbuf.st_size);
//	munmap(outbuf,statbuf.st_size);
	return 0;
}

PGconn * connectToDb(){
	const char host[] = "localhost";
	const char dbname[] = "osmwalk-prepare";
	const char user[] = "jethro";
	const char pwd[] = "kokoko";
	const char * keys [] = {"host","dbname","user","password",NULL};
	const char * vals [] = {host,dbname,user,pwd,NULL};
	PGconn * conn;
	conn = PQconnectdbParams(keys,vals,0);
	if (PQstatus(conn) == CONNECTION_OK){
		printf("Connection OK\n");
	}
	return conn;

}


int main (int argc, char ** argv){
	char * mpInFilename = "../data/mp-stage2";
	char * wayInFilename = "../data/ways-stage2";
	char * nodeInFilename = "../data/nodes-stage2";

	PGconn * conn;
	conn = connectToDb();
	
//	loadFile(mpInFilename,conn,mp_cb);
//	loadFile(wayInFilename,conn,way_cb);
	loadFile(nodeInFilename,conn,node_cb);

}
