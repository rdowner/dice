
stage1 :
	$(MAKE) -f compiler+tools.mk STAGE=1 all install
	$(MAKE) -f libraries.mk STAGE=1 all install
.PHONY : stage1
