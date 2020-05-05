#! gnumake

ROOTFLAGS     = $(shell root-config --cflags)
ROOTLIBS      = $(shell root-config --libs) -lMinuit -lMathCore

EXE_SRCFILES := $(wildcard *.cc)
EXE_EXEFILES := $(foreach file,$(EXE_SRCFILES),$(file:.cc=))

default: $(EXE_EXEFILES)

%: %.cc
	g++  $(ROOTFLAGS) $(MYINCLUDES) -c -o $*.o $*.cc
	g++  $(ROOTFLAGS) $(MYINCLUDES) $(ROOTLIBS) $*.o -o $*


clean:
	@rm -f *.o $(EXE_EXEFILES)

