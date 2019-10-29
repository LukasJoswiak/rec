CC = clang
CXX = clang++

CFLAGS = -Wall -g -std=c11
CXXFLAGS = -Wall -g -std=c++11

SRCDIR = src
OBJDIR = obj
BINDIR = bin

# Default to building the benchmark executable (for now).
all: dirs client

client: dirs $(BINDIR)/client

$(BINDIR)/client: $(OBJDIR)/client.o
	$(CXX) $^ -o $@

$(OBJDIR)/client.o: $(SRCDIR)/client.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Build the benchmark suite.
.PHONY: benchmark
benchmark: | libs dirs
	$(MAKE) -C benchmark/

# Download required dependencies.
libs:
	$(MAKE) -C lib/

# Create directories for object files and executables.
dirs: $(OBJDIR) $(BINDIR)

$(OBJDIR) $(BINDIR):
	mkdir -p $@

.PHONY: clean
clean:
	rm -rf $(OBJDIR) $(BINDIR)
