CC=gcc
CFLAGS= -c -g -Wall -Wformat -std=gnu99 -O3 -march=native -I/usr/include/libucw
PFLAGS= -E -Wall -std=gnu99 -O3 -march=native -I/usr/include/libucw
LDFLAGS= -lprotobuf-c -lucw-6.0 -lproj 
SOURCES=filter.c include/premap.pb-c.c include/types.pb-c.c include/geodesic.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=filter

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

preprocess: filter.c
	$(CC) $(PFLAGS) $< -o $@
