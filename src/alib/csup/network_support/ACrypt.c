/*==========================================================
 * ACrypt.c
 *
 * password encyption function (used by AS225 & Envoy)
 *
 * $Locker:  $
 *
 * $Id: ACrypt.c,v 1.1.1.1 1998/10/10 06:23:35 dillon Exp $
 *
 * $Header: /home/ncvs/dice/src/alib/csup/network_support/ACrypt.c,v 1.1.1.1 1998/10/10 06:23:35 dillon Exp $
 *
 * $Log: ACrypt.c,v $
 * Revision 1.1.1.1  1998/10/10 06:23:35  dillon
 *
 *
 *     Initial import of DICE into CVS repository.  Logs for the thousands of
 *     changes prior to this import have essentially been lost   -Matt.
 *
 * Revision 30.8  1994/08/18  05:54:21  dice
 * .
 *
 * Revision 30.0  1994/06/10  18:10:31  dice
 * .
 *
 * Revision 30.0  1994/06/10  18:10:31  dice
 * .
 *
 * Revision 1.2  93/01/11  14:29:51  bj
 * 1.1 used the name 'crypt'. New version uses 'ACrypt'. No other changes.
 * 
 * Revision 1.1  93/01/07  14:25:14  bj
 * Initial revision
 * 
 * 
 *
 * =========================================================
 */

/*
******* amiga.lib/ACrypt *************************************************
*
*   NAME
*       ACrypt -- Encrypt a password
*
*   SYNOPSIS
*       newpass = ACrypt( buffer, password, username )
*
*       UBYTE *ACrypt( UBYTE *, UBYTE *, UBYTE *) ;
*
*   FUNCTION
*       This function takes a buffer of at least 12 characters in length,
*       an unencrypted password and the user's name (as known to the host
*       system) and returns an encrypted password in the passed buffer. 
*       This is a one-way encryption. Normally, the user's encrypted 
*       password is stored in a file for future password comparison.
*
*   INPUTS
*       buffer     - a pointer to a buffer at least 12 bytes in length.
*       password   - a pointer to an unencrypted password string.
*       username   - a pointer to the user's name.
*
*   RESULT
*       newpass    - a pointer to the passed buffer if successful, NULL
*                    upon failure. The encrypted password placed in the
*                    buffer will be be eleven (11) characters in length
*                    and will be NULL-terminated.
*
*   EXAMPLE
*
*       UBYTE *pw, *getpassword() ;
*       UBYTE *user = "alf"
*       UBYTE *newpass ;
*       UBYTE buffer[16] ;         \* size >= 12 *\
*
*       pw = getpassword() ;   \* your own function *\
*
*       if((newpass = ACrypt(buffer, pw, user)) != NULL)
*       {
*               printf("pw = %s\n", newpass) ; \* newpass = &buffer[0] *\
*       }
*       else
*       {
*               printf("ACrypt failed\n") ;
*       }
*
************************************************************************
*/
  

#include <exec/types.h>
#include <dos/dos.h>
#include <libraries/dos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <utility/name.h>
#include <clib/utility_protos.h>
#include <pragmas/utility_pragmas.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

// extern struct Library *OpenLibrary() ;
// extern ULONG UDivMod32() ;
extern ULONG getreg(int);

extern struct ExecBase *SysBase ;

#define OSIZE 12

UBYTE *ACrypt( UBYTE *buffer, UBYTE *password, UBYTE *user )
{
        struct Library *UtilityBase ;
        int i ;
        int k ;
        long d1 ;
        unsigned int buf[OSIZE];

        if(UtilityBase = OpenLibrary("utility.library", 37L))
        {        
                for(i = 0; i < OSIZE; i++)
                {
                        buf[i] = 'A' + (*password? *password++:i) + (*user? *user++:i);
                }

                for(i = 0; i < OSIZE; i++)
                {
                        for(k = 0; k < OSIZE; k++)
                        {                           
                                buf[i] += buf[OSIZE-k-1];
                                UDivMod32((long)buf[i], 53L) ;
                                d1 = getreg(1) ;
                                buf[i] = (unsigned int)d1 ;
                        }
                        buffer[i] = buf[i] + 'A' ;
                }
                buffer[OSIZE-1] = 0;
                CloseLibrary(UtilityBase) ;
                return(buffer) ;
        }
        return(NULL) ;
}


