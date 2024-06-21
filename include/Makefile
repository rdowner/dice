
TOPDIR = ..
include $(TOPDIR)/common.mk

INCLUDEDIR=$(DESTDIR)$(PREFIX)/include

AMIGA20_HEADERS=$(HOME)/amiga-ndks/NDK_2.0/NDK2.0-4/include
AMIGA20_FD=$(HOME)/amiga-ndks/NDK_2.0/NDK2.0-2/Development/SupportFiles/fd

all :

clean :

install :
	rsync -av --include='*/' --include='*.h' --exclude='*' ./ $(INCLUDEDIR)
	rsync -av --include='*/' --include='*.h' --exclude='*' $(AMIGA20_HEADERS)/ $(INCLUDEDIR)/amiga20
	rsync -av --include='*/' --include='*.fd' --exclude='*' $(AMIGA20_FD)/ $(INCLUDEDIR)/amiga20/fd
	mkdir -p $(INCLUDEDIR)/fd

uninstall :
	rm $(INSTDIR)config/dice.errors
