CC = clang
CXX = clang++

CFLAGS = -Wall -g -std=c11
CXXFLAGS = -Wall -g -std=c++11

SRCDIR = src
LIBDIR = lib
OBJDIR = obj
BINDIR = bin


all: dirs $(BINDIR)/benchmark

$(BINDIR)/benchmark: $(OBJDIR)/libecwrapper.a $(OBJDIR)/cm256.o $(OBJDIR)/gf256.o $(OBJDIR)/benchmark.o
	# Ignore first dependency when linking object files.
	$(CXX) $(filter-out $<,$^) -o $@ -L$(OBJDIR) -lecwrapper

$(OBJDIR)/benchmark.o: benchmark/benchmark.cc
	$(CXX) $(CXXFLAGS) -I src/ -c -o $@ $<

# Generate the library archive from the object file.
$(OBJDIR)/libecwrapper.a: $(OBJDIR)/fec.o $(OBJDIR)/ec_wrapper.o
	ar rcs $@ $^

$(OBJDIR)/fec.o: $(LIBDIR)/zfec/zfec/fec.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/ec_wrapper.o: $(SRCDIR)/ec_wrapper.c $(SRCDIR)/ec_wrapper.h
	$(CC) $(CFLAGS) -I $(LIBDIR)/zfec/zfec/ -c -o $@ $<

$(OBJDIR)/cm256.o: $(LIBDIR)/cm256/cm256.cpp
	$(CC) $(CXXFLAGS) -I $(LIBDIR)/cm256/ -c -o $@ $<

$(OBJDIR)/gf256.o: $(LIBDIR)/cm256/gf256.cpp
	$(CC) $(CXXFLAGS) -I $(LIBDIR)/cm256/ -c -o $@ $<

dirs: $(OBJDIR) $(BINDIR)

$(OBJDIR) $(BINDIR):
	mkdir -p $@

.PHONY: clean
clean:
	rm -rf $(OBJDIR) $(BINDIR)
