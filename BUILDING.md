Building DICE
=============

**NOTE: build system currently undergoing repair/replacement - work in progress**

There's several possible ways of building DICE, depending on what computer you are starting on and
what computer you want to finish on!

DICE can only generate Motorola 68000 machine code, and it can generate either Amiga object or
executable files, or binary code suitable to burning onto ROM for a 68k embedded system.

DICE can be hosted on either a UNIX-like operating system on an Intel x86 or x86-64 CPU, or on a
real Amiga. (Hosting it on UNIX-like operating systems with a CPU architecture other than
x86/x86-64 is not currently supported but should not be difficult to add.)

There is an important limitation when NOT building DICE on an Amiga: the DICE produced will not
support floating point data. If you need floating point, you need to build DICE on an Amiga (real
or emulated, as long as it runs AmigaOS).


Recommended build path
----------------------

Here we describe the recommended build path. At the end of the document you can find some
suggestions of alternative supported ways to build DICE. (There are still other ways too, but they
are not supported.)

This build path has three stages:

**Stage 1** takes place on a UNIX-like OS, such as Linux. It requires the OS standard C compiler
and related developer tools, and will bootstrap a basic version of DICE C.

**Stage 2** takes place on an Amiga. It takes the stage 1 output and repeats the build process,
this time using AmigaOS and DICE itself to build another basic version of DICE C.

**Stage 3** repeats stage 2, but this time builds the full suite of tools and libraries, and adds
the documentation, examples, etc. to make a full Amiga binary distribution of DICE C.


### Getting the Amiga NDKs

You will require, at a minimum, the NDK (Native Developer Kit) for AmigaOS 2.0.

The NDKs still remain under copyright, so they cannot be included in DICE. You must source the NDKs
yourself. They have been published by Commodore and Amiga Technologies in various forms over the
years. In the later days of the Amiga, they were published on the "Amiga Developer CD" which went
through several different iterations, and earlier to that they were available on floppy disk sets.

Generally each different version of the NDK organised and presented the files differently, so it's
very difficult for the DICE build system to adapt to each known version. Instead, you will be
required to find the files yourself and organise them into a consistent form that DICE's build will
use.

Create a directory to hold the NDKs - for example, `/usr/local/share/amiga-ndk` on UNIX-like OSes,
or `DH1:AmigaNDK` on Amiga.

Within this directory, create directories for each AmigaOS version that you have the NDK for. For
example, `1.3`, `2.0`, `3.1`, etc.

Within each of those directories, create the following directories and fill with the files from the
NDK:

* `include_h` - the Amiga include/header files ending in `.h` (omit the `.i` files - or if you want
  to be thorough, place them into an `include_i` folder)
* `fd` - the AmigaOS "function definition" files - these are named like `LIBNAME_lib.fd`
* `linker_libs` - should contain at least file `amiga.lib`

Remember, you must include at least the 2.0 NDK. You can add additional versions too.

On your UNIX-like OS, set an environment variable called AMIGA_NDK_HOME that points to your
directory of NDKs. For example:

```bash
export AMIGA_NDK_HOME=/usr/local/share/amiga-ndk
```

On your Amiga, create an assign NDK: that points to your directory of NDKs. For example:

```
Assign NDK: DH1:AmigaNDK
```


### Stage 1 build on a UNIX-like OS on Intel x86/x86-64

#### Prerequisites

* A working C compiler
* GNU make
* The `mkdep` command (see: https://command-not-found.com/mkdep)

#### Procedure

TBC


### Stage 2+3 build on an Amiga

You can transfer the DICE directory onto a real Amiga, or use an emulator such as WinUAE that
supports mounting local filesystems. (Using the `vamos` emulator would be good, but it does not
currently support the needed features to build DICE. I hope to be able to add the features to
`vamos` in the non-too-distant future!)

Next, assign `DCC:` to the directory containing DICE. For example:

```
Assign DCC: DH1/DICE
```

Then, initiate the build:

```
Cd DCC:
Execute MakeAmiga-Sequence
```

This script will do the following:

1. Build a feature-complete compiler (i.e. with floating point support) and core tools, using the
   compiler and tools produced in stage 1.
1. Build a feature-complete set of the core libraries (i.e. with floating point support) using that
   newly-built compiler and tools.
1. Re-build the compiler and tools again, using the newly-built libraries, so that we have a set of
   feature-complete libraries, and a feature-complete compiler and core tools built against those
   libraries. That will be the stage 2 build completed.
1. Build the libraries again using the stage 2 artifacts, this time building the comprehensive set
   of libraries: small and large data models, stack-based and register-based OS calls, with and
   without profiling, and OS libraries for all available and recognised NDKs.
1. Build the compiler and tools again, but this time build all the supported tools instead of just
   the core set. This will be stage 3 complete, and a fully supported compiler, tools and
   libraries.


Other supported builds
----------------------

There are many potential ways to build DICE. The process above is the recommended one, but there
are some alternatives that will also be supported. Any build process not described above or in this
section is not officially supported - feel free to try it out, but beware of problems and note that
issues reported (that do not also affect the supported builds) may go unanswered.


### Bootstrap build on PC only

You can simply start the build process as described above, but stop it after the compiler and tools
have been built for PC and the libraries have been built (i.e. before it starts building Amiga
native executables).

This will be a slightly crippled build:

1. It will lack floating point support. (This can only exist when built on an Amiga.)
1. The library and tools will be the core set only, although you can proceed to manually build
   more of the library variants and tools.

What use is this? If you can live with the limitations, then this is a nice and simple
cross-compiler for the Amiga, and 68k embedded targets (if you also build the ROM library set).


### Bootstrap from a previous version of DICE on an Amiga

You may already have an Amiga with a version of DICE installed on it. That can be used as a
ready-to-go stage 1 build, and you can begin your DICE build from stage 2.

To do this, get the DICE source repository onto your Amiga, and assign `DCC:` to it. Create a
directory called `bin1` and copy your existing DICE `bin` directory contents into it. Repeat with a
new directory `dlib1` and existing DICE `dlib` directory. Then begin the build from the stage 2
section above.


Known limitations and future improvements
-----------------------------------------

The first host for the build must be a Linux or BSD PC on an Intel x86-64 CPU, due to assumptions
about byte ordering. It should be relatively easy to fix this though.

It would be nice to use the excellent [vamos tool by Christian Vogelgsang](https://github.com/cnvogelg/amitools/blob/master/docs/vamos.md),
but DICE currently relies on many OS calls not currently supported by vamos. It should be possible
to implement the missing OS calls, but I'm not ready to do this just yet. In the mean time, just
use a real Amiga, or emulate one using WinUAE or a similar tool.
