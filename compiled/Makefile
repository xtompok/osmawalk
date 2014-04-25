CC=gcc
CFLAGS= -c -g -Wall -Wno-format -std=gnu99 -O0 -march=native -I/usr/include/libucw -I/mnt/sda1/home/jethro/Programy/libucw
PFLAGS= -E -Wall -std=gnu99 -O3 -march=native -I/usr/include/libucw -I/mnt/sda1/home/jethro/Programy/libucw
LDFLAGS= -lprotobuf-c -lucw-6.0 -lproj -lm -L/mnt/sda1/home/jethro/Programy/libucw/obj/ucw 
SOURCES=filter.c raster.c hashes.c utils.c include/premap.pb-c.c include/types.pb-c.c include/geodesic.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=filter

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

preprocess: filter.c
	$(CC) $(PFLAGS) $< -o $@

clean:
	rm *.o include/*.o

protobuf:
	protoc-c --proto_path ../scripts/filter/ ../scripts/filter/types.proto --c_out include/
	protoc-c --proto_path ../scripts/filter/ ../scripts/filter/premap.proto --c_out include/

.PHONY: protobuf
