/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
/*
**      $Id: loadfile.c,v 30.326 1995/12/24 06:13:05 dice Exp dice $
**
**      Load a binary file into memory.  Report the address and hole
**      until CTRL-C is pressed.  Useful for looking at binary files
**      with a debugger.
**
*/

#include <stdio.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <clib/exec_protos.h>
#ifdef AMIGA
#include <lib/version.h>
#else
#include <include/lib/version.h>
#endif

#define GUARD   100     /* Padding on either side of allocation */
#define FILL    0xaa    /* Character used to fill guard area */

#ifdef _DCC
IDENT("loadfile",".2");
DCOPYRIGHT;
#endif

void    onbreak((*fptr)());
void    exit(int);

disable_break()
{
        return(0);
}

main(int argc,char *argv[])
{
FILE *  fp;
int32_t fl;
char *  memory;
char *  filptr;
int     err;

        onbreak(disable_break);
        SetSignal(SIGBREAKF_CTRL_C,0);  /* Clear CTRL-C signal */

        if (argc != 2 || ( argc == 2  &&  *argv[1]=='?' ) )
        {
            printf("loadfile <filename>\n");
            printf(";Load binary file into memory, wait for CTRL-C\n");
            exit(5);
        }

        if( fp=fopen(argv[1],"r") )
        {
            fseek(fp,0,2);  /* Seek to end of file */
            fl=ftell(fp);
            fseek(fp,0,0);  /* Seek to start of file */
            if (memory=AllocMem(fl+GUARD+GUARD,0) )
            {
                for(filptr=memory; filptr<(memory+fl+GUARD+GUARD); filptr++)
                        *filptr=FILL;
                err=fread((memory+GUARD),1,fl,fp);
                if(err)
                {
                    printf("Loaded %ld ($%lx) bytes at location $%lx\n",
                            fl,fl,memory+GUARD);
                    fseek(fp,0,0);  /* Seek to start of file */
                    printf("Press CTRL-C to exit\n");
                    Wait(SIGBREAKF_CTRL_C);
                }
                else
                    printf("Error reading file %s, error %d\n",argv[1],err);
                FreeMem(memory,fl+GUARD+GUARD);
            }
            else
                printf("Error: out of memory\n");
            fclose( fp );
        }
        else
            printf("Could not open file %s\n",argv[1]);
}
