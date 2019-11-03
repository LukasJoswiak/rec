CC = clang
CXX = clang++

CFLAGS = -Wall -g -std=c11
CXXFLAGS = -Wall -g -MMD -std=c++17 -I include/

SRCDIR = src
LIBDIR = lib
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

# Rebuild if any depedencies have changed (including header files).
-include $(REPLICA_OBJ:.o=.d)

$(OBJDIR)/%.o: $(SRCDIR)/server/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Build the benchmark suite.
.PHONY: benchmark
benchmark: | libs dirs
	$(MAKE) -C benchmark/

# Download required dependencies.
libs:
	$(MAKE) -C $(LIBDIR)

# Create directories for object files and executables.
dirs: $(OBJDIR) $(BINDIR)

$(OBJDIR) $(BINDIR):
	mkdir -p $@

.PHONY: cpplint
cpplint: ctags
	cpplint --filter=-runtime/references $(shell find . -name \*.h?? -or -name \*.cpp | grep -vE "^\.\/$(LIBDIR)\/")

.PHONY: ctags
ctags:
	ctags -R --exclude=$(LIBDIR) .

.PHONY: clean
clean:
	rm -rf $(OBJDIR) $(BINDIR)
