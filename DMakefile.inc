
#OD= $(TOPDIR)obj/
BIN= $(INSTDIR)/bin/
#ULIB= $(TOPDIR)ulib/
#DLIB= $(TOPDIR)dlib/
#INCLUDE= $(TOPDIR)include/
#INSTDIR= /usr/local/dice/
#ENV= DERRORS=$(TOPDIR)config/dice.errors; DINCLUDE=$(INCLUDE); export DERRORS; export DINCLUDE;

CC=dcc
CFLAGS= -2.0 -// -DNO_ASM -DCROSS_COMPILE -I$(TOPDIR)
