CC=gcc
CFLAGS= -c -g -Wall -Wno-format -std=gnu99 -O0 -march=native -I/usr/include/libucw -I/mnt/sda1/home/jethro/Programy/libucw
PFLAGS= -E -Wall -std=gnu99 -O3 -march=native -I/usr/include/libucw -I/mnt/sda1/home/jethro/Programy/libucw
LDFLAGS= -lprotobuf-c -lucw-6.0 -lproj -lyaml -lm -L/mnt/sda1/home/jethro/Programy/libucw/obj/ucw 
SOURCES=raster.c hashes.c utils.c writegpx.c include/premap.pb-c.c include/types.pb-c.c include/graph.pb-c.c include/geodesic.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE1=filter
EXECUTABLE2=search

all: $(SOURCES) $(EXECUTABLE1) $(EXECUTABLE2)

	
$(EXECUTABLE1): $(OBJECTS) $@.c 
	$(CC) $(CFLAGS) $@.c -o $@.o
	$(CC) $(LDFLAGS) $(OBJECTS) $@.o -o $@

$(EXECUTABLE2): $(OBJECTS) $@.c
	$(CC) $(CFLAGS) $@.c -o $@.o
	$(CC) $(LDFLAGS) $(OBJECTS) $@.o -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

preprocess: filter.c
	$(CC) $(PFLAGS) $< -o $@

clean:
	rm *.o include/*.o

protobuf:
	protoc-c --proto_path ../config/ ../config/types.proto --c_out include/
	protoc-c --proto_path ../config/ ../config/premap.proto --c_out include/
	protoc-c --proto_path ../config/ ../config/graph.proto --c_out include/

.PHONY: protobuf
