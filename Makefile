CC = gcc
CXX = g++
CFLAGS = -Wall -W -pedantic -std=gnu99 -O2
CXXFLAGS = -Wall -W -pedantic -std=gnu++11 -O2
CPPFLAGS = -I.
LIBFLAGS =
PROGRAMS = auto.calibration.cc auto.pgmtobraile.c auto.pgmtobrailedux.c

.PHONY: all clean wipe auto.calibration.cc auto.pgmtobraile.c auto.pgmtobrailedux.c

all: $(PROGRAMS)

auto.calibration.cc: calibration

calibration: calibration.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBFLAGS)

auto.pgmtobraile.c: pgmtobraile

pgmtobraile: pgmtobraile.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBFLAGS)

auto.pgmtobrailedux.c: pgmtobrailedux

pgmtobrailedux: pgmtobrailedux.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBFLAGS)

clean:
	rm -f *.o *~ *.dvi *.aux
