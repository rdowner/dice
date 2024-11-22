Building DICE
=============

Recommended build path
----------------------

Here we describe the recommended build path. There are other ways to build DICE, but this is
path that is most likely to work. (Some of those other ways may be supported paths in future.)

This build path has three stages:

**Stage 1** takes place on a UNIX-like OS, such as Linux. It requires the OS standard C compiler
and related developer tools, and will bootstrap a basic version of DICE C.

**Stage 2** takes place on an Amiga. It takes the stage 1 output and repeats the build process,
this time using AmigaOS and DICE itself to build another basic version of DICE C.

**Stage 3** repeats stage 2, but this time builds the full suite of tools and libraries, and adds
the documentation, examples, etc. to make a full Amiga binary distribution of DICE C.


### Getting the Amiga NDK

You will require the Amiga NDK (Native Developer Kit) 3.2.

The NDKs still remain under copyright, so they cannot be included in DICE. You must source the NDK
yourself. NDK 3.2 is available with the purchase of AmigaOS 3.2 from Hyperion Entertainment.

### Stage 1 build on a UNIX-like OS on Intel x86/x86-64

#### Prerequisites

* A working C compiler
* GNU make
* The `mkdep` command (see: https://command-not-found.com/mkdep)
* AmigaOS NDK 3.2

#### Install essential files from the NDK

Execute this command:

```bash
./bootstrap/install-ndk.sh <LOCATION OF NDK3.2 FILES>
```

#### Build procedure

Execute this command:

```bash
make stage1
```


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


Known limitations and future improvements
-----------------------------------------

The first host for the build must be a Linux or BSD PC on an Intel x86-64 CPU, due to assumptions
about byte ordering. It should be relatively easy to fix this though.

It would be nice to use the excellent [vamos tool by Christian Vogelgsang](https://github.com/cnvogelg/amitools/blob/master/docs/vamos.md),
but DICE currently relies on many OS calls not currently supported by vamos. It should be possible
to implement the missing OS calls, but I'm not ready to do this just yet. In the mean time, just
use a real Amiga, or emulate one using WinUAE or a similar tool.
