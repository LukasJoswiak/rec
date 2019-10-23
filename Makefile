CC = clang
CXX = clang++

CFLAGS = -Wall -g -std=c11
CXXFLAGS = -Wall -g -std=c++11


all: dirs benchmark

benchmark: libecwrapper.a cm256.o gf256.o benchmark.o
	# Ignore first dependency when linking object files.
	$(CXX) $(filter-out $<,$^) -o bin/$@ -L. -lecwrapper

benchmark.o: benchmark/benchmark.cc
	$(CXX) $(CXXFLAGS) -I src/ -c -o benchmark.o $<

# Generate the library archive from the object file.
libecwrapper.a: fec.o ec_wrapper.o
	ar rcs $@ $^

fec.o: lib/zfec/zfec/fec.c
	$(CC) $(CFLAGS) -c -o $@ $<

ec_wrapper.o: src/ec_wrapper.c src/ec_wrapper.h
	$(CC) $(CFLAGS) -I lib/zfec/zfec/ -c -o $@ $<

cm256.o: lib/cm256/cm256.cpp
	$(CC) $(CXXFLAGS) -I lib/cm256/ -c -o $@ $<

gf256.o: lib/cm256/gf256.cpp
	$(CC) $(CXXFLAGS) -I lib/cm256/ -c -o $@ $<

dirs: bin

bin:
	mkdir -p bin/

.PHONY: clean
clean:
	rm -rf a.out benchmark.o ec_wrapper.o fec.o cm256.o gf256.o libecwrapper.a bin/
