CC = gcc
CXX = g++
CFLAGS = -Wall -W -pedantic -std=gnu99
CXXFLAGS = -Wall -W -pedantic -std=gnu++11
CPPFLAGS = -I.
LIBFLAGS =
PROGRAMS = auto.calibration.cc auto.calibrate.c auto.pgmtobraile.c auto.pgmtobrailedux.c

.PHONY: all clean wipe auto.calibration.cc auto.calibrate.c auto.pgmtobraile.c auto.pgmtobrailedux.c

all: $(PROGRAMS)

auto.calibration.cc: calibration

calibration: calibration.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBFLAGS)

auto.calibrate.c: calibrate

calibrate: calibrate.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBFLAGS) -lm

auto.pgmtobraile.c: pgmtobraile

pgmtobraile: pgmtobraile.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBFLAGS)

auto.pgmtobrailedux.c: pgmtobrailedux

pgmtobrailedux: pgmtobrailedux.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBFLAGS)

clean:
	rm -f *.o *~ *.dvi *.aux
