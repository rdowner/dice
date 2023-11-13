##########################################################################
MAKEMETA=       ../../../tools/makemeta

SUBSYSNAME=     math_support
SUBDIRS=        RAM ROM

AFILES=         ascint.asm ffp10tbl.asm ffpafp.asm ffparnd.asm \
                ffpdbf.asm ffpfpa.asm

OFILES=         ascint.obj ffp10tbl.obj ffpafp.obj ffparnd.obj \
                ffpdbf.obj ffpfpa.obj

all quick:      ${SUBDIRS} ${SUBSYSNAME}_lib.lib
                cat ${SUBSYSNAME}_lib.lib RAM/*.obj >amiga.lib
                cat ${SUBSYSNAME}_lib.lib ROM/*.obj >rom.lib

.INCLUDE=${MAKEMETA}
