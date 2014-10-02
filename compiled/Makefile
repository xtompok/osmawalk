CC=gcc
PG=
INC=-I/usr/include/libucw -I/mnt/sda1/home/jethro/Programy/libucw -I/aux/jethro/libucw -I../ext-lib/libosm-0.3
CFLAGS= -c $(PG)  -ggdb3 -Wall -Wno-format -std=gnu99 -O2 -march=native $(INC)
PFLAGS= -E -Wall -std=gnu99 -O3 -march=native -I/usr/include/libucw -I/mnt/sda1/home/jethro/Programy/libucw
LDFLAGS= $(PG) -lprotobuf-c -lucw-6.0 -lproj -lyaml -lm -L/mnt/sda1/home/jethro/Programy/libucw/obj/ucw -L/aux/jethro/libucw/obj/ucw 

FILTER_SRC=raster.c hashes.c utils.c mixnum.c include/premap.pb-c.c include/types.pb-c.c include/graph.pb-c.c
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
PARSE_LDFLAGS=-losm -L../ext-lib/libosm-0.3
PARSE_EXE=parse

ORDER_SRC=include/premap.pb-c.c include/types.pb-c.c
ORDER_OBJ=$(ORDER_SRC:.c=.o)
ORDER_LDFLAGS= -L../ext-lib/libosm-0.3
ORDER_EXE=order

# gcc -R -- runtime hledani knihovny
all: protobuf $(SOURCES) $(FILTER_EXE) $(LIBSEARCH_LIB) $(SEARCH_EXE)

	
$(FILTER_EXE): $(FILTER_OBJ) $@.c 
	$(CC) $(CFLAGS) $@.c -o $@.o
	$(CC) $(LDFLAGS) $(FILTER_OBJ) $@.o -o $@

$(LIBSEARCH_LIB): $(LIBSEARCH_OBJ) searchlib.c
	$(CC) $(CFLAGS) -fpic searchlib.c -o $@.o
	$(CC) $(LDFLAGS) -shared  $(LIBSEARCH_OBJ) $@.o -o $@

$(SEARCH_EXE): $(LIBSEARCH_LIB) $(SEARCH_OBJ) $@.c
	$(CC) $(CFLAGS) $@.c -o $@.o
	$(CC) $(LDFLAGS) $(SEARCH_LDFLAGS) $(SEARCH_OBJ) $@.o -o $@
	
$(PARSE_EXE): $(PARSE_OBJ) $@.c 
	$(CC) $(CFLAGS) $@.c -o $@.o
	$(CC) $(LDFLAGS) $(PARSE_LDFLAGS) $(PARSE_OBJ) $@.o -o $@

$(ORDER_EXE): $(ORDER_OBJ) $@.c 
	$(CC) $(CFLAGS) $@.c -o $@.o
	$(CC) $(LDFLAGS) $(ORDER_LDFLAGS) $(ORDER_OBJ) $@.o -o $@

.c.o:
	$(CC) $(CFLAGS) -fpic $< -o $@

preprocess: filter.c
	$(CC) $(PFLAGS) $< -o $@

clean:
	rm *.o *.so include/*.o

protobuf:
	mkdir -p include
	protoc-c --proto_path ../config/ ../config/types.proto --c_out include/
	protoc-c --proto_path ../config/ ../config/premap.proto --c_out include/
	protoc-c --proto_path ../config/ ../config/graph.proto --c_out include/

.PHONY: protobuf
