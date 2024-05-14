
TOPDIR = ..
include $(TOPDIR)/common.mk

INCLUDEDIR=$(DESTDIR)$(PREFIX)/include

AMIGA20_HEADERS=$(HOME)/amiga-ndks/NDK_2.0/NDK2.0-4/include

all :

clean :

install :
	rsync -av --include='*/' --include='*.h' --exclude='*' ./ $(INCLUDEDIR)
	rsync -av --include='*/' --include='*.h' --exclude='*' $(AMIGA20_HEADERS)/ $(INCLUDEDIR)/amiga20

uninstall :
	rm $(INSTDIR)config/dice.errors
