CC=gcc
PG=
# Hardcoded paths begin
LIBOSMPBFPATH=../ext-lib/libosmpbf
RAPTORPATH=../ext-lib/mmpf/raptor/
#Hardcoded paths end

INC=-I/usr/include/libucw  -I$(LIBOSMPBFPATH) -I$(RAPTORPATH) -I/usr/include/postgresql
CFLAGS= -c $(PG)  -ggdb3 -Wall -Wno-format -Wno-unused-function -std=gnu99 -O2 -march=native $(INC)
PFLAGS= -E -Wall -std=gnu99 -O3 -march=native -I/usr/include/libucw 
LDFLAGS= $(PG) -L$(LIBOSMPBFPATH) -L$(RAPTORPATH) -lraptor -lprotobuf-c -lucw-6.5 -lproj -lyaml -lm -lz 

LIBSEARCH_SRC=writegpx.c include/types.pb-c.c include/graph.pb-c.c include/result.pb-c.c
LIBSEARCH_OBJ=$(LIBSEARCH_SRC:.c=.o)
LIBSEARCH_LIB=libsearch.so

SEARCH_SRC=writegpx.c include/graph.pb-c.c
SEARCH_OBJ=$(SEARCH_SRC:.c=.o)
SEARCH_LDFLAGS=-lsearch -L. -Wl,-rpath,'$$ORIGIN' -Wl,-rpath,'$$ORIGIN/$(RAPTORPATH)'
SEARCH_EXE=search

PARSE_SRC=yamlconf.c include/types.pb-c.c include/premap.pb-c.c
PARSE_OBJ=$(PARSE_SRC:.c=.o)
PARSE_LDFLAGS=-losm -Wl,-rpath,'$$ORIGIN/$(LIBOSMPBFPATH)' 
PARSE_EXE=parse

TODB_SRC=include/premap.pb-c.c include/types.pb-c.c
TODB_OBJ=$(TODB_SRC:.c=.o)
TODB_LDFLAGS=
TODB_EXE=todb

CSVGRAPH_SRC=include/graph.pb-c.c include/types.pb-c.c searchgraph.c
CSVGRAPH_OBJ=$(CSVGRAPH_SRC:.c=.o)
CSVGRAPH_LDFLAGS=-lcsv
CSVGRAPH_EXE=csvtograph

# gcc -R -- runtime hledani knihovny
all: postgres search
search: protobuf $(LIBSEARCH_LIB) $(SEARCH_EXE)
postgres: protobuf $(PARSE_EXE) $(TODB_EXE) $(CSVGRAPH_EXE)

$(LIBSEARCH_LIB): $(LIBSEARCH_OBJ) searchlib.c
	$(CC) $(CFLAGS) -fpic searchlib.c -o $@.o
	$(CC) -shared  $(LIBSEARCH_OBJ) $@.o -o $@ $(LDFLAGS)

$(SEARCH_EXE): $(LIBSEARCH_LIB) $(SEARCH_OBJ) $@.c
	$(CC) $(CFLAGS) $@.c -o $@.o
	$(CC) $(SEARCH_OBJ) $@.o -o $@ $(LDFLAGS) $(SEARCH_LDFLAGS) 
	
$(PARSE_EXE): $(PARSE_OBJ) $@.c 
	$(CC) $(CFLAGS) $@.c -o $@.o
	$(CC) $(PARSE_OBJ) $@.o -o $@ $(LDFLAGS) $(PARSE_LDFLAGS)

$(TODB_EXE): $(TODB_OBJ) $@.c 
	$(CC) $(CFLAGS) $@.c -o $@.o
	$(CC) $(TODB_OBJ) $@.o -o $@ $(LDFLAGS) $(TODB_LDFLAGS)

$(CSVGRAPH_EXE): $(CSVGRAPH_OBJ) $@.c 
	$(CC) $(CFLAGS) $@.c -o $@.o
	$(CC) $(CSVGRAPH_OBJ) $@.o -o $@ $(LDFLAGS) $(CSVGRAPH_LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -fpic $< -o $@

clean:
	rm -f *.o *.so include/*.o $(CSVGRAPH_EXE) $(TODB_EXE) $(PARSE_EXE) $(SEARCH_EXE)

protobuf: ../config/types.proto ../config/premap.proto ../config/graph.proto 
	mkdir -p include
	protoc-c --proto_path ../config/ ../config/types.proto --c_out include/
	protoc-c --proto_path ../config/ ../config/premap.proto --c_out include/
	protoc-c --proto_path ../config/ ../config/graph.proto --c_out include/
	protoc-c --proto_path ../config/ ../config/result.proto --c_out include/
	protoc --proto_path ../config ../config/types.proto --python_out ../webapp/
	protoc --proto_path ../config ../config/result.proto --python_out ../webapp/

.PHONY: protobuf all search postgres
