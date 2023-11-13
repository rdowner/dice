******* amiga.lib/ACrypt *************************************************
*
*   NAME
*       ACrypt -- Encrypt a password (V37)
*
*   SYNOPSIS
*       newpass = ACcrypt( buffer, password, username )
*
*       STRPTR ACrypt( STRPTR, STRPTR, STRPTR);
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
*   NOTES
*       This function first appeared in later V39 versions of amiga.lib,
*       but works under V37 and up.
*
************************************************************************
