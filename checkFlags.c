#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "checkFlags.h"

/* Initializes all flags to unset */
void setFlags(Flags *flags) {
   flags->c = 0;
   flags->t = 0;
   flags->x = 0;
   flags->f = 0;
   flags->s = 0;
   flags->v = 0;
}

Flags checkFlags(int argc, char *argv[]) {
   
   Flags flags;
   char path[PATH_MAX];
   int i = 0;
   
   /* Get current path */
   if (!getcwd(path, sizeof(path))) {
      perror("");
      exit(EXIT_FAILURE);
   }
   
   /* Exit if no arguments entered */
   if (argc < 2) {
      printf("/%s: you must choose one of the 'ctx' options.\n", path);
      printf("usage: /%s [ctxSp[f tarfile]] [file1 [ file2 [...] ] ]\n", path);
      exit(EXIT_FAILURE);
   }
  
   /* Initialize flags */
   setFlags(&flags);
   
   /* Mark inputted flags */
   while (argv[1][i] != '\0') { 

      if (argv[1][i] == 'c')
         flags.c = 1;
      else if (argv[1][i] == 't')
         flags.t = 1;
      else if (argv[1][i] == 'x')
         flags.x = 1;
      else if (argv[1][i] == 'f')
         flags.f = 1;
      else if (argv[1][i] == 'S')
         flags.s = 1;
      else if (argv[1][i] == 'v')
         flags.v = 1;
      else {
         printf("/%s: unrecognized option '%c'.\n", path, argv[1][i]);
      }
      i++;
   }
   
   /* Exit if no 'c', 't', or 'x' */
   if ( !(flags.c || flags.t || flags.x) ) {
      printf("/%s: you must choose one of the 'ctx' options.\n", path);
      printf("usage: /%s [ctxSp[f tarfile]] [file1 [ file2 [...] ] ]\n", path);
      exit(EXIT_FAILURE);
   }
   
   /* Exit if two or all of 'c', 't', or 'x' */
   if ( (flags.c&flags.t) || (flags.c&flags.x) || (flags.t&flags.x)
      || (flags.c&flags.t&flags.x)) {
      printf("/%s: you may only choose one of the 'ctx' options.\n", path);
      printf("usage: /%s [ctxSp[f tarfile]] [file1 [ file2 [...] ] ]\n", path);
      exit(EXIT_FAILURE);
   }

   return flags;
}
