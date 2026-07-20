.PHONY : env clean

CC = g++
C_FLAGS = -O3 -std=c++0x -Wall -fPIC


ROOTSYS = $(shell root-config --prefix)
INCLUDE = -I$(ROOTSYS)/include

ROOT_LIBS = $(shell root-config --libs) 

LIBS = $(ROOT_LIBS)

SOURCES = $(shell ls *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = stage1

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(C_FLAGS) $^ -o $@ $(LIBS)

%.o: %.cpp
	$(CC) -c $(C_FLAGS) $(INCLUDE) $<

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)

env:
	@echo CC = $(CC)
	@echo C_FLAGS = $(C_FLAGS)
	@echo MAIN_FILE = $(MAIN_FILE)
	@echo EXECUTABLE = $(EXECUTABLE)
	@echo SOURCES = $(SOURCES)
	@echo OBJECTS = $(OBJECTS)
	@echo INCLUDE = $(INCLUDE)
	@echo LIBS = $(LIBS)
