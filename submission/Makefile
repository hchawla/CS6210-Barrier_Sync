OMPFLAGS = -fopenmp
OMPLIBS = -lgomp

CC = gcc
CPPFLAGS = -g -Wall 

LDLIBS = $(OMPLIBS)

all: Sense Tournament

Sense: Sense.c
	$(CC) -o $@ $(LDFLAGS) $^ $(LDLIBS) $(OMPFLAGS) $(OMPLIBS) -lm

Tournament: Tournament.c
	$(CC) -o $@ $(LDFLAGS) $^ $(LDLIBS) $(OMPFLAGS) $(OMPLIBS) -lm

clean:
	rm -f *.o Tournament Sense

