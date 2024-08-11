
default :
	@echo "\n  Please read BUILDING.md to find out how to buid DICE. It's not straightforward!\n"
	@false

BIN=$(abspath bin1-bootstrap)

ENV_BOOTSTRAP = PATH="$(BIN):$(PATH)" OD="/tmp/dtmp/obj1-bootstrap/" INSTDIR="$(abspath .)/" BIN="$(BIN)/" CROSS_COMPILE=1 DLIB="$(abspath dlib1)/"
ENV = PATH="$(BIN):$(PATH)" OD="/tmp/dtmp/obj1/" INSTDIR="$(abspath .)/" BIN="$(abspath bin1)/" DCCOPTS="-2.0 -//" DLIB="$(abspath dlib1)/"

stage1 : stage1a stage1b stage1c
stage1a : $(BIN)/dxmake
	( cd suplib && $(ENV_BOOTSTRAP) dxmake all install )
	$(ENV_BOOTSTRAP) dxmake bin-core install-bin-core
stage1b :
	mkdir -p dlib1
	[ -d include/amiga20 ] || cp -a $(AMIGA_NDK_HOME)/2.0/include_h include/amiga20
	[ -d include/amiga20/fd ] || cp -a $(AMIGA_NDK_HOME)/2.0/fd include/amiga20/fd
	( cd lib && PATH="$(BIN):$(PATH)" OD="/tmp/dtmp/obj1/" INSTDIR="$(abspath .)/" BIN="$(BIN)/" DCCOPTS="-2.0 -//" DLIB="$(abspath dlib1)/" dxmake all )
stage1c :
	mkdir -p bin1
	$(ENV) dxmake bin-core install-bin-core

.PHONY : stage1 stage1a stage1b stage1c

$(BIN)/dxmake :
	mkdir -p $(BIN)
	$(MAKE) -C src/dxmake EXED="$(BIN)" all install

clean :
	$(MAKE) -C src/dxmake SUBDIR=src/dxmake clean
	( cd suplib && $(ENV_BOOTSTRAP) dxmake clean )
	rm -rf dlib1 bin1 bin1-bootstrap /tmp/dtmp
.PHONY : clean
