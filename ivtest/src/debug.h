/*
 * Copyright (c) Tony Bybell 1999-2000
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef VCD_DEBUG_H
#define VCD_DEBUG_H

#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>

struct memchunk
{
struct memchunk *next;
void *ptr;
size_t size;
};


/*
 * If you have problems viewing traces (mangled timevalues),
 * make sure that you use longs rather than the glib 64-bit
 * types...
 */
#ifdef G_HAVE_GINT64
	typedef gint64          TimeType;
	typedef guint64         UTimeType;

        #ifndef _MSC_VER
                #define LLDescriptor(x) x##LL
                #define ULLDescriptor(x) x##ULL
                #define TTFormat "%lld"
                #define UTTFormat "%llu"
        #else
                #define LLDescriptor(x) x##i64
                #define ULLDescriptor(x) x##i64
                #define TTFormat "%I64d"
                #define UTTFormat "%I64u"
        #endif

	#define WAVE_MINZOOM (LLDescriptor(-4000000000))
#else
	typedef long            TimeType;
	typedef unsigned long   UTimeType;

	#define TTFormat "%d"
	#define UTTFormat "%u"

	#define LLDescriptor(x) x
	#define ULLDescriptor(x) x

	#define WAVE_MINZOOM (LLDescriptor(-20000000))
#endif

#ifdef DEBUG_PRINTF
#define DEBUG(x) x
#else
#define DEBUG(x)
#endif

#ifdef DEBUG_MALLOC
#define DEBUG_M(x) x
#else
#define DEBUG_M(x)
#endif


#ifdef DEBUG_MALLOC_LINES
void free_2(void *ptr, char *filename, int lineno);
#define free_2(x) free_2((x),__FILE__,__LINE__)
#else
void free_2(void *ptr);
#endif


void *malloc_2(size_t size);
void *realloc_2(void *ptr, size_t size);
void *calloc_2(size_t nmemb, size_t size);

TimeType atoi_64(char *str);
extern char *atoi_cont_ptr;	/* for unformat_time()'s parse continue for the time unit */

#undef WAVE_USE_SIGCMP_INFINITE_PRECISION  /* define this for slow sigcmp with infinite digit accuracy */
#define WAVE_OPT_SKIP 1			   /* make larger for more accel on traces */

#endif
