
/*
 *  DAS/LAB.H
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#define LSIZE	1024
#define LMASK	(LSIZE-1)

#define LT_LOC	0
#define LT_EXT	1
#define LT_REG	2
#define LT_INT	3

/*
 *  A label structure.	In addition to being linked via the hash table, the
 *  label may be linked to a sectional export list by XDefLink when you XDEF
 *  it.   This is handled in PASSA
 */

#define LF_XDEFFLAG 0x01	/*  exported	*/

typedef struct Label {
    struct Label *HNext;	/*  hash table next link	*/
    struct Label *XDefLink;	/*  exported labels		*/
    struct Sect  *Sect; 	/*  section label created in	*/
    struct MachCtx **MC;	/*  mach ctx label cred in (for optimization)   */
    char    *Name;		/*  ptr to name of label	*/
    short   l_Type;		/*  equ, reg, local, ext	*/
    short   l_Refs;		/*  references to the label	*/
    char    l_Reserved;
    char    l_RegNo;		/*  if only one reg, else -1 */
    short   l_Flags;
    union {
	uword	Mask;		/*  register mask		*/
	long	Value;		/*  integral value		*/
	long	Offset; 	/*  offset in section (if def), passB */
	long	Size;		/*  COMMON def			*/
    } u;
} Label;

#define l_Mask	    u.Mask
#define l_Value     u.Value
#define l_Offset    u.Offset

