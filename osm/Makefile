CC=gcc
CFLAGS= -c -Wall -std=gnu99 -O2 -march=native  
LDFLAGS= -lz -lm  

CONVERT_SRC=osmconvert.c 
CONVERT_OBJ=$(CONVERT_SRC:.c=.o)
CONVERT_EXE=osmconvert

MERGE_SRC=merge-srtm.c
MERGE_OBJ=$(MERGE_SRC:.c=.o)
MERGE_EXE=merge-srtm

all: $(CONVERT_EXE) $(MERGE_EXE)
	
$(CONVERT_EXE): $(CONVERT_OBJ)
	$(CC) $(CONVERT_OBJ) -o $@ $(LDFLAGS)

$(MERGE_EXE): $(MERGE_OBJ)
	$(CC) $(MERGE_OBJ) -o $@ $(LDFLAGS)

clean:
	rm -f  *.o $(MERGE_EXE) $(CONVERT_EXE)

.PHONY: all clean
