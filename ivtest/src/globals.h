/*
 * Copyright (c) Tony Bybell 1999-2000.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef WAVE_GLOBALS_H
#define WAVE_GLOBALS_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "alloca.h"
#include "debug.h"

struct globals
{
struct symbol **sym;
struct symbol **facs;
char facs_are_sorted;

int numfacs;
int regions;
int longestname;

struct symbol *firstnode;  /* 1st sym in aet */
struct symbol *curnode;    /* current loaded sym in aet loader */

char hier_delimeter;
char autocoalesce;

int vcd_explicit_zero_subscripts;  /* 0=yes, -1=no */
char convert_to_reals;
char atomic_vectors;

FILE *vcd_handle;
char vcd_is_compressed;

int vcdbyteno;         /* really should be size_t, but this is only used for debugging mangled traces */
int header_over;
int dumping_off;
TimeType start_time;
TimeType end_time;
TimeType current_time;
TimeType time_scale;   /* multiplier is 1, 10, 100 */

int count_glitches;    /* set to 1 if we want to enable glitch code */
int num_glitches;
int num_glitch_regions;

char vcd_hier_delimeter[2];   /* fill in after rc reading code */

struct vcdsymbol *pv, *rootv;

struct slist *slistroot, *slistcurr;
char *slisthier;
int slisthier_len;

int T_MAX_STR;      /* was originally a const..now it reallocs */
char *yytext;
int yylen, yylen_cache;

struct vcdsymbol *vcdsymroot, *vcdsymcurr;
struct vcdsymbol **sorted;

int numsyms;

struct HistEnt *he_curr, *he_fini;

char *vcdbuf, *vst, *vend;

char *varsplit, *vsplitcurr;

int var_prevch;

TimeType currenttime, max_time, min_time;
char time_dimension;
};


struct globals *make_vcd_class(void);


#endif
