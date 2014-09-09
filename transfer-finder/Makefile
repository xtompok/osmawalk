CC=gcc
PG=
CFLAGS= -c $(PG)  -ggdb3 -Wall -Wno-format -std=gnu99 -O2 -march=native -I/usr/include/libucw -I/mnt/sda1/home/jethro/Programy/libucw -I/aux/jethro/libucw
PFLAGS= -E -Wall -std=gnu99 -O3 -march=native -I/usr/include/libucw -I/mnt/sda1/home/jethro/Programy/libucw
LDFLAGS= $(PG) -lprotobuf-c -lucw-6.0 -lproj -lyaml -lm -L/mnt/sda1/home/jethro/Programy/libucw/obj/ucw -L/aux/jethro/libucw/obj/ucw 
C_DIR=../compiled
SOURCES2= $(C_DIR)/writegpx.c $(C_DIR)/searchlib.c $(C_DIR)/include/types.pb-c.c $(C_DIR)/include/graph.pb-c.c
OBJECTS2=$(SOURCES2:.c=.o)
EXECUTABLE2=search-transfer

all: protobuf $(SOURCES) $(EXECUTABLE2)

	
$(EXECUTABLE1): $(OBJECTS1) $@.c 
	$(CC) $(CFLAGS) $@.c -o $@.o
	$(CC) $(LDFLAGS) $(OBJECTS1) $@.o -o $@

$(EXECUTABLE2): $(OBJECTS2) $@.c
	$(CC) $(CFLAGS) $@.c -o $@.o
	$(CC) $(LDFLAGS) $(OBJECTS2) $@.o -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

preprocess: filter.c
	$(CC) $(PFLAGS) $< -o $@

clean:
	rm *.o include/*.o

protobuf:
	mkdir -p include
	protoc-c --proto_path ../config/ ../config/types.proto --c_out include/
	protoc-c --proto_path ../config/ ../config/premap.proto --c_out include/
	protoc-c --proto_path ../config/ ../config/graph.proto --c_out include/

.PHONY: protobuf
