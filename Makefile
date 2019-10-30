CC = clang
CXX = clang++

CFLAGS = -Wall -g -std=c11
CXXFLAGS = -Wall -g -std=c++11 -I include/

SRCDIR = src
OBJDIR = obj
BINDIR = bin

# Get a list of object files related to the replica.
REPLICA_SRC = $(wildcard $(SRCDIR)/server/*.cpp)
REPLICA_OBJ = $(REPLICA_SRC:$(SRCDIR)/server/%.cpp=$(OBJDIR)/%.o)

# Default to building the benchmark executable (for now).
all: dirs client replica

client: dirs $(BINDIR)/client

replica: dirs $(BINDIR)/replica

# Client build rules.
$(BINDIR)/client: $(OBJDIR)/client.o
	$(CXX) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/client/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Replica build rules.
$(BINDIR)/replica: $(REPLICA_OBJ)
	$(CXX) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/server/%.cpp
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
