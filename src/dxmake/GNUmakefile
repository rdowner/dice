# Copyright (c) 1992-2000,2023 The DragonFly Project.  All rights reserved.
#
# This code is derived from software contributed to The DragonFly Project
# by Matthew Dillon <dillon@backplane.com>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. Neither the name of The DragonFly Project nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific, prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
# COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
# OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
SRCS= buffer.c cmdlist.c convert.c depend.c main.c parse.c run.c subs.c var.c lists.c cond.c
OBJS= buffer.o cmdlist.o convert.o depend.o main.o parse.o run.o subs.o var.o lists.o cond.o

PREFIX = /usr/local

EXED= $(PREFIX)/bin
EXE= dxmake
CC= gcc

CFLAGS= -Wall -Wno-unused-result -Wstrict-prototypes -O2 -g
LFLAGS= -g

PROTOS= dmake-protos.h

all: $(PROTOS) $(EXE)

$(EXE) : $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) -o $(EXE)
#	strip $(EXE)

$(PROTOS) : $(SRCS)
	@rm -f $(PROTOS) $(PROTOS).new
	cat $(SRCS) | egrep '^Prototype' > $(PROTOS).new
	@mv -f $(PROTOS).new $(PROTOS)

install:
	rm -f $(EXED)/$(EXE).new
	cp $(EXE) $(EXED)/$(EXE).new
	chmod 755 $(EXED)/$(EXE).new
	mv -f $(EXED)/$(EXE).new $(EXED)/$(EXE)

clean:
	rm -f $(PROTOS) $(PROTOS).new $(OBJS) $(EXE)
