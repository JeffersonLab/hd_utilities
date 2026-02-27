#! gnumake

unexport CPATH

CXX = $(CONDA_PREFIX)/bin/x86_64-conda-linux-gnu-g++

SYSROOT = $(CONDA_PREFIX)/x86_64-conda-linux-gnu/sysroot
GCCVER = $(shell $(CXX) -dumpfullversion)

GCCLIB = $(CONDA_PREFIX)/lib/gcc/x86_64-conda-linux-gnu/$(GCCVER)
CXXINC = $(GCCLIB)/include/c++


SYSROOTFLAGS = --sysroot=$(SYSROOT) -nostdinc \
	       -isystem $(CXXINC) \
	       -isystem $(CXXINC)/x86_64-conda-linux-gnu \
	       -isystem $(CXXINC)/backward \
	       -isystem $(GCCLIB)/include \
	       -isystem $(GCCLIB)/include-fixed \
	       -isystem $(SYSROOT)/usr/include \
	       -isystem $(SYSROOT)/usr/include/x86_64-conda-linux-gnu

ROOTFLAGS     = $(shell $(CONDA_PREFIX)/bin/root-config --cflags)
ROOTLIBS      = $(shell $(CONDA_PREFIX)/bin/root-config --libs) -lMinuit -lMathCore

EXE_SRCFILES := $(wildcard *.cc)
EXE_EXEFILES := $(foreach file,$(EXE_SRCFILES),$(file:.cc=))

default: $(EXE_EXEFILES)

%: %.cc
	$(CXX)  $(SYSROOTFLAGS) $(ROOTFLAGS) $(MYINCLUDES) -c -o $*.o $*.cc
	$(CXX)  $(SYSROOTFLAGS) $(ROOTFLAGS) $(MYINCLUDES) $(ROOTLIBS) $*.o -o $*


clean:
	@rm -f *.o $(EXE_EXEFILES)

