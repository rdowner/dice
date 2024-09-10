
default :
	@echo "\n  Please read BUILDING.md to find out how to buid DICE. It's not straightforward!\n"
	@false

BIN=$(abspath bin1-bootstrap)
DTMP=/tmp/dtmp
DINCLUDE=$(abspath include)

ENV_COMMON = PATH="$(BIN):$(PATH)" INSTDIR="$(abspath .)/" DLIB="$(abspath dlib1)/"

ENV1A = $(ENV_COMMON) OD="$(DTMP)/obj1-bootstrap/" BIN="$(BIN)/"          CROSS_COMPILE=1
ENV1B = $(ENV_COMMON) OD="$(DTMP)/obj1/"           BIN="$(BIN)/"          DCCOPTS="-2.0 -//" DTMP=$(DTMP)/ DINCLUDE=$(DINCLUDE)/
ENV1C = $(ENV_COMMON) OD="$(DTMP)/obj1/"           BIN="$(abspath bin1)/" DCCOPTS="-2.0 -//"

stage1 : stage1a stage1b stage1c
stage1a : $(BIN)/dxmake
	( cd suplib && $(ENV1A) dxmake all install )
	$(ENV1A) dxmake bin-core install-bin-core
stage1b :
	mkdir -p dlib1
	[ -d include/amiga20 ] || cp -a $(AMIGA_NDK_HOME)/2.0/include_h include/amiga20
	[ -d include/amiga20/fd ] || cp -a $(AMIGA_NDK_HOME)/2.0/fd include/amiga20/fd
	( cd lib && $(ENV1B) dxmake all )
stage1c :
	mkdir -p bin1 $(DTMP)/obj1
	$(ENV1C) dxmake bin-core install-bin-core

.PHONY : stage1 stage1a stage1b stage1c

$(BIN)/dxmake :
	mkdir -p $(BIN)
	$(MAKE) -C src/dxmake EXED="$(BIN)" all install

clean :
	$(MAKE) -C src/dxmake SUBDIR=src/dxmake clean
	( cd suplib && $(ENV1A) dxmake clean )
	( cd lib && $(ENV1B) dxmake clean )
	rm -rf dlib1 bin1 bin1-bootstrap /tmp/dtmp include/amiga20
.PHONY : clean
