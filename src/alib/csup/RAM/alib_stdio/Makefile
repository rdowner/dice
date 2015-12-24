########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	printf.obj\
	sprintf.obj\
	fclose.obj\
	fgetc.obj\
	fprintf.obj\
	fputc.obj\
	fputs.obj\
	getchar.obj\
	putchar.obj\
	puts.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../alib_stdio.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
