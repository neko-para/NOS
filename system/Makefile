LD=i686-elf-ld
AR=i686-elf-ar
CXX=i686-elf-g++

LIBRARY=libnos.a
TARGET=test about

all: $(LIBRARY) $(TARGET)
	if [ -e ../install ]; then rm ../install; fi

install: all
	cp $(TARGET) ../disk/bin

test: test.o $(LIBRARY)
	$(LD) -o $@ $< -L . -lnos

test.o: test.cpp ../kernel/export/nos.h
	$(CXX) -c -o $@ $< -ffreestanding -I../kernel/export

about: about.o $(LIBRARY)
	$(LD) -o $@ $< -L . -lnos

about.o: about.cpp ../kernel/export/nos.h
	$(CXX) -c -o $@ $< -ffreestanding -I../kernel/export

nos.o: ../kernel/export/nos.cpp ../kernel/export/nos.h
	$(CXX) -c -o $@ $< -ffreestanding

libnos.a: nos.o
	$(AR) r $@ $^

clean:
	-rm -f $(LIBRARY) $(TARGET) $(patsubst %,%.o,$(TARGET))

.SUFFIXES:
.PHONY: all install clean