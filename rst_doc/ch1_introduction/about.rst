**********
About DICE
**********

DICE is a complete C Language development system written for and on
the Commodore Amiga computer. DICE includes a compiler, editor,
linker, source code management tools, help system, debugger, and
examples in short, everything you need to write programs which take
full advantage of everything the Amiga has to offer.

DICE was designed from the start with the international ANSI C
standard in mind, as well as compatibility with other Amiga C
Compiler products on the market. In addition, DICE provides a number
of extensions targeted at providing you with easy access to the
Amiga's special features.

It also supports embedded systems development for Motorola 68000-based
systems, with special options and tools for writing C code that will be
burnt to ROMs.

DICE is targeted at the general Amiga programmer with a typical
machine. It will run effectively a low-end Amiga 500 with only 1
megabyte of memory and two floppy drives, while at the same time it
will take advantage of everything an Amiga with a 68040 processor
and AGA chipset can throw at it. We will continue to address the
low-end machine as the base environment while enhancing the
performance of the compiler and increasing the number of features
available.

You can expect to see continued improvements in code generation
quality, more features in the debugger, more libraries, and more
examples. DICE will continue to evolve in step with the future
directions that Commodore is taking the Amiga product line. If you
see something that is needed, let us know.


Goals
=====

DICE was written with several goals in mind:

* Reasonably fast compilation. Modular executables for ease of use,
  reliability, and testability.

* Tight, productive integration between modules using open standards,
  such as Amiga ARexx, to allow for custom environments and free
  choice of tools.

* Provide optimization but not at the expense of reliability. DICE
  does no common sub-expression or loop unrolling optimization, but
  does do smart automatic register allocation, ``switch()`` optimizations
  and has multi-level history to propagate conditional expressions.

* Provide comprehensive support of the Amiga, especially for new
  versions of the OS that come out. DICE fully supports 1.3, 2.0 and
  3.0 with an easy extension mechanism to allow development on
  several OS versions /betas simultaneously.

* Provide source code where appropriate so modifications or fixes can
  be easily made. DICE includes full source for its support libraries
  (``c.lib``, ``m.lib``, ``auto.lib``) and a means to remake the libraries. DICE
  also includes full source to most of its own utilities to allow
  users to make enhancements.


Product Contents
================

DICE comes complete with:

Compiler
--------

This is the heart of the DICE product.  It includes DC1, DCPP, DAS,
and DCC.  Together, these control the complete stages of compiling
and linking both C and assembler source code.  DCPP may also be used
as a standalone preprocessing tool.  The compiler supports inline
calls to all Amiga Library functions through standard ``#pragma``
statements, registerized parameters, profiling code,
auto-initialization code, and pure code for residentability.

When combined with the Editor, Visual Options, and Integrated Help,
the Compiler will allow for interactive development and error
corrections.

Debugger
--------

No C compiler product is complete without a source level debugger.
As a windowing debugger, DEBUG gives you the ability to examine the
execution of your code, set break points, watch memory, and even
modify the execution.  Fully integrated with the Amiga, it supports
ARexx, cut and paste, mouse clicks, and all Amiga screen modes.

Linker
------

Compatible with standard Amiga Object files, DLINK is a fast linker.
It allows control of data placement and will combine code to provide
the powerful auto-initialization capabilities of DICE.

Profiler
--------

Once you have a program running, DPROF gives you the ability to
identify hot spots in the code to allow you to improve algorithms or
even recode critical portions of the code.  You can control the level
of code that you wish to profile through a series of compiler and
linker options.

Assembler
---------

DICE includes two assemblers.  DAS is a low overhead assembler which
is used by the compiler because of its speed.  When you wish to do
advanced Amiga programming, you may also use A68k which includes
powerful macros and complete integration with C calling conventions.

Online Help
-----------

In order to speed the development process, DiceHelp allows immediate
access to all DICE and Amiga subroutines with just a single
keystroke.  In addition, a C language reference and all error
messages are immediately accessible.  DiceHelp works with your
favorite editor.

VMake and VOpts
---------------

In order to speed development, DICE includes a set of visual
development tools that allow you to quickly edit, compile, link, and
debug your project without having to resort to the CLI or continually
type commands to invoke the compiler.  VMake eliminates the tedium of
maintaining a Makefile by letting you just enter the files for the
project and tell it the type of project to build.  It even will read
and write Makefiles so that you can resort to the CLI if you feel
more comfortable.

Within VMake, you can edit files by just double clicking on them. If
they happened to be stored in RCS, VMake will automatically check
them out.  Checking them back into an archive is as simple as
selecting a menu item.  You may also invoke the debugger on your
application, and test it out without having to exit the VMake
environment.

Of course, it is not complete without the ability to visually set the
compiler options that your project demands.  VOpts talks directly to
VMake to manage the options for compiling your project.

Development Tools
-----------------

Because writing a program on the Amiga requires more than just a
compiler and a linker, DICE provides a wide range of tools for
manipulating the environment. This includes text tools like ``Cat``,
``Dsearch``, ``Head``, ``Touch``, ``WC``, and ``IStrip``.

There are also tools for manipulating object files and libraries such
as ``DObj``, ``FDToLib``, ``FDToPragma``, ``LbMake``, and ``LibToS``. ``MakeProto`` will
automatically collect prototypes into a common header file. ``DiceCache``
allows control over caching of files to speed up the development
environment. Lastly, for dealing with groups of files we have ``Du``,
``DUpdate``, and ``LHarc``.

Make Utility
------------
While not identical to the standard Unixtm make utility, DMake offers
the power to control putting together a project without having to
retype commands each time.  With a built-in macro facility and rule
based decision engine, DMake will only recompile those pieces of a
project that have changed. DMake files are easy to create, consistent
and powerful.

Editor
------

Because not everyone has an editor that they may be happy with, DME
is provided to permit easy editing of source code and other text
files.  It includes complete Amiga integration with menus, ARexx,
mouse support, programmable keys, and multiple-window support.  Use
of DME is not required; DICE will integrate easily with virtually any
Amiga editor

Romable Code Generation Tools
-----------------------------

In addition to generating Amiga executables, DICE allows generation
of code for any 68000 based platform.  ``Romable`` and ``BinToHex`` combined
with a number of DCC options allow generation of romable code and
standard Motorola S records.

Workbench Support Tools
-----------------------

For those people who wish to work from Workbench instead of the CLI,
DICE includes VOpts for setting compiler options visually, and Vmake
for controlling a project build visually.  Although they work with
Workbench, the CLI user may also use them to manage a project
interactively.

Include Files
-------------

Because people wish to develop source code that works under all
versions of the Amiga Operating System, DICE supports several sets of
header files for different AmigaOS versions. You can choose which of these includes to reference
based on an environment variable or compiler option.

Libraries
---------

This is one area where DICE really shines.  Included are libraries
which provide ANSI, Unixtm, and general Amiga compatibility.  These
libraries are provided in a number of models allowing for linking
with just about any normal program. However there are cases where
something fancy needs to be done, or a special combination of library
switches is used for which a library hasn't been supplied. For this,
we also provide the complete source code to the libraries along with
the configuration scripts to build them.

Example Source Code
-------------------

DICE also comes with example source code to show you how to program
on the Amiga.

VOPTS
^^^^^

The Vopts utility shipped with DICE allows customizing of the
compiler environment.  It illustrates everything necessary for
programming gadgets, menus, keyboard input, file i/o, and even
internationalization on the Amiga.  It also illustrates how to write
a program which takes advantage of both 1.3 and 2.0 capabilities.
One special bonus with this example is the ability to allow the user
to customize the gadget presentation of the program without having to
change the source code.  In this case, the program read the gadgets
from a file and dynamically lays them out based on the screen
resolution, current font, and color choices.

Shared_Lib
^^^^^^^^^^

One of the key components of the Amiga, libraries are the method of
adding dynamic linked routines to the system.  This example includes
everything necessary to build a library, add it to the system and to
provide some simple services.  This particular library allows saving
and retrieving a simple string. It also shows how to write a program
using the newly created library.

Printer_Driver
^^^^^^^^^^^^^^

A sample printer driver that compiles and runs.  This includes
support for graphics, font selection, and extended attributes.  It
can be readily modified for a printer that you might not have a
printer driver for.

Exec_Dev
^^^^^^^^

An example Exec device which emulates a disk level device as a single
file on a hard disk.  This shows how to handle the basic
init/termination and read/write functions. As a tool, it is extremely
useful for emulating a second (or multiple) floppy disk on a system
with a hard disk.

DosHan
^^^^^^

This is a simple AmigaDOS handler which supports basic
open/close/read/write packets. It can be used as the basis for a more
complex handler.  It also includes instructions for adding the
handler to the system.

DCC
^^^
This is the complete source for the DCC front end to the compiler.

cat/count/wc
^^^^^^^^^^^^

These simple programs illustrate basic C programming with command
line arguments and simple output.  They parallel examples found in
many C programming books.

hello.c
^^^^^^^

Of course, no compiler product would be complete without the
definitive source code to the standard Hello world program.



Section heading
===============

Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur ut velit lorem. Duis sed dui commodo, pulvinar dui vitae, efficitur arcu. Ut fringilla justo at varius blandit. Donec quis lectus iaculis, convallis tortor ut, sagittis massa. Cras iaculis nulla ut lectus sodales, a lobortis enim hendrerit. Morbi et nibh sodales, sodales metus ut, sodales lorem. Sed ullamcorper massa commodo nibh suscipit vulputate. Aliquam egestas orci a lectus cursus ullamcorper. Integer aliquet, ante ac sagittis scelerisque, augue lorem accumsan nulla, posuere tristique nunc dui non tortor. Cras volutpat metus lorem, non pellentesque tellus feugiat ac. Curabitur imperdiet, neque eget tincidunt pretium, felis ante porttitor dui, eget hendrerit sapien nibh nec nibh. Vestibulum nec congue quam, nec finibus nisi. Suspendisse at neque pharetra, fermentum quam non, rutrum lacus. Duis massa felis, interdum eget porttitor et, vestibulum a odio. Vivamus enim orci, porttitor hendrerit risus tempus, elementum sollicitudin leo.

Subsection heading
------------------

Sed lacinia varius ante vitae cursus. Sed leo arcu, sagittis quis ex in, commodo sagittis tortor. Donec et magna magna. Ut dapibus, ante eu lobortis ullamcorper, quam nunc congue mauris, eget dapibus nunc ligula a magna. Vestibulum lacinia rutrum libero a porttitor. In congue arcu egestas, interdum diam ullamcorper, vestibulum lorem. Aliquam interdum massa lacus, eget malesuada neque pulvinar ac. Duis auctor est sit amet eleifend commodo. Morbi nec hendrerit dolor, et egestas lorem.

Subsubsection heading
^^^^^^^^^^^^^^^^^^^^^

Pellentesque auctor elementum faucibus. Suspendisse accumsan vulputate magna, sed pellentesque turpis pharetra volutpat. Integer ac sem sapien. Suspendisse potenti. In pulvinar pretium urna, ac tempor enim accumsan at. Sed et enim placerat, venenatis ipsum ut, lacinia leo. Aenean rutrum, purus eget congue pharetra, mi metus eleifend quam, et interdum nisi nibh sed tellus. Nunc vitae enim a massa laoreet tempus a eget sem. In ut congue eros, fermentum sagittis urna. Aliquam ullamcorper aliquam tincidunt. Ut id commodo ipsum. Nullam luctus ultricies turpis, eget dignissim ligula efficitur vel.

Paragraph heading
"""""""""""""""""

Aenean ut libero lobortis, dapibus sapien a, pretium risus. Donec posuere erat velit, sed ullamcorper nunc egestas vitae. Donec imperdiet efficitur elit vitae consequat. Proin a venenatis tortor, ac ultrices augue. Proin a sollicitudin turpis. Nunc porta risus justo, non volutpat justo mollis efficitur. Vivamus eget viverra nisi, vitae semper mauris. Nunc egestas, eros faucibus maximus fermentum, turpis nunc interdum ligula, in dapibus felis urna a elit. Integer massa purus, fringilla sed orci tempor, facilisis tempor mi. Suspendisse molestie urna nec arcu aliquam, non bibendum tortor imperdiet. Ut ornare id arcu at viverra. Phasellus tempor sapien eros, eu maximus risus gravida sed. 
