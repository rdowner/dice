
default :
	false

all : stage1 stage2

clean: stage1clean stage2clean

stage1 :
	$(MAKE) -f compiler+tools.mk STAGE=1 all install
	$(MAKE) -f libraries.mk STAGE=1 all install
.PHONY : stage1

stage1clean :
	$(MAKE) -f compiler+tools.mk STAGE=1 clean
	$(MAKE) -f libraries.mk STAGE=1 clean
	rm -rf tmp-stage1 stage1
.PHONY : stage2clean

stage2 :
	$(MAKE) -f compiler+tools.mk STAGE=2 all install
	$(MAKE) -f libraries.mk STAGE=2 all install
.PHONY : stage2

stage2clean :
	$(MAKE) -f compiler+tools.mk STAGE=2 clean
	$(MAKE) -f libraries.mk STAGE=2 clean
	rm -rf tmp-stage2 stage2
.PHONY : stage2clean
