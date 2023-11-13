
/*
 *  DC1/TYPES.H
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  note: low level type bits reflect token ids.
 *
 *  typequalmask : those qualifiers that should be propogated in types
 *  storqualmask : those qualifiers that should be propogated in var->Flags
 *
 *  Some qualifiers will propogate in both
 *
 *  TF_TYPEQUALMASK:    initial separation (in CompType()).  storage flags
 *                      and any related to flag propogation for procedures
 *                      are weeded out.
 *
 *                      note: these are the only items kept for proc-arg
 *                      declarators
 *
 *  TF_STORQUALMASK:    represents qualifiers that have nothing to do with
 *                      the type, but everything to do with the storage
 *                      class of the variable/procedure
 *
 *  TF_KEEPPROC:        when one procedure def overrides another, keep
 *
 *  TF_STORNOTLOCAL:    illegal within a procedure
 *
 *  TF_NOTINTYPE:       TF's that should not be placed in the type structure
 *
 *
 *  XXX things like __geta4 & __stkargs can be both storage qualifiers and
 *      type qualifiers.  Confusion!
 */

#define TF_TYPEQUALMASK (TF_SIGNED|TF_UNSIGNED|TF_CONST|TF_VOLATILE|TF_UNALIGNED)
#define TF_STORQUALMASK (TF_STATIC|TF_AUTO|TF_EXTERN|TF_AUTOINIT|TF_AUTOEXIT|TF_INTERRUPT|\
                         TF_NOPROF|TF_GETA4|TF_REGISTER|TF_CHIP|TF_NEAR|TF_FAR|TF_ALIGNED|TF_CONFIG|TF_SHARED|TF_STKCALL|TF_REGCALL|TF_LIBCALL|TF_STKCHECK|TF_NOSTKCHECK|TF_DOTDOTDOT|TF_PROTOTYPE|TF_DYNAMIC)
#define TF_COMPAREQUALS (TF_ALIGNED|TF_SIGNED|TF_UNSIGNED|TF_DOTDOTDOT|TF_VOLATILE|TF_CONST|TF_NEAR|TF_FAR|TF_CHIP|TF_SHARED|TF_CONFIG|TF_UNALIGNED|TF_REGCALL|TF_STKCALL|TF_LIBCALL|TF_STKCHECK|TF_NOSTKCHECK|TF_PROTOTYPE)

#define TF_STORNOTLOCAL (TF_STATIC|TF_EXTERN)
#define TF_AUTOILLEGAL  (TF_AUTOINIT|TF_AUTOEXIT|TF_NOPROF|TF_INTERRUPT|TF_GETA4|TF_CHIP|TF_NEAR|TF_FAR|TF_CONFIG|TF_SHARED|TF_STKCALL|TF_REGCALL|TF_LIBCALL|TF_STKCHECK|TF_NOSTKCHECK)
#define TF_AUTOPROCOK   (TF_NOPROF|TF_INTERRUPT|TF_GETA4|TF_STKCALL|TF_REGCALL|TF_LIBCALL|TF_STKCHECK|TF_NOSTKCHECK)
#define TF_NOTINTYPE    (TF_STATIC|TF_EXTERN|TF_AUTO|TF_REGISTER)
#define TF_KEEPPROC     (TF_STORQUALMASK & ~TF_EXTERN)

#define TF_SIGNED       0x00000001
#define TF_UNSIGNED     0x00000002
#define TF_DOTDOTDOT    0x00000004      /*  for TID_PROC    */
#define TF_VOLATILE     0x00000008
#define TF_CONST        0x00000010
#define TF_STATIC       0x00000020
#define TF_AUTO         0x00000040
#define TF_EXTERN       0x00000080
#define TF_REGISTER     0x00000100
#define TF_AUTOINIT     0x00000200
#define TF_INTERRUPT    0x00000400
#define TF_NEAR         0x00000800
#define TF_FAR          0x00001000
#define TF_CHIP         0x00002000
#define TF_AUTOEXIT     0x00004000
#define TF_ALIGNED      0x00008000
#define TF_GETA4        0x00010000
#define TF_SHARED       0x00020000
#define TF_CONFIG       0x00040000
#define TF_UNALIGNED    0x00080000
#define TF_REGCALL      0x01000000
#define TF_STKCALL      0x02000000
#define TF_STKCHECK     0x04000000
#define TF_NOSTKCHECK   0x08000000

#define VF_ARG          0x00100000    /*  procedure argument      */
#define VF_ADDR         0x00200000    /*  address of var taken (or force out of reg)  */
#define VF_DECLD        0x00400000    /*  queued for asm xref     */
#define VF_EXTD         0x00800000    /*  asm xref has occured    */

#define TF_DYNAMIC      0x10000000    /*  dynamic loaded routine  */
#define TF_LIBCALL      0x20000000    /*  library call first arg-offset */
#define TF_PROTOTYPE    0x40000000    /*  prototype declaration   */
#define TF_NOPROF       0x80000000    /*  disable profiling       */

#define TID_INT         0
#define TID_FLT         1
#define TID_PTR         2
#define TID_ARY         3
#define TID_PROC        4
#define TID_STRUCT      5
#define TID_UNION       6
#define TID_BITFIELD    7           /*  bit field (only structure elements) */

/*
 *  register field flags (var structure only for now)
 */

#define RF_REGMASK      0x00FF
#define RF_REGISTER     0x0100
#define RF_MODIFIED     0x0200      /*  register modified directly          */

/*
 *  note on type->Args for procedure.  negative:  int foo();
 *                                     zero:      int foo(void);
 */

typedef struct Type {
    short   Id;
    const int    *Align;
    int32_t    Flags;
    const int    *Size;

    struct Type *Next;
    struct Type *BasedOnType;  /* qualified type is based on... */
    struct Type *PList;     /*  base of next list of types whos subtype == me */
    struct Type *SubType;   /*  pointer to type, array of type  */

    short   Args;           /*  if procedure or structure.      */
    short   DDFlags;        /*  special Id dependant flags      */

    /*
     *  if procedure or structure.  NOTE!  if defered structure this
     *  holds the lexical state
     */

    struct Var  **Vars;
} Type;

#define DDTYPEF_DEFERED     0x0001  /*  resolution of type defered  */

typedef struct TmpFlt {
    union {
        uint32_t l_Mantissa[4];  /*  msb ... lsb     */
        uword   w_Mantissa[8];
    } u;
    int32_t  tf_Exponent;      /*  base 10         */
    short    tf_Negative;
    short    tf_Reserved1;
} TmpFlt;

#define tf_LMantissa    u.l_Mantissa
#define tf_WMantissa    u.w_Mantissa

