
/*
 * $VER: lib/profile.h 1.0 (17.4.93)
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 *
 *  note: machine structure, fixed length
 */

#ifndef LIB_PROFILE_H
#define LIB_PROFILE_H

#define PROF_MAGIC  0x22AA

typedef struct ProfHdr {
    short   ph_Magic;
    unsigned short ph_NumIds;
    long    ph_Reserved;
    long    ph_TimeBase;
} ProfHdr;

typedef struct ProfSym {
    struct ProfSym *ps_Link;	/*  hash link		*/
    struct ProfSym *ps_SibLink; /*  different parents	*/
    struct ProfSym *ps_Parent;	/*  parent		*/
    unsigned short   ps_Size;
    unsigned short   ps_Id;
    long    ps_TimeStamp;
    long    ps_AccumTime;
    long    ps_TotalTime;
    long    ps_NumCalls;
    void    *ps_BegId;
    void    *ps_EndId;
    char    ps_FuncName[4];	/*  extended structure	*/
} ProfSym;


#endif

