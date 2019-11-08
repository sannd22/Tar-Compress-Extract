#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "createheader.h"
#include "binaryHeaders.h"
#include "archiveListing.h"

#define BLOCKSIZE 512

#define NAMEOFF 0
#define MODEOFF 100
#define UIDOFF 108
#define GIDOFF 116
#define SIZEOFF 124
#define MTIMEOFF 136
#define CHKSUMOFF 148
#define TYPEFLAGOFF 156
#define LINKNAMEOFF 157
#define MAGICOFF 257
#define VERSIONOFF 263
#define UNAMEOFF 265
#define GNAMEOFF 297
#define PREFIXOFF 345

long int octTOint(char *octal) {
  char *oct_ptr;
  long int oct_int, dec_int = 0;
  int i = 0;
  oct_int = (long int) strtol(octal, &oct_ptr, 10);
  /* convert octal to decimal for time_t */
  while(oct_int != 0) {
    dec_int = dec_int + (oct_int % 10)* pow(8, i++);
    oct_int = oct_int / 10;
  }
  return dec_int;
}

char *permsExpand(char *perms) {
  int permList[9] = { S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH };
  int i = 0;
  char *permissions;
  long int perm_int;

  perm_int = octTOint(perms);

  permissions = calloc(10, sizeof(char));

  while(i < 9) {
    if(perm_int & permList[i]) {
      permissions[i] = 'r';
    }
    else {
      permissions[i] =  '-';
    }
    i++;
    if(perm_int & permList[i]) {
      permissions[i] = 'w';
    }
    else {
      permissions[i] = '-';
    }
    i++;
    if(perm_int & permList[i]) {
      permissions[i] = 'x';
    }
    else {
      permissions[i] = '-';
    }
    i++;
  }
  return permissions;
}

int checkIfEnd(int tarFD) {
  off_t offset;
  int i;
  char blocks[1024];

  read(tarFD, blocks, 1024);

  for (i=0; i < 1024; i++) {
    if (blocks[i] != '\0') {
      offset = lseek(tarFD, -1024, SEEK_CUR);
      if(offset == -1) {
        perror("lseek failure");
        exit(EXIT_FAILURE);
      }
      return 1;
    }
  }
  return 0;
}

void printFullpath(char *name, char *prefix) {
  char fullpath[256];
  /* check if there is part of the pathname in prefix */
  if(strlen(prefix) == 0) {
    printf("%s\n", name);
  }
  /* if prefix exits is fullpath = prefix + name */
  else {
    sprintf(fullpath, "%s%c%s", prefix, '/', name);
    printf("%s\n", fullpath);
  }
}

void readFile(int tarFD, int v) {
  char *permissions, *owner, *group; 
  long int size;
  time_t mtime;
  struct tm timer;
  /*long int oct_mtime, dec_mtime;*/
  /*char *mtime_ptr;*/
  char timebuff[80];
  /*int i;*/
  char header[BLOCKSIZE];
  int numBlocks = 0;
  if (read(tarFD, header, BLOCKSIZE) == -1)
     perror("read failure, check permissions");
  
  size = octTOint(&header[SIZEOFF]);

  if(v) {
    /* get file type */
    if(header[TYPEFLAGOFF] == '5') {
      printf("%c", 'd');
    }
    else if(header[TYPEFLAGOFF] == '2') {
      printf("%c", '1');
    }
    else
      printf("%c", '-');

    /* permissions */
    permissions = permsExpand(&header[MODEOFF]);
    printf("%9s ", permissions);
    free(permissions);

    /* owner and group */
    owner = &header[UNAMEOFF];
    group = &header[GNAMEOFF];
    strcat(owner, "/");
    strcat(owner, group);
    printf("%-17s ", owner);

    /*size (in bytes) */
    printf("%8ld ", size);

    /* set mtime to new decimal value */
    mtime = (time_t) octTOint(&header[MTIMEOFF]);
    /* retrive time */
    localtime_r(&mtime, &timer);
    strftime(timebuff, 80, "%Y-%m-%d %H:%M", &timer);
    printf("%16s ", timebuff);
  }
  /* always print full filename */
  printFullpath(&header[NAMEOFF], &header[PREFIXOFF]);

  /* increment file pointer to next header block in file */
  if (size) {
    numBlocks = 1 + (size / BLOCKSIZE); 
  }
  
  while (numBlocks) {
     read(tarFD, header, BLOCKSIZE);
     numBlocks --;
  }
}

/* function will read through tar and print appropriate listings for each file specified */
int printListings(char *tarName, int v) {
  int tarFD;

  if((tarFD = open(tarName, O_RDONLY)) == -1) {
    perror("invalid tar file");
    exit(EXIT_FAILURE);
  }

  while(checkIfEnd(tarFD)) {
    readFile(tarFD, v);
  }
  close(tarFD);
  return 0;
}
