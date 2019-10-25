OBJDIR = obj
BINDIR = bin

# Default to building the benchmark executable (for now).
all: dirs benchmark

# Build the benchmark suite.
.PHONY: benchmark
benchmark: | dirs
	$(MAKE) -C benchmark/

# Create directories for object files and executables.
dirs: $(OBJDIR) $(BINDIR)

$(OBJDIR) $(BINDIR):
	mkdir -p $@

.PHONY: clean
clean:
	rm -rf $(OBJDIR) $(BINDIR)
