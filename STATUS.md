DICE-nx Status
==============

The file summarises the current development status of the DICE-nx project -
what things are working, what is not working, and what is planned.

The good news is that DICE-nx appears to be working well, and able to generate
working Amiga binaries that can operate on AmigaOS 3.2 on real Amiga hardware.
This is after a prolonged period of time where DICE would only as a
cross-compiler hosted on UNIX-like systems and where Amiga native functionality
suffered. Once DICE-nx could be persuaded to build Amiga native binaries again,
lots of well-established DICE features came back to life and worked as
expected.

That said, there is still significant work to be done before a DICE-nx reaches
release quality again.


AmigaOS NDK compatibility
-------------------------

I made the decision to support only the AmigaOS 3.2 NDK. This is because I
consider AmigaOS 3.2 support to be essential for the Amiga community in 2024.
If the compiler supports 3.2, then it is still possible to build Amiga binaries
that target 3.0, 2.0 and 1.3, and maybe even earlier. If you do want to target
earlier AmigaOS versions, then you must obviously take care to not use any
features that are not supported by the target AmigaOS version - however that
has always been the case for developing on the Amiga.

Support for the earlier Commodore NDKs is desirable however, and I would like
to support them in the future.

I currently do not plan to support the 3.5 and 3.9 series of AmigaOS releases.
There is currently no legal way to obtain these AmigaOS releases, which makes
it difficult to develop and test DICE-nx on them. I am interested in hearing
from people who would like to see this support added to DICE-nx - if there is
sufficient interest - and a way for me to do this legally - I would consider
it.


amiga.lib
---------

DICE could use the amiga.lib from the Amiga NDK for the large-model,
stack-argument variant `amigaXXl.lib`, and with the use of the `libtos` tool,
it could derive a small-model, stack-argument variant `amigaXXs.lib`. But for
the register-argument variants and profiling-enabled variants (variants with
`r` and `p` in their suffix), DICE built its own version of amiga.lib.

DICE-nx does not currently have a buildable amiga.lib of its own. Therefore,
enabling register-argument builds with `-mr`, `-mR` or `-mRR`, and/or enabling
profiled builds with `-prof`, will not currently work.

The lack of register-argument builds can be ameliorated by enabling inline OS
calls by using the `-mi` build option and including the OS headers in the
`pragmas` directory. For example:

```C
#include <pragmas/dos_pragmas.h>
```

This restriction does not apply if you are not linking with the Amiga OS
library - for example, when building ROMable code.

The long-term solution here is obviously to fix the build of the DICE-nx
version of amiga.lib. However it is also significantly outdated compared to the
AmigaOS 3.2 NDK version of amiga.lib, which has added many functions over the
various releases of AmigaOS. Efforts need to be made to make the DICE-nx
amiga.lib compatibile with the NDK 3.2 version of amiga.lib.


Branding
--------

Following a request from Matt Dillon, the name of DICE had to be changed
slightly to make it clear that this is a fork. The name of this fork is now
"DICE-nx". There needs to be a comprehensive effort to ensure that the name
"DICE-nx" is used by default, and use of the name "DICE" is only used when
referring to the historical DICE project.


Releasing
---------

Some thought needs to be given to how to release DICE-nx. This would likely
take the form of source and Amiga-native binary releases. Any files from the
AmigaOS 3.2 NDK would have to be stripped out during the release process as
they are subject to copyright, which means that - especially for the binary
releases - there needs to be a way for users receiving a DICE-nx packaged
release to install NDK 3.2 themselves.


Documentation
-------------

The DICE documentation contains lots of very good detail, but is not organised
very well and contains much outdated information. It needs an overhaul to make
it accurate, usable, easy to navigate, and in a format that meets expectations
in 2024.


Modernisation
-------------

DICE was written in the era where ANSI C had been recently standardised, but
most existing code (and coder's habits) of the time was in K&R-style C. K&R
syntax is used by default in the DICE source code. An effort to make it fully
ANSI C (now retconned as "C90") compliant is worthwhile.

Future releases of DICE-nx could even add compliance with C99, C11 or C17.


Bugs
----

There are bugs, of course, as in any complex piece of software. These will be
tracked using GitHub issues rather than being documented here.
