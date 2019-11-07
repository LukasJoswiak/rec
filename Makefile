CC = clang
CXX = clang++

CFLAGS = -Wall -g -std=c11
CXXFLAGS = -Wall -g -MMD -std=c++17 -I include/ -I build/gen
LIBS = -lprotobuf

SRCDIR = src
LIBDIR = lib
BUILDDIR = build
GENDIR = $(BUILDDIR)/gen
BINDIR = bin

CLIENT_SRC = $(wildcard $(SRCDIR)/client/*.cpp)
CLIENT_OBJ = $(CLIENT_SRC:$(SRCDIR)/client/%.cpp=$(BUILDDIR)/%.o)

# Get a list of object files related to the replica.
REPLICA_SRC = $(wildcard $(SRCDIR)/server/*.cpp)
REPLICA_OBJ = $(REPLICA_SRC:$(SRCDIR)/server/%.cpp=$(BUILDDIR)/%.o)

PROTO_SRC = $(wildcard $(SRCDIR)/proto/*.proto)
PROTO_GEN = $(PROTO_SRC:$(SRCDIR)/proto/%.proto=$(GENDIR)/proto/%.pb.cc)
PROTO_OBJ = $(PROTO_SRC:$(SRCDIR)/proto/%.proto=$(BUILDDIR)/%.o)

# Default to building the benchmark executable (for now).
all: dirs client replica

client: dirs $(BINDIR)/client

replica: dirs $(BINDIR)/replica

# Client build rules.
$(BINDIR)/client: $(CLIENT_OBJ)
	$(CXX) $^ -o $@

-include $(CLIENT_OBJ:.o=.d)

$(BUILDDIR)/%.o: $(SRCDIR)/client/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Replica build rules.
$(BINDIR)/replica: $(PROTO_OBJ) $(REPLICA_OBJ)
	$(CXX) $^ -o $@ $(LIBS)

$(PROTO_GEN): $(PROTO_SRC)
	protoc --proto_path=src/ --cpp_out=$(GENDIR) $^

$(PROTO_OBJ): $(PROTO_GEN)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Rebuild if any depedencies have changed (including header files).
-include $(REPLICA_OBJ:.o=.d)

$(BUILDDIR)/%.o: $(SRCDIR)/server/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Build the benchmark suite.
.PHONY: benchmark
benchmark: | libs dirs
	$(MAKE) -C benchmark/

# Download required dependencies.
libs:
	$(MAKE) -C $(LIBDIR)

# Create directories for object files and executables.
dirs: $(BUILDDIR) $(BINDIR)

$(BUILDDIR) $(GENDIR) $(BINDIR):
	mkdir -p $@ $(BUILDDIR) $(GENDIR)

.PHONY: cpplint
cpplint: ctags
	cpplint --filter=-runtime/references $(shell find . -name \*.h?? -or -name \*.cpp | grep -vE "^\.\/$(LIBDIR)\/")

.PHONY: ctags
ctags:
	ctags -R --exclude=$(LIBDIR) .

.PHONY: clean
clean:
	rm -rf $(BUILDDIR) $(BINDIR)
