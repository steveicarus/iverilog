/*
 * Copyright (c) 2001 Tony Bybell.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the   
 * Software is furnished to do so, subject to the following conditions: 
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,  
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL   
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING   
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef DEFS_LXT_H
#define DEFS_LXT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define LT_HDRID (0x0138)
#define LT_VERSION (0x0001)
#define LT_TRLID (0xB4)

#define LT_CLKPACK (4)

#define LT_MVL_2	(1<<0)
#define LT_MVL_4	(1<<1)
#define LT_MVL_9	(1<<2)

struct lt_timetrail
{
struct lt_timetrail *next;
int timeval;
unsigned int position;
};


#define LT_SYMPRIME 65519

#define LT_SECTION_END			(0)
#define LT_SECTION_CHG			(1)
#define LT_SECTION_SYNC_TABLE		(2)
#define LT_SECTION_FACNAME		(3)
#define LT_SECTION_FACNAME_GEOMETRY	(4)
#define LT_SECTION_TIMESCALE		(5)
#define	LT_SECTION_TIME_TABLE		(6)
#define LT_SECTION_INITIAL_VALUE	(7)
#define LT_SECTION_DOUBLE_TEST		(8)

struct lt_trace
{
FILE *handle;
unsigned int position;

struct lt_symbol *sym[LT_SYMPRIME];
struct lt_symbol **sorted_facs;
struct lt_symbol *symchain;
int numfacs;
int numfacbytes;
int longestname;
int mintime, maxtime;
int timescale;
int initial_value;

struct lt_timetrail *timehead, *timecurr, *timebuff;
int timechangecount;
char double_used;
char do_strip_brackets;
char clock_compress;

unsigned int change_field_offset;
unsigned int facname_offset;
unsigned int facgeometry_offset;
unsigned int time_table_offset;
unsigned int sync_table_offset;
unsigned int initial_value_offset;
unsigned int timescale_offset;
unsigned int double_test_offset;

char *compress_fac_str;
int compress_fac_len;

int timeval; /* for clock induction */
};


struct lt_symbol
{
struct lt_symbol *next;
struct lt_symbol *symchain;
char *name;
int namlen;

int facnum;
struct lt_symbol *aliased_to;

unsigned int rows;
int msb, lsb;
int len;
int flags;

unsigned int last_change;

int 		clk_delta;
int		clk_prevtrans;
int 		clk_numtrans;
char 		clk_prevval;
};

#define LT_SYM_F_BITS           (0)
#define LT_SYM_F_INTEGER        (1<<0)
#define LT_SYM_F_DOUBLE         (1<<1)
#define LT_SYM_F_STRING         (1<<2)
#define LT_SYM_F_ALIAS          (1<<3)


struct lt_trace *	lt_init(const char *name);
void 			lt_close(struct lt_trace *lt);

struct lt_symbol *	lt_symbol_find(struct lt_trace *lt, const char *name);
struct lt_symbol *	lt_symbol_add(struct lt_trace *lt, const char *name, unsigned int rows, int msb, int lsb, int flags);
struct lt_symbol *	lt_symbol_alias(struct lt_trace *lt, const char *existing_name, const char *alias, int msb, int lsb);
void			lt_symbol_bracket_stripping(struct lt_trace *lt, int doit);

void 			lt_set_timescale(struct lt_trace *lt, int timescale);
void 			lt_set_initial_value(struct lt_trace *lt, char value);
int 			lt_set_time(struct lt_trace *lt, int timeval);
void 			lt_set_clock_compress(struct lt_trace *lt);

/*
 * value change functions..note that if the value string len for 
 * lt_emit_value_bit_string() is shorter than the symbol length 
 * it will be left justified with the rightmost character used as 
 * a repeat value that will be propagated to pad the value string out:
 *
 * "10x" for 8 bits becomes "10xxxxxx"
 * "z" for 8 bits becomes   "zzzzzzzz"
 */
int 			lt_emit_value_int(struct lt_trace *lt, struct lt_symbol *s, unsigned int row, int value);
int 			lt_emit_value_double(struct lt_trace *lt, struct lt_symbol *s, unsigned int row, double value);
int 			lt_emit_value_string(struct lt_trace *lt, struct lt_symbol *s, unsigned int row, char *value);
int 			lt_emit_value_bit_string(struct lt_trace *lt, struct lt_symbol *s, unsigned int row, char *value);

#endif
