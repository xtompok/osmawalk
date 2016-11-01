CC=gcc
PG=
# Hardcoded paths begin
INC=-I/usr/include/libucw  -I../ext-lib/libosmpbf -I/usr/include/postgresql
LDPATHS= -L../ext-lib/libosmpbf
#LIBCSV_C=../ext-lib/libcsv-3.0.3/libcsv.c
#Hardcoded paths end

CFLAGS= -c $(PG)  -ggdb3 -Wall -Wno-format -Wno-unused-function -std=gnu99 -O2 -march=native $(INC)
PFLAGS= -E -Wall -std=gnu99 -O3 -march=native -I/usr/include/libucw -I/mnt/sda1/home/jethro/Programy/libucw
LDFLAGS= $(PG) $(LDPATHS) -lprotobuf-c -lucw-6.5 -lproj -lyaml -lm -lz 

FILTER_SRC=raster.c hashes.c utils.c mixnum.c include/premap.pb-c.c include/types.pb-c.c include/graph.pb-c.c searchgraph.c
FILTER_OBJ=$(FILTER_SRC:.c=.o)
FILTER_EXE=filter

LIBSEARCH_SRC=writegpx.c include/types.pb-c.c include/graph.pb-c.c
LIBSEARCH_OBJ=$(LIBSEARCH_SRC:.c=.o)
LIBSEARCH_LIB=libsearch.so

SEARCH_SRC=writegpx.c include/graph.pb-c.c
SEARCH_OBJ=$(SEARCH_SRC:.c=.o)
SEARCH_LDFLAGS=-lsearch -L.
SEARCH_EXE=search

PARSE_SRC=yamlconf.c include/types.pb-c.c include/premap.pb-c.c
PARSE_OBJ=$(PARSE_SRC:.c=.o)
PARSE_LDFLAGS=-losm 
PARSE_EXE=parse

ORDER_SRC=include/premap.pb-c.c include/types.pb-c.c
ORDER_OBJ=$(ORDER_SRC:.c=.o)
ORDER_LDFLAGS= 
ORDER_EXE=order

TODB_SRC=include/premap.pb-c.c include/types.pb-c.c
TODB_OBJ=$(TODB_SRC:.c=.o)
TODB_LDFLAGS=-lpq
TODB_EXE=todb

CSVGRAPH_SRC=include/graph.pb-c.c include/types.pb-c.c searchgraph.c
CSVGRAPH_OBJ=$(CSVGRAPH_SRC:.c=.o)
CSVGRAPH_LDFLAGS=-lcsv
CSVGRAPH_EXE=csvtograph

# gcc -R -- runtime hledani knihovny
all: protobuf $(LIBSEARCH_LIB) $(SEARCH_EXE)
postgress: $(PARSE_EXE) $(TODB_EXE) $(CSVGRAPH_EXE)
old: $(FILTER_EXE) $(ORDER_EXE)

	
$(FILTER_EXE): $(FILTER_OBJ) $@.c 
	$(CC) $(CFLAGS) $@.c -o $@.o
	$(CC) $(FILTER_OBJ) $@.o -o $@ $(LDFLAGS)

$(LIBSEARCH_LIB): $(LIBSEARCH_OBJ) searchlib.c
	$(CC) $(CFLAGS) -fpic searchlib.c -o $@.o
	$(CC) -shared  $(LIBSEARCH_OBJ) $@.o -o $@ $(LDFLAGS)

$(SEARCH_EXE): $(LIBSEARCH_LIB) $(SEARCH_OBJ) $@.c
	$(CC) $(CFLAGS) $@.c -o $@.o
	$(CC) $(SEARCH_OBJ) $@.o -o $@ $(LDFLAGS) $(SEARCH_LDFLAGS) 
	
$(PARSE_EXE): $(PARSE_OBJ) $@.c 
	$(CC) $(CFLAGS) $@.c -o $@.o
	$(CC) $(PARSE_OBJ) $@.o -o $@ $(LDFLAGS) $(PARSE_LDFLAGS)

$(ORDER_EXE): $(ORDER_OBJ) $@.c 
	$(CC) $(CFLAGS) $@.c -o $@.o
	$(CC) $(ORDER_OBJ) $@.o -o $@ $(LDFLAGS) $(ORDER_LDFLAGS)

$(TODB_EXE): $(TODB_OBJ) $@.c 
	$(CC) $(CFLAGS) $@.c -o $@.o
	$(CC) $(TODB_OBJ) $@.o -o $@ $(LDFLAGS) $(TODB_LDFLAGS)

$(CSVGRAPH_EXE): $(CSVGRAPH_OBJ) $@.c 
	$(CC) $(CFLAGS) $@.c -o $@.o
	$(CC) $(CSVGRAPH_OBJ) $@.o -o $@ $(LDFLAGS) $(CSVGRAPH_LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -fpic $< -o $@

preprocess: filter.c
	$(CC) $(PFLAGS) $< -o $@

clean:
	rm -f *.o *.so include/*.o $(CSVGRAPH_EXE) $(TODB_EXE) $(ORDER_EXE) $(PARSE_EXE) $(SEARCH_EXE)

protobuf: ../config/types.proto ../config/premap.proto ../config/graph.proto 
	mkdir -p include
	protoc-c --proto_path ../config/ ../config/types.proto --c_out include/
	protoc-c --proto_path ../config/ ../config/premap.proto --c_out include/
	protoc-c --proto_path ../config/ ../config/graph.proto --c_out include/

.PHONY: protobuf
