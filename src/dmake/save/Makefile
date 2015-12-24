
#   Makefile

OD= objs/
SRCS= main.c parse.c cmdlist.c convert.c depend.c var.c run.c buffer.c \
      subs.c
OBJS= $(OD)main.o $(OD)parse.o $(OD)cmdlist.o $(OD)convert.o $(OD)depend.o \
	$(OD)var.o $(OD)run.o $(OD)buffer.o $(OD)subs.o

CFLAGS= -I.. -L../suplib
PROTOS= dmake-protos.h
EXE= /usr/local/bin/dmake

all: $(OD) $(PROTOS) $(EXE)

$(EXE) : $(OBJS)
	cc $(CFLAGS) -o $@ $(OBJS) -lamiga

$(OD)main.o : main.c
	cc $(CFLAGS) -c -o $@ $*.c

$(OD)parse.o : parse.c
	cc $(CFLAGS) -c -o $@ $*.c

$(OD)cmdlist.o : cmdlist.c
	cc $(CFLAGS) -c -o $@ $*.c

$(OD)convert.o : convert.c
	cc $(CFLAGS) -c -o $@ $*.c

$(OD)depend.o : depend.c
	cc $(CFLAGS) -c -o $@ $*.c

$(OD)var.o : var.c
	cc $(CFLAGS) -c -o $@ $*.c

$(OD)run.o : run.c
	cc $(CFLAGS) -c -o $@ $*.c

$(OD)buffer.o : buffer.c
	cc $(CFLAGS) -c -o $@ $*.c

$(OD)subs.o : subs.c
	cc $(CFLAGS) -c -o $@ $*.c

$(PROTOS) : $(SRCS)
	-rm -f $@
	makeproto -o $@ $(SRCS)

$(OD) : $(OD)
	mkdirs $@

clean:
	-rm -f $(OBJS)
	-rm -f $(PROTOS)

