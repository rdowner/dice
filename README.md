DICE-nx - a C compiler for Commodore Amiga computers and embedded 68k systems
=============================================================================

This is DICE-nx - a C compiler for Commodore Amiga computers and embedded 68k
systems. DICE-nx includes all the tools needed to build Amiga executables - the
C compiler itself, as well as an assembler, linker and numerous other support
tools. It also has several Amiga-specific features, such as automatic opening
of the common Amiga OS libraries, easy support for ARexx, and more. DICE can
also target embedded systems using 68000 processors.

DICE-nx is a fork of DICE, which was originally a commercial product written by
Matt Dillon in the 1990s. In the late 1990s, Matt largely ceased development of
DICE and released the source code under a "non-commercial use only" license. A
small amount of development continued on DICE in the 2000s, largely around
porting to 64-bit UNIX hosts and targetting embedded 68000 systems. In 2023,
Matt re-licensed it under the standard 3-clause BSD license, meaning for the
first time ever DICE was available under a true open-source license.

DICE-nx aims to re-start development of DICE, and focusing on its original aim
of providing a fully-featured C compiler for Amiga computers. At this time, the
original 68000-based Amiga series is seeing increased support from
retro-computing enthusiasts, including the restarting of AmigaOS development
with the release of AmigaOS 3.2. DICE-nx aims to provide for this audience,
fully supporting AmigaOS 3.2 and native development on real Amiga hardware, and
a secondary aim of supporting those who want to cross-compile for the Amiga
from cheap and powerful Linux systems.

DICE-nx is currently maintained by Richard Downer, who can be emailed at
[dice@richard.downer.tech](mailto:dice@richard.downer.tech). DICE-nx is not
associated with Matt Dillon or the original DICE project. I have been in
contact with Matt and although he is encouraging of this effort, it is not
endorsed by Matt and bears a slightly different name at Matt's request. Matt's
name appears extensively throughout this repository, but he is not involved in
DICE-nx development or support. Therefore, please direct any questions or
discussions to the above email address or the GitHub repository for DICE-nx.

---

*Following is the original README from the original DICE project.*

So here it is, BSD licensed.  The DICE compiler from my Amiga days.
I have sinced made it compile under bsd/linux on hopefully 32 or 64
bit architectures (it may need a bit of work to compiler under linux
but since it compiles under BSD the work should be minimal).

It will output 68K assembly.  Since it was originally targetted to
the Amiga, DICE totally depended on the Amiga libraries for all floating
point support so this version here which compiles under unix and
generates 68K basically has NO floating point support.  Sorry, nothing
I can do about that :-).  Nor is there any 64-bit integer support.
A "long" is 32 bits,

I wrote the original DICE many many years ago and sold it as shareware
for the Amiga.  It originally cost $50 and many people got their start
on C with it on the Amiga.  I originally wrote it because Lattice C
and Aztec C were both too expensive for many developers just starting
out, and I wanted the community to have something that included full
sources that people could use and learn from.

For quite a few years in the 1990's I would receive cashiers checks in
the mail from the U.S. and many other countries and would dutifully
generate the floppy disks, put them in a mailer, and mail them out.
DICE did not make me a millionaire or anything like that, but it did
provide a second income and I appreciate the life options that gave me
in later years.

Later on a number of people, particularly John Toebes and Bryce Nesbit,
encouraged me to put together a company (which John did most of the
work for) and spruce up the compiler.  That company was called
Obviously Implementations Corp.  It was basically a 3-man company
and we continued to sell DICE under that name for (I think) the same
price.

When we finally decided to end, many years later, a few checks would
still come in every once in a while.  My good friend Bryce took on the
task of attempting to return these cashiers checks to their owners
along with a pointer to the now (almost) open-source DICE sources.

As of now, all DICE sources are BSD licensed and truly open source.

It is still mostly K&R C because ANSI C was quite new at the time.  Still
prototyped, however, and compiles pretty cleanly with modern C compilers.

I do not know if it will self-compile on 68K, that is a code path I haven't
been able to test when I ported it over to UNIX.


DICE source code release README
-------------------------------

(NOTE, some of these instructions may be obsolete or inapplicable now)

(c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
use is allowed under the terms of the [LICENSE](LICENSE) file.

Well, here it is.  The source to DICE.  I really meant to release this a
lot sooner.  I plead laziness :-)  Note that I haven't touched my Amiga
for quite a while, and the library support may very well be lacking.
Support up to the amiga 3.0 includes / library is included, but the
amiga includes themselves are not included because I am not sure what
the disposition of commodore's copyright is.

DICE is a 68000 compiler.  It was written as an Amiga C compiler, but it
can really be used as a generic 68000 C compiler.  Even though I haven't
touched an Amiga for a while, I still use DICE on my FreeBSD box to 
compile 68000 projects.

DICE will compile on an Amiga, linux, or FreeBSD machine.  Most of the
utilities will generate the correct 68000 byte ordering even when compiled
on an Intel box.  It will generate Amiga binaries and utilities are 
included to generate ROMable code.   All DICE features work and the
relocateable and small-data models are incredibly powerful when used in
an embedded environment.

My email address and web pages

* dillon@backplane.com
* http://www.backplane.com/

I am willing to maintain the source tree, at least for a little while.
Bug fixes should be emailed to me.


Missing files
-------------

There is some confusion as to whether the Amiga includes can be 
distributed.  Before commodore went belly-up, they did make the .FD
files free, so I am including the fd/ and clib/ subdirectories for
the 1.3, 2.0. and 3.0 includes but I am not including any of the
commodore structural includes (exec/, intuition/, etc...)

If you have the amiga includes, the 1.3 includes should be placed in
include/amiga13, the 2.0 placed in include/amiga20, and so on.

What this means is that you CAN generate an 'amiga.lib' and pragmas
from the .fd files, but the structural includes aren't available unless
you already have them.

The amiga includes are only required if you are compiling the
compiler on the Amiga or compiling for an Amiga target.  These 
includes are not required if you use DICE to compile embedded 68000 code.


Compiling DICE under UNIX for cross-compiling
---------------------------------------------

NOTE: Floating point constants are not supported because they require
the Amiga FP library.  However, you can generate all amiga libraries
except those routines which employe FP constants from your sources.

(1) install dmake.  Unfortunately many years after dmake someone wrote
a dmake so to avoid confusion, my dmake was renamed to dxmake.

I'm sure someone will be able to clean up the topology and generate
Makefile's for BSD make or gmake, but I haven't done that so...

(2) setup target directories via the top-level DMakefile and DMakefile.inc.
Get the byte order -D option correct.  It defaults to -DINTELBYTEORDER
(little endian I think).

(3) dxmake inside the project directory.

    dxmake obj
    dxmake
    dxmake install

(4) AMIGA SPECIFIC AMIGAxxx.LIB EQUIVALENTS

These libraries are provided precompiled in the distribution and may
also be regenerated.

You can create amigaxxx.lib equivalents.  The auto library is used
to automatically open those system libraries used by the code.

    cd lib
    dxmake -f DMakefile.unix amiga13
    dxmake -f DMakefile.unix amiga20
    dxmake -f DMakefile.unix auto

(4) ROM LIBRARY, AMIGA LIBRARIES, AMIGA SUPPORT LIBRARIES

These are in the lib/ subdirectory, read the DMakefile's available.

    dxmake -f DMakefile.unix rom

DICE includes stdio, string, and other Amiga-specific support 
libraries.   If you have the amiga includes installed, you can compile
these libraries simply with:

    cd lib
    dxmake -f DMakefile.unix all

Most DICE use cases these days do not use the original Amiga OS and
you are kinda on your own in terms of putting together stdio-like
support libraries.


Compiling DICE under the Amiga
------------------------------

Yes you can do it.  Unfortunately, I haven't run the compile myself for
a long time so you will probably have to mess around with the various
DMakefile's.  The DMakefile's for the compiler & utilities look something
like:  src/dc1/DC1.DICE.  You compile by cd'ing into src/dc1 and then
doing a 'DMakefile -f DC1.DICE'.  You will have to setup a compilation
environment, though, and since I don't have an Amiga handy I can't give
you exact instructions.

To compile DICE on the Amiga you need DICE.  This shouldn't be a problem,
I include full amiga compiler binaries.  I do not include the
commodore-specific Amiga includes, however... you need to get those
separately.


Directories
-----------

    abin        amiga binaries
    dlib        amiga/68000 support libraries

    ubin        unix binaries
    ulib        unix support libraries

    src         primary utility and compiler source
    lib         amiga/68000 support library source
    suplib      unix support library source

    include     header files for amiga/68000 libraries, including
                selected portions of the 1.3, 2.0, and 3.0 includes
                (the clib and fd directories only), and the DICE
                ANSI library.

    config      Various amiga-related DMakefile's for compiling
                DICE (included as-is)

    doc         Library and compiler documentation
                (included as-is)


    installer   Bunches of installation scripts.  Included AS-IS.

    master      Random junk (used to be the disk template that I used
                to make floppies).  Removed duplicate information that
                already exists in other directories.
