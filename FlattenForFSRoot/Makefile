#! gnumake

ROOTFLAGS     = $(shell root-config --cflags)
ROOTLIBS      = $(shell root-config --libs) -lMinuit -lMathCore

default: flatten

%: %.cc
	g++  $(ROOTFLAGS) $(MYINCLUDES) -c -o $*.o $*.cc
	g++  $(ROOTFLAGS) $(MYINCLUDES) $(ROOTLIBS) $*.o -o $*


clean:
	@rm -f *.o $* */*.o
