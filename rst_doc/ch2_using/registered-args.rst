####################
Registered Arguments
####################

Convention in the C language is to pass function arguments on the stack.
However on architectures with large numbers of CPU registers - which includes
the Motorola 68000 series used on the Amiga - passing function arguments in
registers is faster and more efficient. DICE fully supports this.

Using registerized arguments will make your code smaller and faster,
but there are some restrictions and various other things you must be
sure to do before you can use the feature reliably.

How it works
============

When you compile a program with DICE without any options to enable registered
arguments, each function in your program is prefixed with an ``_`` underscore
symbol. For example, the ``main`` function in C will actually be named
``_main`` in the object code produced by the compiler. These functions expect
to be called with their arguments on the stack.

When registerized arguments are enabled, the compiler will generate symbols
prefixed with ``@`` instead of ``_``. So our ``main`` function will this time
be called ``@main``. These functions expect to be called with their arguments
in registers.

For DICE to generate a registerized function, the following must all be
true:

* The compiler must be run the with ``-mr`` option.
* The function must be prototyped.
* The function must have between 1 and 4 parameters.
* The function must not be defined with ``__stkargs``.

The compiler will generate a registerized function if all of these conditions
are met.

Functions with no parameters behave identically in both registerized and
stack-argument form, so in this case DICE will fall back to the stack-argument
style with ``_`` prefix. DICE does not currently support more than four
registerized arguments, so in this case DICE falls back to the stack-argument
style again.

It is important that ``main()`` is defined correctly. The registerized startup
code will expect main to have a prototype of ``int main(int argc, char **argv)``.
It is important that you define your main function in this way, even if your
program does not need ``argc`` and/or ``argv``.

.. code-block:: c

    int main(int argc, char **argv)
    {
        /* ... */
        return 0;
    }


Making sure you have prototypes
===============================

All functions must be prototyped - this is how DICE determines what registers
to put things into. The prototypes must match the actual function declarations,
otherwise arguments may be placed into the wrong registers at runtime.

When making Amiga library calls, such as ``Open()``, ``FindTask()``,
``OpenWindow()``, etc., you must include the appropriate prototype file. For
example:

.. code-block:: c

    #include <clib/intuition_protos.h>

For ``c.lib`` functions, again the appropriate ``#include`` directives must be
made, such as ``<stdio.h>``, ``<stdlib.h>``, ``<fcntl.h>``, and ``<string.h>``.

DICE can check that all function calls are made with prototypes by passing the
``-proto`` option. If you do this, DICE will warn you if you are calling
functions without prototypes. In any case, it is a good idea to always use
prototypes for all functions, so we recommend always using the ``-proto``
option.


Enabling and disabling registered arguments
===========================================

To enable registered arguments, pass the ``-mr`` option to ``dcc``. Note that
registered arguments affects both the compilation *and* linking phases of the
build, so if you are compiling C files and linking them in separate steps, the
``-mr`` option must be passed to both.

If you need to explicitly disable registered arguments for some reason (for
example, ``-mr`` is in your ``DCCOPTS`` environment variable and you want to
disable it for a particular build), pass the ``-mr0`` option to ``dcc``.


Forcing a function to use stack-based arguments
===============================================

To override the ``-mr`` option on a function-by-funtion basis, you can use the
``__stkargs`` keyword. Apply this keyword to the function prototype and to the
function definition.

For example:

.. code-block:: c

    extern __stkargs void fubar(char *);

    __stkargs void
    fubar(char *ptr)
    {
        ...
    }

Remember that **both the prototype and the function definition** must
specify the ``__stkargs`` flag. Failure to do this might cause DICE to generate
a registered call to a stack-based function, or vice versa, which will cause
failures during the link stage of a compile.

Function callbacks
==================

Functions with registered arguments can have some unexpected behaviour when
function pointers are passed to library functions, with the expectation that
the library will later call the function pointer, known as ``callbacks``.

Callbacks in the standard C library (c.lib)
-------------------------------------------

When linking with the ``-mr`` option, DICE will link in ``cr.lib`` instead of
``c.lib``. ``cr.lib`` is a modified version of ``c.lib`` that expects to use
registerized functions, including for callbacks. Therefore any function pointer
that you pass to a function in the standard C library must be for a
registerized function. If you pass a function pointer to a function which is
not registerized (see the list of criteria under :ref:`How it works`), then
the code will not work and will likely caused a crash.

        AMIGA.LIB works the flip-side.  Since these are commodore-standard
        routines any call-backs will be using stack-based arguments.  Thus,
        any procedure you pass to an AMIGA.LIB routine must be declared
        __stkargs.  This is a special case:


            int ben(int (*)(int));

            __stkargs int fubar(int x)
            {
                ben(fubar);
            }

        Even though ben() is fully prototyped to take a function pointer
        that is fully prototyped, since fubar has been declared as a
        stack-only routine, _fubar will be passed to ben instead of
        @fubar.

        Thus, Amiga.Lib routines may be fully prototyped but as long as
        you pass a stack-args only routine to them, the stack-args entry
        point will be passed instead of the reg-args entry point.


    FORCING STACK BASED INDIRECT FUNCTION POINTERS

        Lets go back to that structure.. lets say you want any procedures
        called through the indirect function pointer to be called using
        stack based arguments.  You would then specify:

            struct entry {
                char *funcName;
                __stkargs void (*funcPtr)(char *);
            } *En;

        But BE CAREFUL.  Any function assigned to this structure entry
        must also be __stkargs:

            extern __stkargs void fubar(char *);    /*  RIGHT   */
            extern void fubar(char *);              /*  WRONG   */

                ...

            En->funcPtr = fubar;

        If you do NOT match the argument type properly, DICE will
        generate incorrect code.  Again, this is when you use the -mRR
        option.  If you use only -mr or -mR (and specify __stkargs for
        the procedures in question), this kind of problem will not
        crop up.

    RESTRICTIONS WITH 1.3

        No standard prototype headers exist for the Amiga includes under
        1.3.  Since REGARGS depends on the programmer using the same
        prototypes as were used to generate the regargs library, no
        regargs library (would be amigasr.lib) exists for 1.3

        Under 2.0, the prototypes do exist in the directory
        CLIB/#?_protos.h , and a 2.0 version of amiga.lib does
        exist (AMIGASR20.LIB).

        Thus, under 1.3 you cannot use REGARGS for system calls, though
        you can use it for all the functions within your program.  To
        prevent DICE from attempting to make regargs system calls (if
        this occurs you will get undefined symbol errors in link showing
        symbols that begin with an '@'), you must either not prototype
        the arguments to the system calls, or prototype them with
        __stkargs.  Example:

            void *AllocMem();                           /*  OK  */
            __stkargs void *AllocMem(long, long)        /*  OK  */

            void *AllocMem(long, long)                  /*  WRONG */

        The second method is the one of choise (using __stkargs) because it
        allows you to do strict prototype checking (-proto option to DCC)
        and mix non-registered amiga.lib calls with registered calls

    RESTRICTIONS IN AMIGASR20.LIB, (GENERAL)

        The registerized version of amiga.lib does not contain all
        amiga.lib functions.  Of major import: CreateTask() and
        DeleteTask() are missing.  The single precision FFP routines
        do not exist either.

How parameters are assigned to registers
========================================

DICE will use registers D0, D1, A0 and A1 for registered argument functions.
DICE will start by assigning non-pointer parameters to D0 and D1, and pointer
arguments to A0 and A1. If there are more than two non-pointer parameters, DICE
will also use A0 and A1; likewise, if there are more than two pointer
parameters, DICE will use D0 and D1.

The examples below illustrate this in practice:

.. code-block:: c

    extern int one_data_argument_function(int a);
    // registers used:                       D0

    extern int one_pointer_argument_function(int *a);
    // registers used:                           A0

    extern int three_data_argument_function(int a, int b, int c);
    // registers used:                         D0     D1     A0

    extern int three_pointer_argument_function(int *a, int *b, int *c);
    // registers used:                             A0      A1      D0

    extern int two_data_and_two_pointer_function(int a, int b, int *c, int *d);
    // registers used:                              D0     D1      A0      A1

    extern int three_data_and_one_pointer_function(int a, int b, int c, int *d);
    // registers used:                                D0     D1     A0      A1

However, you can manually change the register assignments, and make your own
choices about which registers are assigned to each parameter. This will allow
you to use registers D2-D7 and A2-A7.

To manually assign registers, annotate the parameters with keywords ``__D0``,
``__D1``, ``__D2``, etc. up to ``__D7``, and likewise, ``__A0``, ``__A1``,
etc. You must make the register assignment on both the prototype and the
definition of the function.

.. warning::

    A number of address registers have special meaning to the 68000 CPU and
    to DICE. DICE uses A4 as the data segment pointer; AmigaOS uses A6 for
    library calls; the 68000 uses A7 as the stack pointer. Do not attempt to
    use these registers in normal Amiga programs. Only use them in special
    cases where you know what you are doing.

Registered functions without the ``-mr`` option
===============================================

There are some cases where DICE can be made to generate registered functions,
**even if the ``-mr`` option is not used**.

This will happen when:

* A function is declared with the ``__regargs`` keyword. DICE will generate the
  function with registered arguments, even if the ``-mr`` option is not used.
* A function is declared with its parameters have explicit register keywords
  (e.g. ``__D0``, ``__A0``, etc.). This will implicitly annotate the function
  itself with the ``__regargs`` keyword.
