
.ifdef OD
.else
OD= /tmp/dtmp/
.endif
.ifdef BIN
.else
BIN= $(TOPDIR)bin/
.endif
ULIB= $(TOPDIR)ulib/
INCLUDE= $(TOPDIR)include/


.ifdef CROSS_COMPILE

CC=cc
CFLAGS = -Wall -Wno-unused-result -Wstrict-prototypes -Werror
CFLAGS = $(CFLAGS) -g
CFLAGS = $(CFLAGS) -DNO_ASM -DINTELBYTEORDER -D__STDC_WANT_LIB_EXT2__ -D_INSTDIR=$(INSTDIR)
CFLAGS = $(CFLAGS) -I$(TOPDIR)
SUPLIB = -L /tmp/dtmp/obj1-bootstrap/suplib -lamiga

.else

CC=dcc
CFLAGS= -2.0 -// -DNO_ASM -I$(TOPDIR)
SUPLIB =

.endif

.ifos AmigaOS

RM=Delete
MKDIR=MakeDir
INSTALL=Copy
CHMOD=Echo Not supported on Amiga: chmod
MAKEPROTO=makeproto

.else

CFLAGS= $(CFLAGS) -DCROSS_COMPILE
RM=rm -f
MKDIR=mkdir
INSTALL=install -c -m 0755
CHMOD=chmod
MAKEPROTO=fgrep -h Prototype

.endif
