LD=i686-elf-ld
AR=i686-elf-ar
CXX=i686-elf-g++

LIBRARY=libnos.a
TARGET=test

all: $(LIBRARY) $(TARGET)

install: all
	cp $(TARGET) ../disk/bin

test: test.o $(LIBRARY)
	$(LD) -o $@ $< -L . -lnos

test.o: test.cpp ../kernel/export/syscall.h
	$(CXX) -c -o $@ $< -ffreestanding

syscall.o: ../kernel/export/syscall.cpp ../kernel/export/syscall.h
	$(CXX) -c -o $@ $< -ffreestanding

libnos.a: syscall.o
	$(AR) r $@ $^

clean:
	-rm -f $(LIBRARY) $(TARGET) $(patsubst %,%.o,$(TARGET))

.SUFFIXES:
.PHONY: all install clean