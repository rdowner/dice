
INSTDIR= DCC:
;OD= $(TOPDIR)tmp-stage2/
;BIN= $(INSTDIR)/bin/
ULIB= $(TOPDIR)ulib/
DLIB= $(TOPDIR)dlib/
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

RM=rm -f
MKDIR=mkdir
INSTALL=install -c -m 0755
CHMOD=chmod
MAKEPROTO=fgrep -h Prototype

.endif
