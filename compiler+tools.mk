
# Populate DIRS with the subdirectories containing the build components
DIRS = suplib src/dxmake

# the sets of directories to do various things in
BUILDDIRS = $(DIRS:%=build-%)
INSTALLDIRS = $(DIRS:%=install-%)
UNINSTALLDIRS = $(DIRS:%=uninstall-%)
CLEANDIRS = $(DIRS:%=clean-%)
TESTDIRS = $(DIRS:%=test-%)

all: $(BUILDDIRS)
$(DIRS): $(BUILDDIRS)
$(BUILDDIRS):
	$(MAKE) -C $(@:build-%=%) SUBDIR=$(@:build-%=%)

install: $(INSTALLDIRS)
$(INSTALLDIRS):
	$(MAKE) -C $(@:install-%=%) SUBDIR=$(@:install-%=%) install

uninstall: $(UNINSTALLDIRS)
$(UNINSTALLDIRS):
	$(MAKE) -C $(@:uninstall-%=%) SUBDIR=$(@:uninstall-%=%) uninstall

test: $(TESTDIRS)
$(TESTDIRS): 
	$(MAKE) -C $(@:test-%=%) SUBDIR=$(@:test-%=%) test

clean: $(CLEANDIRS)
$(CLEANDIRS): 
	$(MAKE) -C $(@:clean-%=%) SUBDIR=$(@:clean-%=%) clean


.PHONY: subdirs $(DIRS)
.PHONY: subdirs $(BUILDDIRS)
.PHONY: subdirs $(INSTALLDIRS)
.PHONY: subdirs $(UNINSTALLDIRS)
.PHONY: subdirs $(TESTDIRS)
.PHONY: subdirs $(CLEANDIRS)
.PHONY: all install clean 
