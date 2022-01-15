/*
 * Copyright (c) Tony Bybell 1999-2000.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef VCD_VCD_H
#define VCD_VCD_H

#include <stdio.h>
#include <stdlib.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "misc.h"
#include "alloca.h"
#include "debug.h"
#include "globals.h"

#define VCD_BSIZ 32768	/* size of getch() emulation buffer--this val should be ok */
#define VCD_FAIL 666

TimeType vcd_main(struct globals *obj, char *fname);
void append_vcd_slisthier(struct globals *obj, char *str);

struct sym_chain
{
struct sym_chain *next;
struct symbol *val;
};

struct slist
{
struct slist *next;
char *str;
int len;
};


struct vcdsymbol
{
struct vcdsymbol *root, *chain;
struct symbol *sym_chain;

struct vcdsymbol *next;
char *name;
char *id;
unsigned char vartype;
int msi, lsi;
int size;
char *value;
struct queuedevent *ev; /* only if vartype==V_EVENT */
struct Node **narray;
};


struct queuedevent
{
struct queuedevent *next;
struct vcdsymbol *sym;
TimeType last_event_time;    /* make +1 == 0 if there's not an event there too */
};

extern char autocoalesce;
extern int vcd_explicit_zero_subscripts;  /* 0=yes, -1=no */
extern char convert_to_reals;
extern char atomic_vectors;
extern char hier_delimeter;

extern TimeType currenttime;
extern TimeType max_time;
extern TimeType min_time;
extern char time_dimension;

#endif
