
default :
	@echo "Please read BUILDING.md to find out how to buid DICE. It's not straightforward!"
	@false

all : stage1 stage2

clean: stage1clean stage2clean

stage1 : stage1a stage1b stage1c
stage1a :
	$(MAKE) -f compiler+tools.mk STAGE=1 all install
stage1b :
	$(MAKE) -f libraries.mk STAGE=1 all install
stage1c :
	PATH="$(abspath stage1/ubin):$(PATH)" OD="$(abspath tmp-stage1)/" INSTDIR="$(abspath stage1)/" dxmake all install

.PHONY : stage1 stage1a stage1b stage1c

stage1clean :
	$(MAKE) -f compiler+tools.mk STAGE=1 clean
	$(MAKE) -f libraries.mk STAGE=1 clean
	rm -rf tmp-stage1 stage1
.PHONY : stage2clean

# Stage 2 stuff now needs chucked out of here - it's going to be changed to use dxmake on an Amiga, so there's no need for the GNU makefiles to have anything to do with it any more.

stage2 :
	$(MAKE) -f compiler+tools.mk STAGE=2 all install
.PHONY : stage2

stage2clean :
	$(MAKE) -f compiler+tools.mk STAGE=2 clean
	$(MAKE) -f libraries.mk STAGE=2 clean
	rm -rf tmp-stage2 stage2
.PHONY : stage2clean
