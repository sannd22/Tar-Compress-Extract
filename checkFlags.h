#ifndef CHECKFLAGS_H
#define CHECKFLAGS_H

typedef struct {
   int c;
   int t;
   int x;
   int f;
   int s;
   int v;
} Flags;

Flags checkFlags(int argc, char *argv[]);

#endif
