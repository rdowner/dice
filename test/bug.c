#include <stdio.h>

struct STRINGS {
   char *search;
   char *replace;
};

struct STRING lookup[] = {
   { "<R>",             NULL         },
   { "<%15>",           NULL         },
   { "<%0>"             NULL         },
   { "<$!B1>",          NULL         },
   { "<D>",             NULL         },
   { "<I>",             NULL         },
   { "<T>",             NULL         },
   { "<F1>"             NULL         },
   { "<F255>",          NULL         },
   { "<B>"              NULL         },
   { "<F255>",          "<F255D>"    },
   { "<BI>",            "<I>"        },
   { "<MI>",            "<I>"        },
   { "<F2M>",           "<F102>"     },
   { "<F102>",          "<F1>"       },
   { "<F2MX>",          "<F1>"       },
   { "<M>",             ""           },
   { "<N>",             ""           },
   { "<F2M%15>",        "<F1%15>"    },
   { "<F1B>",           "<B>"        },
   { "<MIX>",           "<I>"        },
   { "<SELF>",          "<<SELF>>"   },
   { "<BX>",            "<F1>"       },
   { "<F255MI>",        "<F255D><I>" },
   { "<F14B>",          "<B>"        },
   { "<F2>",            "<F1>"       },
   { "<F1%15>",         "<F1%15>"    },
   { "<F255MIX>",       "<I>"        },
   { "<F14P12C1>",      "<F255D>"    },
   { "<F102P255C255>",  "<F1>"       }
};

#define C_LEGAL (sizeof(lookup)/sizeof(struct STRINGS))


char buffer[200];
char oldbuf[200];
char hold[200];

void main(int argc, char **argv)
{
   FILE *infp, *outfp;
   int line;

   if (argc != 3)
   {
      printf("Usage: %s infile outfile\n", argv[0]);
      exit(1);
   }

   infp = fopen(argv[1], "r");
   if (infp == NULL)
   {
      perror(argv[1]);
      exit(1);
   }

   outfp = fopen(argv[2], "r");
   if (outfp == NULL)
   {
      perror(argv[2]);
      exit(1);
   }

   line = 0;

   while(fgets(buffer, 200, fp) != NULL)
   {
      int len;
      char *p, *endp, *bp;
      int changed;

      strcpy(oldbuf, buffer);

      line++;

      len = strlen(buffer) - 1;
      changed = 0;

      if (buffer[len] != '\n')
      {
         printf("Line %d too long\n", line);
      }
      buffer[len] = 0;
      /* Now go through and scan for any < sequences */
      bp = buffer;
      while ((p = strchr(bp, '<')) != NULL)
      {
         /* We have the start of a possible sequence */
         if (p[1] == '<')
         {
            bp = p+2;
         }
         else
         {
            int plen, i;
            char *replace;

            endp = strchr(p, '>');
            if (endp == NULL)
            {
               printf("Line %d: Missing '>'\n", line);
               puts(buffer);
               break;
            }
            plen = (endp-p)+1;
            memcpy(hold, p, plen);
            hold[plen] = 0;
            /* Now we need to search for hold amongst the lookup strings */
            for(i = 0; i < C_LEGAL; i++)
            {
               if (!strcmp(lookup[i].search, hold))
               {
                  /* We found a match */
                  replace = lookup[i].replace;
                  break;
               }
            }
            if (i >= C_LEGAL)
            {
               /* No match, let them know and don't replace it */
               replace = NULL;
            }
            if (replace)
            {
               int moveoff;

               moveoff = strlen(replace)-plen;
               if (moveoff)
               {
                  memmove(endp+moveoff, endp, strlen(endp))
               }
               memcpy(p, replace, strlen(replace));
            }
            changed++;
         }
      }
      fprintf(outfp, "%s\n", buffer);
      if (changed)
      {
         printf("Line %d: %d change%s\n%s\n%s\n", line, changed,
                    ((changed > 1) ? "s" : ""), oldbuf, buffer);

      }
   }
}
