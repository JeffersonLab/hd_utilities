#! gnumake

ROOTFLAGS     = $(shell root-config --cflags)
ROOTLIBS      = $(shell root-config --libs) -lMinuit -lMathCore # -lMathMore -lgsl -lgslcblas
ALL_LIBS += $(ROOTLIBS)
MY_FLAGS += $(ROOTFLAGS)

all: format_results_to_html process_timePlugin_mem_vmem

%: %.cc $(MY_LIBS) 
	g++  $(MY_FLAGS) $(INCLUDE_DIRS)  -c -o $*.o $*.cc
	g++  $(MY_FLAGS) $(INCLUDE_DIRS) $(ALL_LIBS) $(MY_LIBS) $*.o -o $*

#.PHONY: clean  
clean:
	@rm -f format_results_to_html process_timePlugin_mem_vmem
