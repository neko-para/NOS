BUILD:=../build
CWD:=system
ROOT:=..

CBUILD=$(BUILD)/$(CWD)

CXXSRC=$(wildcard *.cpp)
CXXOBJ=$(patsubst %.cpp,$(CBUILD)/%.o,$(CXXSRC))
PROGRAMS=$(patsubst %.cpp,$(CBUILD)/%,$(CXXSRC))

LIBRARY=$(CBUILD)/libnos.a
LIBOBJ=$(CBUILD)/nos.o

all: $(LIBRARY) $(PROGRAMS)

include ../Makefile.cfg

$(CBUILD)/%: $(CBUILD)/%.o $(LIBRARY)
	@echo "Linking $@"
	@mkdir -p `dirname $@`
	@$(LD) -o $@ $< $(LDFLAGS) -L$(CBUILD) -lnos

$(CBUILD)/%.o: %.cpp ../kernel/export/nos.h
	@echo "Building $@"
	@mkdir -p `dirname $@`
	@$(CXX) -c -o $@ $< $(CXXFLAGS) -I../kernel/export

$(LIBOBJ): ../kernel/export/nos.cpp ../kernel/export/nos.h
	@echo "Building $@"
	@mkdir -p `dirname $@`
	@$(CXX) -c -o $@ $< -ffreestanding

$(LIBRARY): $(LIBOBJ)
	@echo "Archiving $@"
	@mkdir -p `dirname $@`
	@$(AR) r $@ $^ 2> /dev/null

clean:
	@echo "Cleaning $(LIBRARY) $(PROGRAMS) $(CXXOBJ) $(LIBOBJ)"
	@-rm -f $(LIBRARY) $(PROGRAMS) $(CXXOBJ) $(LIBOBJ)

.SUFFIXES:

.PHONY: all clean