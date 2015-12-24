/* path.c */

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dosasl.h>


/*
 #include <clib/exec_protos.h>
 #include <pragmas/exec_old_pragmas.h>
 #include <clib/dos_protos.h>
 #include <pragmas/dos_pragmas.h>

*/


extern LONG DosNameFromAnchor(struct AnchorPath *anchor, char *buffer, LONG buflen);
extern LONG SetIoErr( long result );
extern BOOL AddPart( STRPTR dirname, STRPTR filename, unsigned long size );

extern struct DosLibrary *DOSBase;

LONG
NameFromAnchor (anchor, buffer, buflen)
	struct AnchorPath *anchor;
	char *buffer;
	LONG buflen;
{
	struct AChain *chain;

	/* override if bit is set */
	if (DOSBase->dl_Root->rn_Flags & RNF_PRIVATE1)
		return DosNameFromAnchor(anchor,buffer,buflen);

	if (buflen == 0)
	{
		SetIoErr(ERROR_LINE_TOO_LONG);
		return FALSE;
	}

	*buffer = '\0';

	for (chain = anchor->ap_Base;
	     chain != NULL;
	     chain = chain->an_Child)
	{
		/* Must check if the node's a pattern node! */
		if (!AddPart(buffer,
			     chain->an_Flags & DDF_PatternBit ?
					&(chain->an_Info.fib_FileName[0]) :
					&chain->an_String[0], 
				buflen))
		{
			return FALSE;	/* IoErr already set */
		}
	}

	return DOSTRUE;
}

