
default :
	@echo "Please read BUILDING.md to find out how to buid DICE. It's not straightforward!"
	@false

BIN=$(abspath bin1-bootstrap)

stage1 : stage1a stage1b stage1c
stage1a :
	mkdir -p $(BIN)
	$(MAKE) -C src/dxmake EXED="$(BIN)" all install
	( cd suplib && PATH="$(BIN):$(PATH)" OD="/tmp/dtmp/obj1-bootstrap/" INSTDIR="$(abspath .)/" BIN="$(BIN)/" CROSS_COMPILE=1 DLIB="$(abspath dlib1)/" dxmake all install )
	PATH="$(BIN):$(PATH)" OD="/tmp/dtmp/obj1-bootstrap/" INSTDIR="$(abspath .)/" BIN="$(BIN)/" CROSS_COMPILE=1 DLIB="$(abspath dlib1)/" dxmake bin-core install-bin-core
stage1b :
	mkdir -p dlib1
	( cd lib && PATH="$(BIN):$(PATH)" OD="/tmp/dtmp/obj1/" INSTDIR="$(abspath .)/" BIN="$(BIN)/" DCCOPTS="-2.0 -//" DLIB="$(abspath dlib1)/" dxmake all )
stage1c :
	mkdir -p bin1
	PATH="$(BIN):$(PATH)" OD="/tmp/dtmp/obj1/" INSTDIR="$(abspath .)/" BIN="$(abspath bin1)/" DCCOPTS="-2.0 -//" DLIB="$(abspath dlib1)/" dxmake bin-core install-bin-core

.PHONY : stage1 stage1a stage1b stage1c

clean :
	$(MAKE) -C src/dxmake SUBDIR=src/dxmake clean
	( cd suplib && PATH="$(BIN):$(PATH)" OD="/tmp/dtmp/obj1-bootstrap/" INSTDIR="$(abspath .)/" BIN="$(BIN)/" CROSS_COMPILE=1 DLIB="$(abspath dlib1)/" dxmake clean )
	rm -rf dlib1 bin1 bin1-bootstrap /tmp/dtmp
.PHONY : clean
