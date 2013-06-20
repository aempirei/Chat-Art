CC = gcc
CXX = g++
CFLAGS = -Wall -W -pedantic -std=gnu99 -O2
CXXFLAGS = -Wall -W -pedantic -std=gnu++11 -O2
CPPFLAGS = -I.
LIBFLAGS =
SOURCE = pgmtobraile.c pgmtobrailedux.c calibration.cc
BUILD = $(SOURCE:%=build.%)
WIPE = $(SOURCE:%=wipe.%)

.PHONY : build wipe clean $(BUILD) $(WIPE)

build : $(BUILD)

wipe : clean $(WIPE)

clean :
	rm -f *.o *~ *.dvi *.aux

%.pdf : %.dvi
	dvipdf $^

%.dvi : %.tex
	latex $^
	latex $^

build.pgmtobraile.c: pgmtobraile.o pgmtobraile

wipe.pgmtobraile.c:
	rm -f pgmtobraile

pgmtobraile: pgmtobraile.o
	$(CC) $(CCFLAGS) -o $@ $^ $(LIBFLAGS)

build.pgmtobrailedux.c: pgmtobrailedux.o pgmtobrailedux

wipe.pgmtobrailedux.c:
	rm -f pgmtobrailedux

pgmtobrailedux: pgmtobrailedux.o
	$(CC) $(CCFLAGS) -o $@ $^ $(LIBFLAGS)

build.calibration.cc: calibration.o calibration

wipe.calibration.cc:
	rm -f calibration

calibration: calibration.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBFLAGS)

