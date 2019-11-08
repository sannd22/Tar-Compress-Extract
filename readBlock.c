#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "readBlock.h"

/* Reads blocks from input FD and writes blocks to output FD */
void readBlocks(int inFile, int outFile, int size) {
   
   int i = 0;
   int inBlock = 0;
   char currentBlock[512];

   if (size == 0)
      return;
   while (i < size) {
      
      if (inBlock == 512) {
         write(outFile, currentBlock, 512);
         inBlock = 0;
      }
      read(inFile, &currentBlock[inBlock], sizeof(uint8_t));
      i ++;
      inBlock ++;
   }
   
   while (inBlock < 512) {
      currentBlock[inBlock] = '\0';
      inBlock ++;
   }
   write(outFile, currentBlock, 512);
}
