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


Status
------

Please read [STATUS.md](STATUS.md) for the current status of DICE-nx.


Building
--------

Please read [BUILDING.md](BUILDING.md) for instructions on building DICE-nx.
Note that building DICE-nx is more complicated than most other open source
projects!


Changelog
---------

This is located in [CHANGELOG.md](CHANGELOG.md).


Contributing
------------

If you would like to contribute to DICE-nx, I would be interested in hearing
from you!

DICE-nx follows the principles of [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/)
and [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).
