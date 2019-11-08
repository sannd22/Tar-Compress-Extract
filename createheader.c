#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdint.h>
#include "createheader.h"
#include "binaryHeaders.h"

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
#define MAXNAME 256


char filetype(struct stat file_stat); 

/* returns 0 on success, 1 on failure */
int writePath(char *filename, char *block) {
   
   int len, prefixLen;
   char *cur, *split;

   len = strlen(filename);

   if (len > MAXNAME) { 
      return 1;
   }
   else {
      if (len <= 100) {
         strcpy(&block[NAMEOFF], filename);
         block[PREFIXOFF] = '\0';
         return 0;
      }
      else {
         cur = filename + len - 101;
         split = index(cur, '/');

         if (split) {
            prefixLen = split - filename;
            if (prefixLen <= 155) {
               
               memmove(&block[PREFIXOFF], filename, prefixLen);
               
               if (prefixLen < 155) {

                  block[PREFIXOFF + prefixLen] = '\0';
               }
               strcpy(&block[NAMEOFF], split + 1);
            }
            else {
               return 1;
            }
         }
         else {
            return 1;
         }
      }
   }
   return 0;
}

void symValue(char *filename, char *buf) {
   int fd;
   int i = 0;

   if ((fd = open(filename, O_RDONLY)) == -1) {
      perror("cannot open sym link");
      exit(EXIT_FAILURE);
   }

   while ((read(fd, &buf[i], sizeof(uint8_t)) > 0) && (i < 100))
      i ++;
}

void uName(uid_t uid, char *buf) {

  struct passwd *pass = getpwuid(uid);
  if (strlen(pass->pw_name) < 32)
    strcpy(buf, pass->pw_name); 
}

void gName(gid_t gid, char *buf) {

   struct group *grp = getgrgid(gid);
   if (strlen(grp->gr_name) < 32)
      strcpy(buf, grp->gr_name);
}

void emptyHeader(char *header, int outFD) {
   strcpy(&header[MODEOFF], "0000000");
   strcpy(&header[UIDOFF],  "0000000");
   strcpy(&header[GIDOFF],  "0000000");
   strcpy(&header[SIZEOFF],  "00000000000");
   strcpy(&header[MTIMEOFF],  "00000000000");
   strcpy(&header[UIDOFF],  "0000000");
   strcpy(&header[CHKSUMOFF], "0004420");
   write(outFD, header, BLOCKSIZE); 
}

int createheader(char *filename, int outFD) {
  int i;
  struct stat file_stat;
  int chksum = 0;
  char m;
  char ustar[6] = {'u','s','t','a','r','\0'};
  char *block = calloc(BLOCKSIZE, sizeof(uint8_t));
  if(lstat(filename, &file_stat) < 0) {
    perror("unable to stat file");
    return 1;
  }
  
  if (writePath(filename, block)) {
     emptyHeader(block, outFD);
     free(block);
     return 1;
  }
  sprintf(&block[MODEOFF], "%07o", file_stat.st_mode);
  block[MODEOFF] = '0';
  block[MODEOFF + 1] = '0';
  block[MODEOFF + 2] = '0';
  if (insert_special_int(&block[UIDOFF], (size_t) 8, (int32_t) file_stat.st_uid))
     perror("");
   
  sprintf(&block[GIDOFF], "%07o", file_stat.st_gid);
  sprintf(&block[SIZEOFF], "%011o", (unsigned int) file_stat.st_size);
  sprintf(&block[MTIMEOFF], "%011o", (unsigned int) file_stat.st_mtime);
  
  m = filetype(file_stat);
  block[TYPEFLAGOFF] = m;
  if (m == '2') {
    symValue(filename, &block[LINKNAMEOFF]);
  }

  if (m == '2' || m == '5') {
     sprintf(&block[SIZEOFF], "%011o", 0);
     if (m == '5')
        block[strlen(&block[NAMEOFF])] = '/';
  }

  strcpy(&block[MAGICOFF], ustar);
  block[VERSIONOFF] = '0';
  block[VERSIONOFF + 1] = '0';
 
  uName(file_stat.st_uid, &block[UNAMEOFF]);
  gName(file_stat.st_gid, &block[GNAMEOFF]);
  
  /* add chksum loop */
  for (i=0; i < 512; i++)
     chksum += (unsigned char) block[i];
  /* add spaces */
  chksum += (32 * 8);
  
  sprintf(&block[CHKSUMOFF], "%07o", chksum);
  /* prefix OVERFLOW */
   
  write(outFD, block, BLOCKSIZE);
  free(block);
  return 0;
}

char filetype(struct stat file_stat) {
  if(S_ISREG(file_stat.st_mode)) {
    return '0';
  }
  else if(S_ISLNK(file_stat.st_mode)) {
    return '2';
  }
  else if(S_ISDIR(file_stat.st_mode)) {
    return '5';
  }
  else {
    /* x is code for differnet file type */
    return 'x';
  }
}
