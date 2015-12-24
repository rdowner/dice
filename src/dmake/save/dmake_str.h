#ifndef DMAKE_STR_H
#define DMAKE_STR_H


/****************************************************************************/


/* This file was created automatically by CatComp.
 * Do NOT edit by hand!
 */


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif


/****************************************************************************/


#define MSG_0000 0
#define MSG_0000_STR "bad variable spec in command list for %s"

#define MSG_0001 1
#define MSG_0001_STR "    %s\n"

#define MSG_0002 2
#define MSG_0002_STR "maximum recursion reached in WildConvert"

#define MSG_0003 3
#define MSG_0003_STR "DMAKE Done.\n"

#define MSG_0004 4
#define MSG_0004_STR "All Targets up to date.\n"

#define MSG_0005 5
#define MSG_0005_STR "Bad ToolType: %s"

#define MSG_0006 6
#define MSG_0006_STR "Ignore"

#define MSG_0007 7
#define MSG_0007_STR "Abort"

#define MSG_0008 8
#define MSG_0008_STR "Expected argument to command line option"

#define MSG_0009 9
#define MSG_0009_STR "Unable to find %s"

#define MSG_0010 10
#define MSG_0010_STR "Unable to open %s"

#define MSG_0011 11
#define MSG_0011_STR "Expected a symbol!"

#define MSG_0012 12
#define MSG_0012_STR "%d items on the left, %d on the right of colon!"

#define MSG_0013 13
#define MSG_0013_STR "Variable %s does not exist"

#define MSG_0014 14
#define MSG_0014_STR "Bad variable specification after name"

#define MSG_0015 15
#define MSG_0015_STR "Bad variable replacement spec: %c"

#define MSG_0016 16
#define MSG_0016_STR "Too many levels of variable recursion"

#define MSG_0017 17
#define MSG_0017_STR "newline in control string"

#define MSG_0018 18
#define MSG_0018_STR "Symbol overflow: %s"

#define MSG_0019 19
#define MSG_0019_STR "Expected closing quote"

#define MSG_0020 20
#define MSG_0020_STR "Unexpected token"

#define MSG_0021 21
#define MSG_0021_STR "Fatal"

#define MSG_0022 22
#define MSG_0022_STR "Warning"

#define MSG_0023 23
#define MSG_0023_STR "Debug"

#define MSG_0024 24
#define MSG_0024_STR "%s: %s Line %d:"

#define MSG_0025 25
#define MSG_0025_STR "Unable to makedir %s\n"

#define MSG_0026 26
#define MSG_0026_STR "Unable to write %s\n"

#define MSG_0027 27
#define MSG_0027_STR "Unable to cd %s\n"

#define MSG_0028 28
#define MSG_0028_STR "%s\n\r"

#define MSG_0029 29
#define MSG_0029_STR "Exit code %d %s\n"

#define MSG_0030 30
#define MSG_0030_STR "(Ignored)"


/****************************************************************************/


#ifdef STRINGARRAY

struct AppString
{
    LONG   as_ID;
    STRPTR as_Str;
};

struct AppString AppStrings[] =
{
    {MSG_0000,(STRPTR)MSG_0000_STR},
    {MSG_0001,(STRPTR)MSG_0001_STR},
    {MSG_0002,(STRPTR)MSG_0002_STR},
    {MSG_0003,(STRPTR)MSG_0003_STR},
    {MSG_0004,(STRPTR)MSG_0004_STR},
    {MSG_0005,(STRPTR)MSG_0005_STR},
    {MSG_0006,(STRPTR)MSG_0006_STR},
    {MSG_0007,(STRPTR)MSG_0007_STR},
    {MSG_0008,(STRPTR)MSG_0008_STR},
    {MSG_0009,(STRPTR)MSG_0009_STR},
    {MSG_0010,(STRPTR)MSG_0010_STR},
    {MSG_0011,(STRPTR)MSG_0011_STR},
    {MSG_0012,(STRPTR)MSG_0012_STR},
    {MSG_0013,(STRPTR)MSG_0013_STR},
    {MSG_0014,(STRPTR)MSG_0014_STR},
    {MSG_0015,(STRPTR)MSG_0015_STR},
    {MSG_0016,(STRPTR)MSG_0016_STR},
    {MSG_0017,(STRPTR)MSG_0017_STR},
    {MSG_0018,(STRPTR)MSG_0018_STR},
    {MSG_0019,(STRPTR)MSG_0019_STR},
    {MSG_0020,(STRPTR)MSG_0020_STR},
    {MSG_0021,(STRPTR)MSG_0021_STR},
    {MSG_0022,(STRPTR)MSG_0022_STR},
    {MSG_0023,(STRPTR)MSG_0023_STR},
    {MSG_0024,(STRPTR)MSG_0024_STR},
    {MSG_0025,(STRPTR)MSG_0025_STR},
    {MSG_0026,(STRPTR)MSG_0026_STR},
    {MSG_0027,(STRPTR)MSG_0027_STR},
    {MSG_0028,(STRPTR)MSG_0028_STR},
    {MSG_0029,(STRPTR)MSG_0029_STR},
    {MSG_0030,(STRPTR)MSG_0030_STR},
};


#endif /* STRINGARRAY */


/****************************************************************************/


#endif /* DMAKE_STR_H */
