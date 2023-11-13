
CFLAGS= -O2 -Wall -I..
OD= objs/
SRCS= addhead.c addtail.c newlist.c remhead.c \
        gethead.c strdup.c getsucc.c gettail.c \
        getpred.c remove.c strnicmp.c getcwd.c \
        findname.c stricmp.c enqueue.c strins.c \
        insert.c

OBJS= $(OD)addhead.o $(OD)addtail.o $(OD)newlist.o $(OD)remhead.o \
        $(OD)gethead.o $(OD)strdup.o $(OD)getsucc.o $(OD)gettail.o \
        $(OD)getpred.o $(OD)remove.o $(OD)strnicmp.o $(OD)getcwd.o \
        $(OD)findname.o $(OD)stricmp.o $(OD)enqueue.o $(OD)strins.o \
        $(OD)insert.o

LIB= libamiga.a

all: $(OD) $(LIB)

$(LIB) : $(OBJS)
        ar cr $@ $(OBJS)
        ranlib $@

$(OD)addhead.o : addhead.c
        cc $(CFLAGS) -c -o $@ $*.c

$(OD)addtail.o : addtail.c
        cc $(CFLAGS) -c -o $@ $*.c

$(OD)newlist.o : newlist.c
        cc $(CFLAGS) -c -o $@ $*.c

$(OD)remhead.o : remhead.c
        cc $(CFLAGS) -c -o $@ $*.c

$(OD)gethead.o : gethead.c
        cc $(CFLAGS) -c -o $@ $*.c

$(OD)strdup.o : strdup.c
        cc $(CFLAGS) -c -o $@ $*.c

$(OD)getsucc.o : getsucc.c
        cc $(CFLAGS) -c -o $@ $*.c

$(OD)gettail.o : gettail.c
        cc $(CFLAGS) -c -o $@ $*.c

$(OD)getpred.o : getpred.c
        cc $(CFLAGS) -c -o $@ $*.c

$(OD)remove.o : remove.c
        cc $(CFLAGS) -c -o $@ $*.c

$(OD)strnicmp.o : strnicmp.c
        cc $(CFLAGS) -c -o $@ $*.c

$(OD)getcwd.o : getcwd.c
        cc $(CFLAGS) -c -o $@ $*.c

$(OD)findname.o : findname.c
        cc $(CFLAGS) -c -o $@ $*.c

$(OD)stricmp.o : stricmp.c
        cc $(CFLAGS) -c -o $@ $*.c

$(OD)enqueue.o : enqueue.c
        cc $(CFLAGS) -c -o $@ $*.c

$(OD)strins.o : strins.c
        cc $(CFLAGS) -c -o $@ $*.c

$(OD)insert.o : insert.c
        cc $(CFLAGS) -c -o $@ $*.c

$(OD) : $(OD)
        mkdirs $@

clean:
        rm -f $(OD)*
