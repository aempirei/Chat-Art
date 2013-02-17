CC = gcc
CCFLAGS = -Wall -W -w
CFLAGS = -Wall -W -I. -pedantic -std=gnu99
LIBFLAGS =
PROGRAMS = calibrate pgmtobraile

.PHONY: all clean wipe

all: $(PROGRAMS)

calibrate: calibrate.o
	$(CC) $(CCFLAGS) -o $@ $^ $(LIBFLAGS) -lm

pgmtobraile: pgmtobraile.o
	$(CC) $(CCFLAGS) -o $@ $^ $(LIBFLAGS)

clean:
	rm -f *.o *~

wipe: clean
	rm -f $(PROGRAMS)
