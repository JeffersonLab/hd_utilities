#! gnumake

ROOTFLAGS     = $(shell root-config --cflags)
ROOTLIBS      = $(shell root-config --libs) -lMinuit -lMathCore

#UTILITIES_LIBS = RootMacros/Utilities/ControlVariables.o RootMacros/Utilities/CanvasUtilities.o RootMacros/Utilities/HistogramUtilities.o RootMacros/Utilities/SystemUtilities.o RootMacros/Utilities/StringUtilities.o RootMacros/Utilities/PhysicsUtilities.o
#MODEMACROS_LIBS = RootMacros/ModeMacros/ModeInfo.o RootMacros/ModeMacros/ModeHistogram.o RootMacros/ModeMacros/ModeCollection.o
#ANALYSIS_LIBS = RootMacros/Analysis/HistogramAnalysis.o RootMacros/Analysis/HistogramAnalysisUser.o

MYINCLUDES = -I.

#all: $(UTILITIES_LIBS) $(MODEMACROS_LIBS) $(ANALYSIS_LIBS)

#%: %.cc $(UTILITIES_LIBS) $(MODEMACROS_LIBS) $(ANALYSIS_LIBS)

%: %.cc
	g++  $(ROOTFLAGS) $(MYINCLUDES) -c -o $*.o $*.cc
	g++  $(ROOTFLAGS) $(MYINCLUDES) $(ROOTLIBS) $*.o -o $*


clean:
	@rm -f *.o $* */*.o
