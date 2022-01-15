/*
 * Copyright (c) Tony Bybell 1999-2000.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */


/*
 * misc.h 08/09/97ajb
 */
#ifndef VCD_MISC_H
#define VCD_MISC_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "alloca.h"
#include "analyzer.h"
#include "debug.h"
#include "globals.h"

#define SYMPRIME 4093
#define WAVE_DECOMPRESSOR "gzip -cd "	/* zcat alone doesn't cut it for AIX */

struct symbol
{
struct symbol *altsym;	/* points to sym in alt vcd file */

struct symbol *nextinaet;/* for aet node chaining */
struct HistEnt *h;	 /* points to previous one */

struct symbol *vec_root, *vec_chain;

struct symbol *next;	/* for hash chain */
char *name;

struct Node *n;
};


struct symbol *symfind(struct globals *, char *);
struct symbol *symadd(struct globals *, char *, int);
int hash(char *s);

int sigcmp(char *, char *);
void quicksort(struct symbol **, int, int);


extern struct symbol **sym, **facs;
extern char facs_are_sorted;
extern int numfacs;
extern int regions;
extern struct symbol *firstnode;
extern struct symbol *curnode;
extern int longestname;
extern char hier_delimeter;

#endif
