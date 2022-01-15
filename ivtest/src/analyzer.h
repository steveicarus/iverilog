/*
 * Copyright (c) Tony Bybell 1999-2000.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef VCD_ANALYZER_H
#define VCD_ANALYZER_H

#include <gtk/gtk.h>
#include <stdlib.h>
#include "alloca.h"
#include "debug.h"


typedef struct Node	 *nptr;
typedef struct HistEnt	 *hptr;
typedef struct Bits      *bptr;
typedef struct VectorEnt *vptr;
typedef struct BitVector *bvptr;

typedef unsigned long  Ulong;
typedef unsigned int   Uint;


typedef struct HistEnt
{
hptr next;	      /* next transition in history */
TimeType time;        /* time of transition */
unsigned char flags;  /* so far only set on glitch/real condition */

union
  {
  unsigned char val;    /* value: "0XZ1"[val] */
  char *vector;		/* pointer to a whole vector */
  } v;

} HistEnt;

enum HistEntFlagBits
{ HIST_GLITCH_B, HIST_REAL_B, HIST_STRING_B
};

#define HIST_GLITCH (1<<HIST_GLITCH_B)
#define HIST_REAL   (1<<HIST_REAL_B)

#ifndef STRICT_VCD_ONLY
	#define HIST_STRING (1<<HIST_STRING_B)
#else
	#define HIST_STRING 0	/* for gcc -O2 optimization */
#endif


#define MAX_HISTENT_TIME (~( (ULLDescriptor(-1)) << (sizeof(TimeType) * 8 - 1)))


typedef struct ExtNode
  {
  int msi, lsi;
  } ExtNode;

struct Node
  {
    char     *nname;	/* ascii name of node */
    ExtNode  *ext;	/* extension to node for vectors */
    HistEnt  head;	/* first entry in transition history */
    hptr     curr;      /* ptr. to current history entry */

    hptr     *harray;   /* fill this in when we make a trace.. contains  */
			/*  a ptr to an array of histents for bsearching */
    int      numhist;	/* number of elements in the harray */
  };


#endif
