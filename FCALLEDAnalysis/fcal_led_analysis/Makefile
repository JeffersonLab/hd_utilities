CXX?=g++
VERS:=0.1

OBJ:=obj/
SRC:=src/
DEP:=deps/

WARNINGS:=all extra no-unused-parameter no-format-nonliteral \
					strict-overflow=3 write-strings switch-default \
					switch-enum unreachable-code format=2 no-deprecated-copy
FSWITCH:= diagnostics-color=always
DSWITCH:=
ISWITCH:=

DEBUG?=no

ifeq ($(DEBUG), yes)
	WARNINGS+=error
	CXXFLAGS+=-ggdb -Og
	VERSION:=$(NAME)-$(VERS)-debug
	FSWITCH+=trapv
else
	CXXFLAGS+=-O2
	VERSION:=$(NAME)-$(VERS)
	FSWITCH+=fast-math
endif

PINFO:=./pprint.sh info
PERR:=./pprint.sh err
PLOG:=./pprint.sh log
PSUCC:=./pprint.sh succ

CXXFLAGS+=$(shell root-config --cflags) -pedantic
LDFLAGS+=$(shell root-config --ldflags --libs) -Wl,-rpath,$(shell root-config --libdir)

WFLAGS:=$(addprefix -W,$(WARNINGS))
FFLAGS:=$(addprefix -f,$(FSWITCH))
DFLAGS:=$(addprefix -D,$(DSWITCH)) \
			 -Dcompiletime='$(shell date +"%Y-%m-%d %H:%M %z")' \
			 -DVERSION='$(VERSION)'
IFLAGS:=$(addprefix -I,$(ISWITCH))

COMPFLAGS:=-c $(IFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(WFLAGS) $(FFLAGS) $(DFLAGS)
TESTSTD=$(shell root-config --cflags | xargs printf "%s\n" | grep "std" | sed "s/c++/gnu++/")
TESTCOMPFLAGS:=-c $(IFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(DFLAGS) -I$(TEST) $(TESTSTD) $(FFLAGS)

SRCS:=$(wildcard $(SRC)*.cpp)
EXES:=$(wildcard $(SRC)*.cc)
OBJS:=$(patsubst $(SRC)%.cpp,$(OBJ)%.o,$(SRCS))
EOBJS:=$(patsubst $(SRC)%.cc,$(OBJ)%.o,$(EXES))
NAMES:=$(patsubst $(SRC)%.cc,%,$(EXES))
DEPS:=$(patsubst $(SRC)%.cpp,$(DEP)%.dep,$(SRCS)) $(patsubst $(SRC)%.cc,$(DEP)%.dep,$(EXES)) \
			$(patsubst $(TEST)$(TESTEXE)%,$(DEP)%.dep,$(TESTS)) $(DEP)$(TESTMAIN).dep

TESTS:=$(shell find $(TEST) -mindepth 1 -type d)
TESTDEPS:=$(shell find $(TEST) -type f -name "*.cpp")
TESTDEPS:=$(patsubst $(TEST)%.cpp,$(DEP)$(TEST)%.dep,$(TESTDEPS))
TESTEXEDEPS:=$(patsubst $(TEST)%,$(DEP)$(TESTEXE)%.dep,$(TESTS))
TESTDEPS+= $(TESTEXEDEPS)
TESTNAMES:=$(patsubst $(TEST)%,$(TESTEXE)%,$(TESTS))

ERRPIPE= >> $@.log 2>&1
FAILCOND=! [ -e $@ ] || ! [ -s $@ ] || [ -s $@.log ]

DEPFLG:=--posix -e
DEPCMD=s@^\($*\)\.o:@$(DEP)\1.dep $(OBJ)\1.o:@
TESTMAINDEPCMD:=s@^main.o:@$(DEP)$(TESTMAIN).dep $(OBJ)$(TEST)$(TESTMAIN).o:@
TESTDEPCMD=s@^\([^\.]\+\).o:@$(DEP)$(TEST)\1.dep \
	$(TESTEXE)$(patsubst $(OBJ)$(TEST)%/,%,$(dir $@)) $@@

all: $(OBJS) $(NAMES)
	@if [ -s errs.log ] ; then cat errs.log | less -R; rm errs.log; fi
	@if [ -e errs.log ] ; then rm errs.log; fi

%: $(OBJS) $(OBJ)%.o
	@touch errs.log
	@-$(CXX) $^ -o $@ $(LDFLAGS) $(ERRPIPE)
	@if $(FAILCOND) ; then \
		$(PERR) Failed to link program $@! ; else $(PSUCC) Linked Program $@! ; fi
	@flock errs.log cat $@.log >> errs.log
	@rm $@.log

$(OBJ)%.o: $(SRC)%.cc
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(DEP)$*.dep)
	@touch errs.log
	@-$(CXX) $(COMPFLAGS) -o $@ $< $(ERRPIPE)
	@-$(CXX) $(COMPFLAGS) -MM -MF $(DEP)$*.dep $< 2>/dev/null >/dev/null
	@-sed $(DEPFLG) "$(DEPCMD)" $(DEP)$*.dep > $(DEP)$*.tmpdep
	@rm -f $(DEP)$*.dep
	@-mv $(DEP)$*.tmpdep $(DEP)$*.dep
	@if $(FAILCOND) ; then \
		$(PERR) Failed to create $@! ;\
		else $(PSUCC) Created $@! ; $(PSUCC) Created $(DEP)$*.dep! ; fi
	@flock errs.log cat $@.log >> errs.log
	@rm $@.log

$(OBJ)%.o: $(SRC)%.cpp
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(DEP)$*.dep)
	@touch errs.log
	@-$(CXX) $(COMPFLAGS) -o $@ $< $(ERRPIPE)
	@-$(CXX) $(COMPFLAGS) -MM -MF $(DEP)$*.dep $< 2>/dev/null >/dev/null
	@sed $(DEPFLG) "$(DEPCMD)" $(DEP)$*.dep > $(DEP)$*.tmpdep
	@rm -f $(DEP)$*.dep
	@mv $(DEP)$*.tmpdep $(DEP)$*.dep
	@if $(FAILCOND) ; then \
		$(PERR) Failed to create $@! ;\
		else $(PSUCC) Created $@! ; $(PSUCC) Created $(DEP)$*.dep! ; fi
	@flock errs.log cat $@.log >> errs.log
	@rm $@.log

clean: cleanobj cleandep
	@rm -f errs.log
	@$(PSUCC) Cleaned!

cleanobj:
	@$(PINFO) Removing objects...
	@rm -rf $(OBJ)

cleandep:
	@$(PINFO) Removing dependencies...
	@rm -rf $(DEP)

cleanexe:
	@$(PINFO) Removing executables...
	@rm -f $(NAMES)
	@rm -rf $(TESTEXE)

cleanso:
	@$(PINFO) Removing $(SONAME)...
	@rm -f $(SONAME)

cleandist: cleanobj cleandep cleanexe cleanso
	@rm -f errs.log
	@$(PSUCC) Cleaned!

cleandata: cleanobj cleandep cleanexe cleanso
	@rm -f errs.log
	@$(PINFO) Removing data...
	@rm -rf data/
	@$(PSUCC) Cleaned!

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
-include $(DEPS)
endif

ifeq (test,$(findstring test,$(MAKECMDGOALS)))
-include $(TESTDEPS)
endif

FORCE:

.DEFAULT_GOAL:=all
.PHONY: all clean cleanexe cleandep cleanobj \
		cleandist cleandata cleanso
.PRECIOUS: $(EOBJS) $(OBJ)$(TEST)%.o
