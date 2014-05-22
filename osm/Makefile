CC=gcc
CFLAGS= -c -Wall -std=gnu99 -O2 -march=native  
LDFLAGS= -lz -lm  
SOURCE1= osmconvert.c 
SOURCE2= merge-srtm.c
OBJECT1=$(SOURCE1:.c=.o)
OBJECT2=$(SOURCE2:.c=.o)
EXECUTABLE1=osmconvert
EXECUTABLE2=merge-srtm

all: $(SOURCES) $(EXECUTABLE1) $(EXECUTABLE2)
	
$(EXECUTABLE1): $(OBJECT1) $@.c
	$(CC) $(CFLAGS) $@.c -o $@.o
	$(CC) $(LDFLAGS) $(OBJECTS1) $@.o -o $@

$(EXECUTABLE2): $(OBJECT2) $@.c
	$(CC) $(CFLAGS) $@.c -o $@.o
	$(CC) $(LDFLAGS) $(OBJECTS2) $@.o -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@


clean:
	rm *.o

