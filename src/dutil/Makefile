
TOPDIR = ../..
include $(TOPDIR)/common.mk

# This one is a bit different...! Instead of the directory containing lots of C source code that is
# compiled into a single binary, this directory generates a number of utility programs, each from a
# single C source file. So we include common.mk to get some useful definitions, but the rest we
# work out ourselves.

C_SRCS = bintohex.c fdtolib.c fdtopragma.c istrip.c lbmake.c libtos.c makeproto.c romable.c hunks.c uprev.c
BINS = $(addprefix $(BUILDDIR)/, $(C_SRCS:.c=))

all : $(BINS)

%: %.o
	$(CC) $(LDFLAGS) -o $@ $^ $(SUPLIB)

clean :
	-rm -f $(BINS) $(OBJS) .depend

install :
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(BINS) $(DESTDIR)$(PREFIX)/bin

uninstall :
	rm -f $(addprefix $(DESTDIR)$(PREFIX)/bin/, $(C_SRCS:.c=))
