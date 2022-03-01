BUILD:=../build
CWD:=system
ROOT:=..

CBUILD=$(BUILD)/$(CWD)
CDEP=$(BUILD)/.dep/$(CWD)

CXXSRC=$(wildcard *.cpp)
CXXOBJ=$(patsubst %.cpp,$(CBUILD)/%.cpp.o,$(CXXSRC))
CXXDEP=$(patsubst %.cpp,$(CDEP)/%.cpp.d,$(CXXSRC))
PROGRAMS=$(patsubst %.cpp,$(CBUILD)/%,$(CXXSRC))

LIBRARY=$(CBUILD)/libnos.a
CRT0=$(CBUILD)/crt0.c.o

all: Lib.check $(LIBRARY) $(CXXDEP) $(PROGRAMS)

-include $(CXXDEP) $(ROOT)/Makefile.cfg

$(CBUILD)/%: $(CBUILD)/%.cpp.o $(LIBRARY) $(CRT0)
	@echo "Linking $@"
	@mkdir -p `dirname $@`
	@$(LD) -o $@ $(CRT0) $< $(LDFLAGS) -L$(CBUILD) -lnos

$(CDEP)/%.cpp.d: %.cpp
	@echo "Generating dependency of $<"
	@mkdir -p `dirname $@`
	@$(CXX) -M -MF $@ -MT $(CBUILD)/$<.o $< $(CXXFLAGS) -I../kernel/nos
	@echo '	@echo "Building $$@"' >> $@
	@echo '	@mkdir -p `dirname $$@`' >> $@
	@echo '	@$$(CXX) -c -o $$@ $$< $$(CXXFLAGS) -fomit-frame-pointer -I../kernel/nos' >> $@

$(CRT0): crt0.c
	@echo "Building $@"
	@mkdir -p `dirname $@`
	@$(CC) -c -o $@ $< $(CFLAGS) -I../kernel/nos

Lib.check: 
	@make -C $(ROOT)/kernel/nos all BUILD=../$(CBUILD) CWD=nos ROOT=../..

Lib.clean:
	@make -C $(ROOT)/kernel/nos clean BUILD=../$(CBUILD) CWD=nos ROOT=../..

clean: Lib.clean
	@echo "Cleaning $(LIBRARY) $(PROGRAMS) $(CXXOBJ) $(CXXDEP)"
	@-rm -f $(LIBRARY) $(PROGRAMS) $(CXXOBJ) $(CXXDEP)

.SUFFIXES:

.PHONY: all clean Lib.check Lib.clean