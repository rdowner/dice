# Makefiles for the individual modules should include this Makefile as follows:
#
# TOPDIR = (relative path back to this directory, e.g. "../..")
# BIN = (name of the binary to build; optional)
# LIB = (name of the library to build; optional)
# C_SRCS = (list of C source files; optional)
# PROTOS = (name of the header file for automatically-generated prototypes; optional)
# include $(TOPDIR)/common.mk

BUILDDIR = /tmp/dtmp/obj1/$(SUBDIR)

OBJS = $(addprefix $(BUILDDIR)/, $(C_SRCS:.c=.o))

CFLAGS = -Wall -Wno-unused-result -Wstrict-prototypes -Werror
CFLAGS += -g
CFLAGS += -DNO_ASM -DINTELBYTEORDER -D__STDC_WANT_LIB_EXT2__ -D_INSTDIR=$(abspath $(TOPDIR))/
CFLAGS += -I$(BUILDDIR) -I$(TOPDIR)

SUPLIB = -L /tmp/dtmp/obj1/suplib -lamiga

ifdef BIN
BIN_WITH_PATH = $(BUILDDIR)/$(BIN)
endif
ifdef LIB
LIB_WITH_PATH = $(BUILDDIR)/$(LIB)
endif

all : $(BUILDDIR) $(LIB_WITH_PATH) $(BIN_WITH_PATH)

clean :
	-rm -f $(LIB_WITH_PATH) $(BIN_WITH_PATH) $(OBJS) $(LIB_OUT) $(PROTOS) .depend

install :
ifdef BIN
	install -d $(TOPDIR)/bin1-bootstrap
	install -m 755 $(BIN_WITH_PATH) $(TOPDIR)/bin1-bootstrap
endif

uninstall :
ifdef BIN
	rm -f $(TOPDIR)/bin1-bootstrap/$(BIN)
endif

$(BUILDDIR) :
	mkdir -p $@

ifdef PROTOS
$(PROTOS) : $(C_SRCS)
	-fgrep -h Prototype $^ >$@.new
	diff -q $@.new $@ || mv -f $@.new $@
endif

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $^

ifdef BIN_WITH_PATH
$(BIN_WITH_PATH) : $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(SUPLIB)
endif

ifdef LIB_WITH_PATH
$(LIB_WITH_PATH) : $(OBJS)
	ar $(ARFLAGS) $@ $^
endif

ifdef C_SRCS
.depend : $(PROTOS)
	mkdep $(CFLAGS) $(C_SRCS)
include .depend
endif
