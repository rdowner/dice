
OD= $(TOPDIR)obj/
UBIN= $(TOPDIR)ubin/
ULIB= $(TOPDIR)ulib/
DLIB= $(TOPDIR)dlib/
INCLUDE= $(TOPDIR)include/
INSTDIR= /usr/local/dice/
ENV= DERRORS=$(TOPDIR)config/dice.errors; DINCLUDE=$(INCLUDE); export DERRORS; export DINCLUDE;

CFLAGS= -Wall -Wstrict-prototypes -Werror -O2 -DREGISTERED -DCOMMERCIAL -DNO_ASM -DINTELBYTEORDER -D_INSTDIR=$(INSTDIR) -I$(OD). -I$(TOPDIR). -L$(OD). -g

