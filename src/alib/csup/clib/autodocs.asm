
******* amiga.lib/AddTOF *****************************************************
*
*   NAME
*       AddTOF - add a task to the VBlank interrupt server chain.
*
*   SYNOPSIS
*       AddTOF(i,p,a);
*
*       VOID AddTOF(struct Isrvstr *, APTR, APTR);
*
*   FUNCTION
*       Adds a task to the vertical-blanking interval interrupt server
*       chain. This prevents C programmers from needing to write an
*       assembly language stub to do this function.
*
*   INPUTS
*       i - pointer to an initialized Isrvstr structure
*       p - pointer to the C-code routine that this server is to call each
*           time TOF happens
*       a - pointer to the first longword in an array of longwords that
*           is to be used as the arguments passed to your routine
*           pointed to by p.
*
*   SEE ALSO
*       RemTOF(), <graphics/graphint.h>
*
******************************************************************************


******* amiga.lib/RemTOF *****************************************************
*
*   NAME
*       RemTOF - remove a task from the VBlank interrupt server chain.
*
*   SYNOPSIS
*       RemTOF(i);
*
*       VOID RemTOF(struct Isrvstr *);
*
*   FUNCTION
*       Removes a task from the vertical-blanking interval interrupt server
*       chain.
*
*   INPUTS
*       i - pointer to an Isrvstr structure
*
*   SEE ALSO
*       AddTOF(), <graphics/graphint.h>
*
******************************************************************************


******* amiga.lib/FastRand ***************************************************
*
*   NAME
*       FastRand - quickly generate a somewhat random integer
*
*   SYNOPSIS
*       number = FastRand(seed);
*
*       ULONG FastRand(ULONG);
*
*   FUNCTION
*       Seed value is taken from stack, shifted left one position,
*       exclusive-or'ed with hex value $1D872B41 and returned.
*
*   INPUTS
*       seed - a 32-bit integer
*
*   RESULT
*       number - new random seed, a 32-bit value
*
*   SEE ALSO
*       RangeRand()
*
******************************************************************************


******* amiga.lib/RangeRand **************************************************
*
*   NAME
*       RangeRand - generate a random number within a specific integer range
*
*   SYNOPSIS
*       number = RangeRand(maxValue);
*
*       UWORD RangeRand(UWORD);
*
*   FUNCTION
*       RangeRand() accepts a value from 0 to 65535, and returns a value
*       within that range.
*
*       maxValue is passed on stack as a 32-bit integer but used as though
*       it is only a 16-bit integer. Variable named RangeSeed is available
*       beginning with V33 that contains the global seed value passed from
*       call to call and thus can be changed in a program by declaring:
*
*         extern ULONG RangeSeed;
*
*   INPUTS
*       maxValue - the returned random number will be in the range
*                  [0..maxValue-1]
*
*   RESULT
*       number - pseudo random number in the range of [0..maxValue-1].
*
*   SEE ALSO
*       FastRand()
*
******************************************************************************


******* amiga.lib/printf *****************************************************
*
*   NAME
*       printf - print a formatted output line to the standard output.
*
*   SYNOPSIS
*       printf(formatstring [,value [,values] ] );
*
*   FUNCTION
*       Format the output in accordance with specifications in the format
*       string.
*
*   INPUTS
*       formatString - a C-language-like NULL-terminated format string,
*                      with the following supported % options:
*
*         %[flags][width][.limit][length]type
*
*           $     - must follow the arg_pos value, if specified
*         flags   - only one allowed. '-' specifies left justification.
*         width   - field width. If the first character is a '0', the
*                   field is padded with leading 0s.
*           .     - must precede the field width value, if specified
*         limit   - maximum number of characters to output from a string.
*                   (only valid for %s or %b).
*         length  - size of input data defaults to word (16-bit) for types c,
*                   d, u and x, 'l' changes this to long (32-bit).
*         type    - supported types are:
*                         b - BSTR, data is 32-bit BPTR to byte count followed
*                             by a byte string. A NULL BPTR is treated as an
*                             empty string. (V36)
*                         d - signed decimal
*                         u - unsigned decimal
*                         x - hexadecimal with hex digits in uppercase
*                         X - hexadecimal with hex digits in lowercase
*                         s - string, a 32-bit pointer to a NULL-terminated
*                             byte string. A NULL pointer is treated
*                             as an empty string.
*                         c - character
*
*       value(s) - numeric variables or addresses of null-terminated strings
*                  to be added to the format information.
*
*   NOTE
*       The global "_stdout" must be defined, and contain a pointer to
*       a legal AmigaDOS file handle. Using the standard Amiga startup
*       module sets this up. In other cases you will need to define
*       stdout, and assign it to some reasonable value (like what the
*       dos.library/Output() call returns). This code would set it up:
*
*               ULONG stdout;
*               stdout=Output();
*
*   BUGS
*       This function will crash if the resulting stream after
*       parameter substitution is longer than 140 bytes.
*
******************************************************************************


******* amiga.lib/sprintf ****************************************************
*
*   NAME
*       sprintf - format a C-like string into a string buffer.
*
*   SYNOPSIS
*       sprintf(destination formatstring [,value [, values] ] );
*
*   FUNCTION
*       Performs string formatting identical to printf, but directs the output
*       into a specific destination in memory. This uses the ROM version
*       of printf (exec.library/RawDoFmt()), so it is very small.
*
*       Assembly programmers can call this by placing values on the
*       stack, followed by a pointer to the formatstring, followed
*       by a pointer to the destination string.
*
*   INPUTS
*       destination - the address of an area in memory into which the
*                     formatted output is to be placed.
*       formatstring - pointer to a null terminated string describing the
*                      desired output formatting (see printf() for a
*                      description of this string).
*       value(s) - numeric information to be formatted into the output
*                  stream.
*
*   SEE ALSO
*        printf(), exec.library/RawDoFmt()
*
******************************************************************************


******* amiga.lib/stdio ******************************************************
*
*   NAMES
*       fclose  - close a file
*       fgetc   - get a character from a file
*       fprintf - format data to file (see printf())
*       fputc   - put character to file
*       fputs   - write string to file
*       getchar - get a character from stdin
*       printf  - put format data to stdout (see exec.library/RawDoFmt)
*       putchar - put character to stdout
*       puts    - put string to stdout, followed by newline
*
*   FUNCTION
*       These functions work much like the standard C functions of the same
*       names. The file I/O functions all use non-buffered AmigaDOS
*       files, and must not be mixed with the file I/O of any C
*       compiler. The names of these functions match those found in many
*       standard C libraries, when a name conflict occurs, the function is
*       generally taken from the FIRST library that was specified on the
*       linker's command line.  Thus to use these functions, specify
*       the amiga.lib library first.
*
*       To get a suitable AmigaDOS FileHandle, the dos.library/Open() or
*       dos.library/Output() functions must be used.
*
*       All of the functions that write to stdout expect an appropriate
*       FileHandle to have been set up ahead of time. Depending on
*       your C compiler and options, this may have been done by the
*       startup code.  Or it can be done manually
*
*       From C:
*               extern ULONG stdout;
*               /* Remove the extern if startup code did not define stdout */
*               stdout=Output();
*
*       From assembly:
*               XDEF    _stdout
*               DC.L    _stdout ;<- Place result of dos.library/Output() here.
*
******************************************************************************

