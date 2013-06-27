CC = gcc
CXX = g++
CFLAGS = -Wall -W -pedantic -std=gnu99 -O2
CXXFLAGS = -Wall -W -pedantic -std=gnu++11 -O2
CPPFLAGS = -Iinclude
LIBFLAGS =
SOURCE = calibration.cc palette.cc pgmtobraile.c pgmtobrailedux.c pnmtoansi.cc
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

build.calibration.cc: calibration.o calibration

wipe.calibration.cc:
	rm -f calibration

calibration: calibration.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBFLAGS)

build.palette.cc: palette.o palette

wipe.palette.cc:
	rm -f palette

palette: palette.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBFLAGS)

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

build.pnmtoansi.cc: pnmtoansi.o pnmtoansi

wipe.pnmtoansi.cc:
	rm -f pnmtoansi

pnmtoansi: pnmtoansi.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBFLAGS)

