/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "uprev_rev.h"

#define MAX_FNAME 256

//const static char version_string[] = VERSTAG;

int main(int argc, char **argv)
{
   FILE *fp;
   char *vertag;
   int version;
   int revision;
   char fname[MAX_FNAME];
   char buf[80];
   time_t t;
   struct tm *tp;

   if ((argc > 3) || (argc < 2))
   {
      printf("Usage: %s <Project> [<Version>]\n", argv[0]);
      return(5);
   }

   vertag = "";
   version =  1;
   revision = 0;

   /* Determine the name of the revision file */
   strncpy(fname, argv[1], MAX_FNAME-7);
   fname[MAX_FNAME-7] = 0;
   strcat(fname, "_rev.h");

   /* See if the file already exists. */
   fp = fopen(fname, "r");
   if (fp != NULL)
   {
      /* It exists, get the version and revision out of it */
      while(fgets(buf, 80, fp) != NULL)
      {
         int ln;
         ln = strlen(buf)-1;
         if (buf[ln] == '\n') buf[ln] = 0;

         if (!memcmp(buf, "#define ", 8))
         {
            char *p;
            p = buf+8;
            while(*p == ' ' || *p == '\t') p++;
            if (!memcmp(p, "VERS ", 5))
            {
               p += 4;
               while(*p == ' ' || *p == '\t') p++;
               /* Now we need to get past the name of the executable */
               ln = strlen(argv[1]);
               if (strlen(p) > ln)
               {
                  char c;
                  c = p[ln];
                  p[ln] = 0;
                  if (strcasecmp(p, argv[1]))
                  {
                     /* We don't have a match, we need to go for the */
                     /* space which separates the name               */
                     p[ln] = c;
                     while (*p && *p != ' ') p++;
                  }
                  else
                  {
                     /* Well, it matches, shift past the name */
                     p += ln + 1;
                  }
                  /* Now we should be pointing past the name (and a single space) */
                  /* at any aux version tag information that they might have      */
                  /* we want to go from the END of the string and eliminate any   */
                  /* numeric digits that are there.                               */
                  ln = strlen(p);
                  if ((ln > 1) && (p[ln-1] == '"')) ln--;
                  while((ln > 1) && (p[ln-1] >= '0') && (p[ln-1] <= '9')) ln--;
                  if ((ln > 1) && (p[ln-1] == '.'))
                  {
                      ln--;
                      while((ln > 1) && (p[ln-1] >= '0') && (p[ln-1] <= '9')) ln--;
                  }
                  p[ln] = 0;
                  while (*p == ' ' || *p == '\t') p++;
                  vertag = strdup(p);
               }
            }
            else if (!memcmp(p, "VERSION ", 8))
            {
               version = atoi(p+7);
            }
            else if (!memcmp(p, "REVISION ", 9)) revision = atoi(p+8);
         }
      }

      fclose(fp);
   }

   revision++;

   /* Figure out what version number we will be using */
   if (argc > 2)
   {
      int oldver = version;
      version = atoi(argv[2]);
      if (version != oldver)
         revision = 0;
   }

   fp = fopen(fname, "w");
   if (fp == NULL)
   {
      perror(fname);
      return(20);
   }

   time(&t);
   tp = localtime(&t);

   (void)tp->tm_mday;
   (void)tp->tm_year;
   (void)tp->tm_mon;

   sprintf(buf, "%d.%d.%d", tp->tm_mday, tp->tm_mon+1, tp->tm_year);

   /* Figure out what is the number to */

   fprintf(fp, "#define VERSION        %d\n",
                                       version);
   fprintf(fp, "#define REVISION       %3d\n",
                                       revision);
   fprintf(fp, "#define DATE    \"%s\"\n",
                                  buf);
   fprintf(fp, "#define VERS    \"%s %s%d.%d\"\n",
                                 argv[1], vertag, version, revision);
   fprintf(fp, "#define VSTRING \"%s %s%d.%d (%s)\"\n",
                                 argv[1], vertag, version, revision, buf);
   fprintf(fp, "#define VERSTAG \"\\0$%s: %s %s%d.%d (%s)\"\n",
                                 "VER", argv[1], vertag, version, revision, buf);
   fclose(fp);

   return(0);
}
