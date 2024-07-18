
default :
	@echo "Please read BUILDING.md to find out how to buid DICE. It's not straightforward!"
	@false

stage1 : stage1a stage1b stage1c
stage1a :
	$(MAKE) -C suplib SUBDIR=suplib all install
	$(MAKE) -C src/dxmake SUBDIR=src/dxmake all install
	PATH="$(abspath bin1-bootstrap):$(PATH)" OD="/tmp/dtmp/obj1-bootstrap/" INSTDIR="$(abspath .)/" BIN="$(abspath bin1-bootstrap)/" CROSS_COMPILE=1 DLIB="$(abspath dlib1)/" dxmake bin-core install-bin-core
stage1b :
	mkdir -p dlib1
	cd lib && $(MAKE) lib-core
stage1c :
	mkdir -p bin1
	PATH="$(abspath bin1-bootstrap):$(PATH)" OD="/tmp/dtmp/obj1/" INSTDIR="$(abspath .)/" BIN="$(abspath bin1)/" DCCOPTS="-2.0 -//" DLIB="$(abspath dlib1)/" dxmake bin-core install-bin-core

.PHONY : stage1 stage1a stage1b stage1c

clean :
	$(MAKE) -C suplib SUBDIR=suplib clean
	$(MAKE) -C src/dxmake SUBDIR=src/dxmake clean
	rm -rf dlib1 bin1 bin1-bootstrap /tmp/dtmp
.PHONY : clean
