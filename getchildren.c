#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdint.h>
#include "getchildren.h"
#include "readBlock.h"
#include "createheader.h"

int getchildrenhelper(char *filename, int outFD, int v);

void writeFile(char *fullpath, int outFD, int size) {
   int inFile;
   if ((inFile = open(fullpath, O_RDONLY)) == -1) {
      perror("Couldn't open file");
      return;
   }
   readBlocks(inFile, outFD, size);
}

int getchildren(char *filename, int outFD, int v) {
  struct stat buff;
  int success;
  char *nulls = calloc(1024, sizeof(uint8_t));
  success = lstat(filename, &buff);
  if(success != 0) {
    perror("could not lstat file");
    exit(EXIT_FAILURE);
  }

  if(S_ISDIR(buff.st_mode)) {
    if (createheader(filename, outFD) == 0) {
      success = getchildrenhelper(filename, outFD, v);
      if(success != 0) {
        perror("Error getting file");
      }
    }
    else
      perror("");
  }

  if(S_ISREG(buff.st_mode)) {
 
    if (createheader(filename, outFD) == 0) {
       if (v)
          printf("%s\n", filename);
       writeFile(filename, outFD, (int) buff.st_size);
     } else
      perror("");
  }
  write(outFD, nulls, 1024);
  free(nulls);
  return 0;
}

int getchildrenhelper(char *filename, int outFD, int v) {
  int success;
  DIR *file_dir;
  struct dirent *file_dirent;
  char fullpath[PATH_MAX + 1];
  char *nulls = calloc(1024, sizeof(uint8_t));
  if (v)
    printf("%s/\n", filename);

  file_dir = opendir(filename);
  if(file_dir == NULL) {
    perror("could not open directory");
    exit(EXIT_FAILURE);
  }  

  /* get all children in directory and print out */
  file_dirent = readdir(file_dir);
  if(file_dirent == NULL) {
    perror("could not read first file in directory");
    exit(EXIT_FAILURE);
  } 
  while(file_dirent != 0) {
    if(strcmp(file_dirent->d_name, ".") && strcmp(file_dirent->d_name, "..")) {
      struct stat file_buff;
      /* get stat for file */
      strcpy(fullpath, filename);
      strcat(fullpath, "/");
      strcat(fullpath, file_dirent->d_name);
      success = stat(fullpath, &file_buff);
      if(success == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
      }
      /* regular file case */
      if(S_ISREG(file_buff.st_mode)) {
        
        if (createheader(fullpath, outFD) == 0) {
           if (v)
              printf("%s\n", fullpath);
           writeFile(fullpath, outFD, (int) file_buff.st_size);
        }
        else
           perror("");
      }
      /* directory file case */
      if(S_ISDIR(file_buff.st_mode)) {
        if (createheader(fullpath, outFD) == 0) {
          success = getchildrenhelper(fullpath, outFD, v);
          if(success != 0) {
            return -1;
          }
        }
        else
           perror("");
      }
    }
    file_dirent = readdir(file_dir);
  }
  closedir(file_dir);
  free(nulls);
  return 0;
}
