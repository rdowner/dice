######################################################################
MAKEMETA=       ../../../tools/makemeta

SUBSYSNAME=     exec_support

AFILES=         BeginIO.asm NewList.asm pools.asm cpools.asm

CFILES=         CreatePort.c CreateTask.c CreateStdIO.c CreateExtIO.c

OFILES=         BeginIO.obj NewList.obj CreatePort.obj CreateStdIO.obj \
                CreateExtIO.obj CreateTask.obj \
                pools.o cpools.o

all quick:      ${SUBSYSNAME}_lib.lib

.INCLUDE=${MAKEMETA}
