LD=i686-elf-ld
CXX=i686-elf-g++

TARGET=test

all: $(TARGET)

install: all
	cp $(TARGET) ../disk/bin

%: %.o
	$(LD) -o $@ $<

%.o: %.cpp
	$(CXX) -c -o $@ $< -ffreestanding

clean:
	-rm -f $(TARGET) $(patsubst %,%.o,$(TARGET))

.SUFFIXES:
.PHONY: all install clean