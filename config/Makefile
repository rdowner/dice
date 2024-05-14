
TOPDIR = ..
include $(TOPDIR)/common.mk

INSTDIR=$(realpath $(DESTDIR)$(PREFIX))/

all :

clean :

install : $(INSTDIR)config
	cp dice.errors $(INSTDIR)config/dice.errors

uninstall :
	rm $(INSTDIR)config/dice.errors

$(INSTDIR)config : $(INSTDIR)config
	mkdir $@
