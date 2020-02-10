# Invoke this Makefile from the top level Makefile.

CC = clang
CXX = clang++

CFLAGS = -Wall -g -std=c11
CXXFLAGS = -Wall -g -march=native -std=c++17

SRCDIR = ../src
LIBDIR = ../lib
BUILDDIR = ../build
BINDIR = ../bin


all: $(BINDIR)/benchmark

$(BINDIR)/benchmark: $(BUILDDIR)/libecwrapper.a $(BUILDDIR)/cm256.o $(BUILDDIR)/gf256.o $(BUILDDIR)/benchmark.o
	# Ignore first dependency when linking object files.
	$(CXX) $(filter-out $<,$^) -o $@ -L$(BUILDDIR) -lecwrapper

$(BUILDDIR)/benchmark.o: ../benchmark/benchmark.cpp
	$(CXX) $(CXXFLAGS) -I $(SRCDIR) -c -o $@ $<

# Generate the library archive from the object file.
$(BUILDDIR)/libecwrapper.a: $(BUILDDIR)/fec.o $(BUILDDIR)/ec_wrapper.o
	ar rcs $@ $^

$(BUILDDIR)/fec.o: $(LIBDIR)/zfec/zfec/fec.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILDDIR)/ec_wrapper.o: $(SRCDIR)/ec_wrapper.c $(SRCDIR)/ec_wrapper.h
	$(CC) $(CFLAGS) -I $(LIBDIR)/zfec/zfec/ -c -o $@ $<

$(BUILDDIR)/cm256.o: $(LIBDIR)/cm256/cm256.cpp
	$(CC) $(CXXFLAGS) -I $(LIBDIR)/cm256/ -c -o $@ $<

$(BUILDDIR)/gf256.o: $(LIBDIR)/cm256/gf256.cpp
	$(CC) $(CXXFLAGS) -I $(LIBDIR)/cm256/ -c -o $@ $<
