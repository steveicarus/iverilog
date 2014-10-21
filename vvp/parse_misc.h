#ifndef IVL_parse_misc_H
#define IVL_parse_misc_H
/*
 * Copyright (c) 2001-2014 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "vpi_priv.h"

/*
 * This method is called to compile the design file. The input is read
 * and a list of statements is created.
 */
extern int compile_design(const char*path);

/*
 * This routine is called to check that the input file has a compatible
 * version.
 */
extern void verify_version(char *ivl_ver, char* commit);

/*
 * Set the default delay type for the $sdf_annotate task (min/typ/max).
 */
extern void set_delay_selection(const char* sel);

/*
 * various functions shared by the lexor and the parser.
 */
extern int yylex(void);
extern void yyerror(const char*msg);

extern void destroy_lexor();

/*
 * This is the path of the current source file.
 */
extern const char*yypath;
extern unsigned yyline;

struct symb_s {
      char*text;
      unsigned idx;
};

struct symbv_s {
      unsigned cnt;
      struct symb_s*vect;
};

extern void symbv_init(struct symbv_s*obj);
extern void symbv_add(struct symbv_s*obj, struct symb_s item);

struct numbv_s {
      unsigned cnt;
      long*nvec;
};

struct enum_name_s {
      char*text;
      uint64_t val2;
      char*val4;
};

extern void numbv_init(struct numbv_s*obj);
extern void numbv_add(struct numbv_s*obj, long item);
extern void numbv_clear(struct numbv_s*obj);

struct argv_s {
      unsigned  argc;
      vpiHandle*argv;
      char    **syms;
};


extern void argv_init(struct argv_s*obj);
extern void argv_add(struct argv_s*obj, vpiHandle);
extern void argv_sym_add(struct argv_s*obj, char *);
extern void argv_sym_lookup(struct argv_s*obj);

#endif /* IVL_parse_misc_H */
