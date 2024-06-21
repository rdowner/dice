Building DICE
=============

**NOTE: build system currently undergoing repair/replacement - work in progress**

Prerequisites
-------------

* The `mkdep` command (see: https://command-not-found.com/mkdep)
* The Amiga NDKs (Native Developer Kits). NDK for AmigaOS 2.0 is compulsory; additional versions
  can also be added.


Theory
------

Given the aim of making a full binary distribution of DICE for the Amiga, but starting from a Linux
or BSD PC, starts with a three-stage build process. Within each stage there are two parts - the
first to build the compiler and essential tools, the second which then uses those tools to build
the standard library and AmigaOS link libraries. This is followed by a fourth stage which builds
the compiler again, along with the full suite of tools (not just those that are essential to the
build). Finally, the fifth stage pulls together the remaining distribution material, such as the
README files and documentation, to result in a full binary distribution of DICE.

In more detail, the stages and their associated parts are:

* **Stage 1a** executes on a Linux or BSD PC. It uses the host's own C compiler (usually GCC) to
  build a minimal but functional version of DICE that is suitable for the host's native platform,
  and which produces Amiga outputs. (DICE can only ever produce Amiga outputs, whether it is
  running on an Amiga or on a Linux PC.)
* **Stage 1b** executes in the same environment, and uses the stage 1a DICE compiler to build the
  minimum require components from the DICE standard libraries and AmigaOS support libraries.
  At this point there is a fully functional, if minimal, distribution of the DICE compiler, tools
  and libraries.
* **Stage 2a** again executes on a Linux or BSD PC. It uses the stage 1 outputs over the same DICE
  source code, and this time it produces native Amiga executable versions of the DICE compiler and
  essential tools.
* **Stage 2b** switches to an Amiga. Using the Amiga-hosted DICE compiler and tools from stage 2a,
  it builds the DICE standard library and AmigaOS support libraries again.
* **Stage 3a** begins the fully Amiga-hosted build cycle. The Amiga-hosted DICE compiler, tools and
  libraries from stage 2 are now used to compile the DICE compiler and tools again, this time
  without any need of the Linux or BSD host.
* **Stage 3b** again builds the DICE standard library and AmigaOS support libraries; however this
  time the entire suite of libraries is built, not just those that are essential to the build.
* **Stage 4** makes a final rebuild the DICE compiler, along with the entire tool suite, not just
  those that are essential to the build. The final produce here is a set of standard libraries, and
  the compiler and tool suite linked with those same libraries.
* **Stage 5** brings in the documentation, READMEs, examples, and other extra resources that form
  the full binary distribution.

At the end of this process, there is a fully functional, comprehensive binary distribution of DICE,
ready to be archived and published.


Known limitations and future improvements
-----------------------------------------

The first host for the build must be a Linux or BSD PC on an Intel x86-64 CPU, due to assumptions
about byte ordering. It should be relatively easy to fix this though.


Full multi-stage build process
------------------------------

Build stage 1:

```bash
make stage1
```
