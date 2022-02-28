BUILD:=../build
CWD:=kernel
ROOT:=..

CBUILD=$(BUILD)/$(CWD)
CDEP=$(BUILD)/.dep/$(CWD)

DIRS=$(patsubst %/.,%,$(wildcard */.))
LIBRARYS=$(patsubst %,$(CBUILD)/lib%.a,$(DIRS))
KERNEL=$(BUILD)/kernel/kernel.bin

all: Library.all $(KERNEL)

-include $(CDEP)/kernel.cpp.d $(ROOT)/Makefile.cfg

$(KERNEL): $(CDEP)/kernel.cpp.d $(CBUILD)/kernel.cpp.o link.ld $(LIBRARYS)
	@echo "Linking $@"
	@$(LD) -T link.ld $(LDFLAGS) -o $@ $(CBUILD)/kernel.cpp.o $(LIBRARYS)

$(CDEP)/kernel.cpp.d: kernel.cpp
	@echo "Generating dependency of $<"
	@mkdir -p `dirname $@`
	@$(CXX) -MM -MF $@ -MT $(CBUILD)/$<.o $< $(CXXFLAGS)
	@echo '	@echo "Building $$@"' >> $@
	@echo '	@mkdir -p `dirname $$@`' >> $@
	@echo '	@$$(CXX) -c -o $$@ $$< $$(CXXFLAGS)' >> $@

Library.check:
	@for i in $(DIRS); do \
		if ! [ -e $$i/Makefile ]; then \
			echo "Linking $$i/Makefile"; \
			ln -sr SubMakefile $$i/Makefile; \
		fi; \
	done;

Library.all: Library.check
	@for i in $(DIRS); do \
		make -s -C $$i all BUILD=../$(BUILD) CWD=$(CWD)/$$i ROOT=../$(ROOT); \
	done;

Library.clean: Library.check
	@for i in $(DIRS); do \
		make -s -C $$i clean BUILD=../$(BUILD) CWD=$(CWD)/$$i ROOT=../$(ROOT); \
	done;

clean: Library.clean
	@echo "Cleaning $(KERNEL) $(CDEP)/kernel.cpp.d $(CBUILD)/kernel.cpp.o"
	@-rm -f $(KERNEL) $(CDEP)/kernel.cpp.d $(CBUILD)/kernel.cpp.o

.PHONY: all clean Library.all Library.clean Library.check