#ifndef __globals_H
#define __globals_H
/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#ifdef HAVE_CVS_IDENT
#ident "$Id: globals.h,v 1.7 2002/08/12 01:35:02 steve Exp $"
#endif

# include  <stdio.h>

extern void reset_lexor(FILE*out, char*paths[]);
extern void define_macro(const char*name, const char*value, int keyword);

/* These variables contain the include directories to be searched when
   an include directive in encountered. */
extern char**include_dir;
extern unsigned include_cnt;

/* This flag is true if #line directives are to be generated. */
extern int line_direct_flag;

extern unsigned error_count;

extern FILE *depend_file;

/* This is the entry to the parser. */
extern int yyparse();

/*
 * $Log: globals.h,v $
 * Revision 1.7  2002/08/12 01:35:02  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.6  2002/04/04 05:26:13  steve
 *  Add dependency generation.
 *
 * Revision 1.5  2000/09/13 22:33:13  steve
 *  undefined macros are null (with warnings.)
 *
 * Revision 1.4  2000/08/20 17:49:04  steve
 *  Clean up warnings and portability issues.
 *
 * Revision 1.3  2000/06/30 15:49:44  steve
 *  Handle errors from parser slightly differently.
 *
 * Revision 1.2  1999/09/05 22:33:18  steve
 *  Take multiple source files on the command line.
 *
 * Revision 1.1  1999/07/03 20:03:47  steve
 *  Add include path and line directives.
 *
 */
#endif
