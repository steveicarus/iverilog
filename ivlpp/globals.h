#ifndef IVL_globals_H
#define IVL_globals_H
/*
 * Copyright (c) 1999-2017 Stephen Williams (steve@icarus.com)
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

# include  <stdio.h>

extern void reset_lexor(FILE*out, char*paths[]);
extern void destroy_lexor(void);
extern void load_precompiled_defines(FILE*src);
extern void define_macro(const char*name, const char*value, int keyword,
                         int argc);
extern void free_macros(void);
extern void dump_precompiled_defines(FILE*out);

/* These variables contain the include directories to be searched when
   an include directive in encountered. */
extern char**include_dir;
extern unsigned include_cnt;
/* Program to use for VHDL processing. */
extern char*vhdlpp_path;
/* vhdlpp work directory */
extern char*vhdlpp_work;

extern char**vhdlpp_libdir;
extern unsigned vhdlpp_libdir_cnt;

extern int relative_include;

/* This flag is true if #line directives are to be generated. */
extern int line_direct_flag;

extern unsigned error_count;

extern FILE *depend_file;
extern char dep_mode;

extern int verbose_flag;

extern int warn_redef;
extern int warn_redef_all;

/* This is the entry to the lexer. */
extern int yylex(void);

#endif /* IVL_globals_H */
