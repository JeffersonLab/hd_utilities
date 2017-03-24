#---------------#
# Define macros #
#---------------#
SERIES = $(eval $(1)=$(2)/$(4)) $(eval $(1)_BASE=$(3)) \
         $(eval $(1)_HDR=$(2)/$(4)/inc) $(eval $(1)_LIB=$(2)/lib/$(HOST))
SHARED = -L$(1) -Wl,-rpath,$(1)

#-------------------------------------#
# Define external component locations #
#-------------------------------------#
HOST = rhel-6-x86_64
CERT = /cs/certified/apps
$(call SERIES,CPPLIB,$(CERT)/cpplib,3.18,3.18)
$(call SERIES,DBLIB,$(CERT)/dblib,2.4,2.4)
MYALIB = $(CERT)/mya/5.6/lib
JLABLIB = $(CERT)/jlablib/4.2
EPICS = $(CERT)/epics/v3.13.10-j1
MYAPI = $(CERT)/myapi/6.4

#---------------------------------#
# Set compiler and linker options #
#---------------------------------#
INC = -I$(CPPLIB_HDR) -I$(MYAPI)/inc -I$(MYALIB) -I$(EPICS)/include \
      -I$(DBLIB_HDR) -I/usr/include/mysql -I$(JLABLIB)/inc \
      -I$(ROOTSYS)/include
LIB_PATH = $(call SHARED,$(CPPLIB_LIB)) \
           $(call SHARED,$(DBLIB_LIB)) \
           $(call SHARED,$(MYAPI)/lib/$(HOST)) \
           $(shell $(ROOTSYS)/bin/root-config --cflags --glibs)
LIB = -lmyapi -lcpp$(CPPLIB_BASE)
COPTS = -D$(subst -,_,$(HOST)) -D_REENTRANT -Wall
compile = g++

#-------------------------#
# Designate project files #
#-------------------------#
SRC = get_TOF_EPICS_scalers.cpp
OBJ = $(SRC:.cpp=.o)
HDR =
EXE = ./get_TOF_EPICS_scalers
HDR_CTL = hdr.ctl   # To force compile-all on any header change

#-------------------#
# 'make' directives #
#-------------------#
all:	$(HDR_CTL) $(EXE)
compact: clean pure

.SUFFIXES: .cpp

# Compile a .cpp file.
.cpp.o:
	@echo "Compiling" $<
	@$(compile) $(COPTS) -c $< -o $@ $(INC)

# Create the executable
$(EXE): $(OBJ) $(HDR_CTL)
	@echo "Linking" $@
	@$(compile) $(COPTS) -o $@ -Wl,-s $(OBJ) $(LIB_PATH) $(LIB)

# A header changed, so build all.
$(HDR_CTL): $(HDR)
	@rm -f $(OBJ)
	@touch $(HDR_CTL)

# Remove intermediate files.
clean:
	@rm -f $(OBJ)
	@rm -f $(HDR_CTL)

# Remove the created executable file.
pure:
	@rm -f $(EXE)

