TMPDIR ?= /tmp
DTMP := $(TMPDIR)/dtmp/obj1/
export PATH := ../bin1-bootstrap:$(PATH)
export DLIB := $(realpath ../dlib1)/
DINCLUDE := ../include/
export LIBDEF = ulib.def
export DCCOPTS = -2.0 -//

lib-core: prep $(LIBDEF)
	dxmake -f DMakefile core

prep:
	mkdir -p $(DTMP) $(DLIB)

$(LIBDEF): lib.def
	sed -E \
		-e 's_^(@DefTree\s+).*$$_\1$(DTMP)_' \
		-e 's_^(@AltTree\s+[^\s]+\s+).*$$_\1$(DTMP)_' \
		-e 's_dlib:_$(DLIB)_' \
		-e 's_dinclude:_$(DINCLUDE)_' \
		$^ > $@
