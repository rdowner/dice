
INSTDIR= DCC:
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
ENV= 
# ENV= DERRORS=$(TOPDIR)config/dice.errors; DINCLUDE=$(INCLUDE); export DERRORS; export DINCLUDE;

CC=dcc
CFLAGS= -2.0 -// -DNO_ASM -I$(TOPDIR)

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
