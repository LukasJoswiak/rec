# Invoke this Makefile from the top level Makefile.
# Downloads required project dependencies.

LIBDIR = ../lib

all: libs

libs: $(LIBDIR)/zfec $(LIBDIR)/cm256

$(LIBDIR)/zfec:
	git clone https://github.com/tahoe-lafs/zfec.git $@

$(LIBDIR)/cm256:
	git clone https://github.com/catid/cm256.git $@

# TODO: add spdlog download
