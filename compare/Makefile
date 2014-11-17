#! gnumake

ROOTFLAGS     = $(shell root-config --cflags)
ROOTLIBS      = $(shell root-config --libs) -lMinuit -lMathCore # -lMathMore -lgsl -lgslcblas
ALL_LIBS += $(ROOTLIBS)
MY_FLAGS += $(ROOTFLAGS)
# INCLUDE_DIRS += -I${HOME}/include/ -I${HALLD_HOME}/src/libraries/include/
# MY_LIBS = objects/Xi_KpKp_selector.o objects/eventdict.o  objects/Xi_KpKp_Thrown_selector.o objects/eventdict_thrown.o

all:  rootcompare

%: %.cc
	g++  $(MY_FLAGS) $(INCLUDE_DIRS)  -c -o objects/$*.o $*.cc
	g++  $(MY_FLAGS) $(INCLUDE_DIRS) $(ALL_LIBS) $(MY_LIBS) objects/$*.o -o $*

objects/%.o: %.h %.C
	g++ $(ROOTFLAGS)  $(INCLUDE_DIRS) -c -o objects/$*.o $*.C

#.PHONY: clean	
clean:
	@rm -f objects/*.o $* rootcompare
