CC=gcc
CFLAGS= -c -Wall -std=c99 -O3 -march=native -I/usr/include/libucw
LDFLAGS= -lprotobuf-c -lucw-6.0 
SOURCES=filter.c include/premap.pb-c.c include/types.pb-c.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=filter

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
