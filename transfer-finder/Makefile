# Hardcoded paths begin
INC=-I/usr/include/libucw 
#Hardcoded paths end

CC=gcc
PG=
CFLAGS= -c $(PG)  -ggdb3 -Wall -Wno-format -std=gnu99 -O2 -march=native $(INC) 
PFLAGS= -E -Wall -std=gnu99 -O3 -march=native $(INC)
LDFLAGS= $(PG) -lprotobuf-c -lucw-6.5 -lproj -lyaml -lm $(LDPATHS) 
C_DIR=../compiled
SOURCES2= $(C_DIR)/writegpx.c $(C_DIR)/searchlib.c $(C_DIR)/include/types.pb-c.c $(C_DIR)/include/graph.pb-c.c
OBJECTS2=$(SOURCES2:.c=.o)
EXECUTABLE2=search-transfer

all: protobuf $(SOURCES) $(EXECUTABLE2)
	
$(EXECUTABLE2): $(OBJECTS2) $@.c
	$(CC) $(CFLAGS) $@.c -o $@.o
	$(CC) $(OBJECTS2) $@.o -o $@ $(LDFLAGS) 

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o include/*.o

protobuf:
	mkdir -p include
	protoc-c --proto_path ../config/ ../config/types.proto --c_out include/
	protoc-c --proto_path ../config/ ../config/premap.proto --c_out include/
	protoc-c --proto_path ../config/ ../config/graph.proto --c_out include/

.PHONY: protobuf
