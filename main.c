#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include "checkFlags.h"
#include "readBlock.h"
#include "getchildren.h"
#include "createheader.h"
#include "archiveListing.h"
#include "archiveExtract.h"

int main(int argc, char *argv[]) {
   
   Flags flags;
   int outFD;
   flags = checkFlags(argc, argv);
   
   /* Checks for f flag */
   if (!flags.f) {
      printf("f flag required\n");
      printf("usage: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
      exit(EXIT_FAILURE);
   }

   /* Create an Archive */
   if (flags.c) {
      
      if ((outFD = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0777)) == -1) {
         perror("open failure");
         exit(EXIT_FAILURE);
      }
      getchildren(argv[3], outFD, flags.v);
   }
   
   /* Print the table of contents of an archive */
   if (flags.t) {
   
      printListings(argv[2], flags.v);

   }
   
   /* Extract the contents of an archive */
   if (flags.x) {
      
      extractMain(argc, argv, flags.v);
   }
   
   return 0;
}
