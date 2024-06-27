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
PREFIX ?= /usr/local
PROG= dxmake
PROTOS= dmake-protos.h
SRCS= buffer.c cmdlist.c convert.c depend.c main.c parse.c run.c subs.c var.c lists.c cond.c
HFILES= defs.h dmake_str.h lists.h tokens.h dmake-protos.h
BINDIR= $(PREFIX)/bin/
MANDIR= $(PREFIX)/man/man
TARFILES= $(SRCS) Makefile Makefile.portable dxmake.1 $(HFILES)
CFLAGS= -Wall -Wstrict-prototypes
REV= 2.12
PORTTAR= /home/dillon/htdocs/FreeBSDPorts/$(PROG)-$(REV).tar.gz

all:	$(PROTOS)

$(PROTOS) : $(SRCS)
	(cd ${.CURDIR}; cat $(SRCS)) | egrep '^Prototype' > $(PROTOS)

tar:	clean
	rm -f dmake-protos.h
	make -f Makefile.portable dmake-protos.h
	rm -rf /tmp/$(PROG)-$(REV)
	mkdir /tmp/$(PROG)-$(REV)
	tar cf - $(TARFILES) | (cd /tmp/$(PROG)-$(REV); tar xvpf -)
	(cd /tmp; tar czf $(PORTTAR) $(PROG)-$(REV))
	chown dillon $(PORTTAR)
	md5 $(PORTTAR)

.include <bsd.prog.mk>

