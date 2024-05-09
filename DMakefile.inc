
OD= $(TOPDIR)obj/
UBIN= $(TOPDIR)ubin/
ULIB= $(TOPDIR)ulib/
DLIB= $(TOPDIR)dlib/
INCLUDE= $(TOPDIR)include/
INSTDIR= /usr/local/dice/
ENV= DERRORS=$(TOPDIR)config/dice.errors; DINCLUDE=$(INCLUDE); export DERRORS; export DINCLUDE;

CFLAGS= -Wall -Wno-unused-result -Wstrict-prototypes -Werror -O2 -DCOMMERCIAL -DNO_ASM -DINTELBYTEORDER -D__STDC_WANT_LIB_EXT2__ -D_INSTDIR=$(INSTDIR) -I$(OD). -I$(TOPDIR). -L$(OD). -g

