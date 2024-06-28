
TOPDIR=..
include $(TOPDIR)/common.mk

AMIGA20_LIB=$(HOME)/amiga-ndks/NDK_2.0/NDK2.0-2/Development/SupportFiles/lib/amiga.lib
DLIB=$(DESTDIR)$(PREFIX)/dlib

export DCCOPTS = -2.0 -//
export PATH := $(DESTDIR)$(PREFIX)/bin:$(PATH)

ulib.def: lib.def
	sed -E \
		-e 's_^(@DefTree\s+).*$$_\1$(BUILDDIR)/_' \
		-e 's_^(@AltTree\s+[^\s]+\s+).*$$_\1$(BUILDDIR)/_' \
		-e 's_dlib:_$(DLIB)/_' \
		-e 's_dinclude:_$(DESTDIR)$(abspath $(PREFIX))/include/_' \
		$^ > $@

all: prep ulib.def core amiga20

prep:
	mkdir -p $(DLIB)

core:
	LIBDEF=$< DLIB=$(DLIB)/ dxmake -f DMakefile.amiga all

amiga20: $(DLIB)/dlib.orig/amiga20l.lib
	LIBDEF=$< DLIB=$(DLIB)/ dxmake -f DMakefile.amiga amiga20
$(DLIB)/dlib.orig/amiga20l.lib: $(AMIGA20_LIB)
	mkdir $(dir $@)
	cp $^ $@
