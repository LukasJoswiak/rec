CC = clang
CXX = clang++

CFLAGS = -Wall -g -std=c11
CXXFLAGS = -Wall -g -MMD -march=native -std=c++17 -I include/ -I build/gen -I lib/
LIBS = -pthread -lprotobuf

SRCDIR = src
LIBDIR = lib
BUILDDIR = build
GENDIR = $(BUILDDIR)/gen
BINDIR = bin

CLIENT_SRC = $(wildcard $(SRCDIR)/client/*.cpp)
CLIENT_OBJ = $(CLIENT_SRC:$(SRCDIR)/client/%.cpp=$(BUILDDIR)/%.o)

# Get a list of object files related to the replica.
REC_SRC = $(wildcard $(SRCDIR)/server/*.cpp)
REC_OBJ = $(REC_SRC:$(SRCDIR)/server/%.cpp=$(BUILDDIR)/%.o)

KVSTORE_SRC = $(wildcard $(SRCDIR)/kvstore/*.cpp)
KVSTORE_OBJ = $(KVSTORE_SRC:$(SRCDIR)/kvstore/%.cpp=$(BUILDDIR)/%.o)

PROCESS_SRC = $(wildcard $(SRCDIR)/process/*.cpp)
PROCESS_OBJ = $(PROCESS_SRC:$(SRCDIR)/process/%.cpp=$(BUILDDIR)/%.o)

PAXOS_SRC = $(wildcard $(SRCDIR)/process/paxos/*.cpp)
PAXOS_OBJ = $(PAXOS_SRC:$(SRCDIR)/process/paxos/%.cpp=$(BUILDDIR)/%.o)

PROTO_SRC = $(wildcard $(SRCDIR)/proto/*.proto)
PROTO_GEN = $(PROTO_SRC:$(SRCDIR)/proto/%.proto=$(GENDIR)/proto/%.pb.cc)
PROTO_OBJ = $(PROTO_SRC:$(SRCDIR)/proto/%.proto=$(BUILDDIR)/%.o)

# Default to building the benchmark executable (for now).
all: dirs client rec

client: dirs $(BINDIR)/client

rec: dirs $(BINDIR)/rec

# Client build rules.
$(BINDIR)/client: $(PROTO_OBJ) $(CLIENT_OBJ)
	$(CXX) $^ -o $@ $(LIBS)

-include $(CLIENT_OBJ:.o=.d)
-include $(PAXOS_OBJ:.o=.d)

$(BUILDDIR)/%.o: $(SRCDIR)/client/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Replica (rec) build rules.
$(BINDIR)/rec: $(PROTO_OBJ) $(PROCESS_OBJ) $(PAXOS_OBJ) $(KVSTORE_OBJ) $(REC_OBJ) $(BUILDDIR)/cm256.o $(BUILDDIR)/gf256.o
	$(CXX) $^ -o $@ $(LIBS)

# Rebuild if any depedencies have changed (including header files).
-include $(REC_OBJ:.o=.d)

$(BUILDDIR)/%.o: $(SRCDIR)/server/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Key-value store build rules.
$(BUILDDIR)/%.o: $(SRCDIR)/kvstore/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Process build rules.
$(BUILDDIR)/%.o: $(SRCDIR)/process/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Paxos build rules.
$(BUILDDIR)/%.o: $(SRCDIR)/process/paxos/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Protobuf build rules.
$(PROTO_GEN): $(PROTO_SRC)
	protoc --proto_path=src/ --cpp_out=$(GENDIR) $^

$(BUILDDIR)/%.o: $(GENDIR)/proto/%.pb.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Build the benchmark suite.
.PHONY: benchmark
benchmark: | dirs
	$(MAKE) -C benchmark/erasure/

# Create directories for object files and executables.
dirs: $(BUILDDIR) $(BINDIR)

$(BUILDDIR) $(GENDIR) $(BINDIR):
	mkdir -p $@ $(BUILDDIR) $(GENDIR)

# Library build rules.

$(BUILDDIR)/cm256.o: $(LIBDIR)/cm256/cm256.cpp
	$(CC) $(CXXFLAGS) -fPIC -I $(LIBDIR)/cm256/ -c -o $@ $<

$(BUILDDIR)/gf256.o: $(LIBDIR)/cm256/gf256.cpp
	$(CC) $(CXXFLAGS) -fPIC -I $(LIBDIR)/cm256/ -c -o $@ $<

.PHONY: cpplint
cpplint: ctags
	cpplint --filter=-runtime/references $(shell find . -name \*.h?? -or -name \*.cpp | grep -vE "^\.\/$(LIBDIR)\/")

.PHONY: ctags
ctags:
	ctags -R --exclude=$(LIBDIR) .

.PHONY: clean
clean:
	rm -rf $(BUILDDIR) $(BINDIR)
