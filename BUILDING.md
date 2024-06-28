Building DICE
=============

**NOTE: build system currently undergoing repair/replacement - work in progress**

Prerequisites
-------------

* The `mkdep` command (see: https://command-not-found.com/mkdep)
* The Amiga NDKs (Native Developer Kits). NDK for AmigaOS 2.0 is compulsory; additional versions
  can also be added.


Build stages
------------

Given the aim of making a full binary distribution of DICE for the Amiga, but starting from a Linux
or BSD PC, there's a two-stage build process needed.

**Stage 1** takes place on a UNIX-like OS, such as Linux. It requires the OS standard C compiler
and related developer tools, and will bootstrap a basic version of DICE C.

**Stage 2** takes place on an Amiga. It takes the stage 1 output and repeats the build process,
this time using AmigaOS and DICE itself to build another basic version of DICE C.

**Stage 3** augments the stage 2 output with the full suite of DICE tools, plus documentation,
examples, etc. to make a full Amiga binary distribution of DICE C.


Known limitations and future improvements
-----------------------------------------

The first host for the build must be a Linux or BSD PC on an Intel x86-64 CPU, due to assumptions
about byte ordering. It should be relatively easy to fix this though.

It would be nice to use the excellent [vamos tool by Christian Vogelgsang](https://github.com/cnvogelg/amitools/blob/master/docs/vamos.md),
but DICE currently relies on many OS calls not currently supported by vamos. It should be possible
to implement the missing OS calls, but I'm not ready to do this just yet. In the mean time, just
use a real Amiga, or emulate one using WinUAE or a similar tool.


Full multi-stage build process
------------------------------

Build stage 1:

```bash
make stage1
```
