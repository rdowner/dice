
TOPDIR=..
include $(TOPDIR)/common.mk

LBMAKE=$(DESTDIR)$(PREFIX)/bin/lbmake
DLIB=$(DESTDIR)$(PREFIX)/dlib
CORE_LIBS=$(BUILDDIR)/c.o $(BUILDDIR)/c_pi.o $(BUILDDIR)/c_pr.o $(BUILDDIR)/x.o $(BUILDDIR)/autos.lib $(BUILDDIR)/amiga20l.lib $(BUILDDIR)/amiga20s.lib $(BUILDDIR)/cs.lib

export DCCOPTS = -2.0 -//
export PATH := $(DESTDIR)$(PREFIX)/bin:$(PATH)

all: core-build

install: core-install

setup:
	sed -E -e 's_^(@DefTree\s+).*$$_\1$(BUILDDIR)/_' -e 's_(\s)../dlib/_\1$(BUILDDIR)/_' ulib.def > ulib.def.new
	diff -q ulib.def ulib.def.new || mv -f ulib.def.new ulib.def

core-build: setup $(CORE_LIBS)

$(BUILDDIR)/autos.lib: ulib.def
	$(LBMAKE) auto s

#   startup
$(BUILDDIR)/c.o $(BUILDDIR)/c_pi.o $(BUILDDIR)/c_pr.o $(BUILDDIR)/x.o : ulib.def
	$(LBMAKE) startup

#   C[model].lib
$(BUILDDIR)/cs.lib : ulib.def
	$(LBMAKE) clib s

#   AMIGA20[model].lib
ifdef BUILD_A20FUNC20
#   The amiga20l.lib from commodore has some incorrect tags, this stuff
#   here fixes that.  dlib.orig/amiga20l.lib contains the actual, original
#   Commodore library.  The amiga20l.lib provided with DICE is exactly
#   that plus bug fixes to the Commodore library.
#
# ...only, this build doesn't work - lbmake seems to do nothing. Leaving this out for now; FIXME later.
$(BUILDDIR)/amiga20l.lib : $(BUILDDIR)/amiga20_func_l.lib $(HOME)/amiga-ndks/NDK_2.0/NDK2.0-2/Development/SupportFiles/lib/amiga.lib
	cat $^ > $@
$(BUILDDIR)/amiga20_func_l.lib :
	$(LBMAKE) a20func20 l
else
$(BUILDDIR)/amiga20l.lib : $(HOME)/amiga-ndks/NDK_2.0/NDK2.0-2/Development/SupportFiles/lib/amiga.lib
	cp $< $@
endif
$(BUILDDIR)/amiga20s.lib : $(BUILDDIR)/amiga20l.lib
	$(DESTDIR)$(PREFIX)/bin/libtos $< $@

core-install : $(CORE_LIBS)
	mkdir -p $(DLIB)
	install $^ $(DLIB)

#	$(LBMAKE) startup
#	cp $(TOPDIR)/dlib/amiga20l.lib $(DLIB)/amiga20l.lib
#	$(DESTDIR)$(PREFIX)/bin/libtos $(DLIB)/amiga20l.lib $(DLIB)/amiga20s.lib
#	$(LBMAKE) 
