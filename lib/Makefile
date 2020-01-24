# Invoke this Makefile from the top level Makefile.
# Downloads required project dependencies.

LIBDIR = ../lib

all: libs

libs: $(LIBDIR)/zfec $(LIBDIR)/cm256

$(LIBDIR)/zfec:
	git clone git@github.com:tahoe-lafs/zfec.git $@

$(LIBDIR)/cm256:
	git clone git@github.com:catid/cm256.git $@

# TODO: add spdlog download
