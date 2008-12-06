#ifndef __parse_misc_H
#define __parse_misc_H
/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#ident "$Id: parse_misc.h,v 1.7 2002/08/12 01:35:08 steve Exp $"
#endif


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
 * various functions shared by the lexor and the parser.
 */
extern int yylex(void);
extern void yyerror(const char*msg);

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

extern void numbv_init(struct numbv_s*obj);
extern void numbv_add(struct numbv_s*obj, long item);

struct argv_s {
      unsigned  argc;
      vpiHandle*argv;
      char    **syms;
};

extern void argv_init(struct argv_s*obj);
extern void argv_add(struct argv_s*obj, vpiHandle);
extern void argv_sym_add(struct argv_s*obj, char *);
extern void argv_sym_lookup(struct argv_s*obj);

/*
 * $Log: parse_misc.h,v $
 * Revision 1.7  2002/08/12 01:35:08  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.6  2001/07/11 04:43:57  steve
 *  support postpone of $systask parameters. (Stephan Boettcher)
 *
 * Revision 1.5  2001/05/02 23:16:50  steve
 *  Document memory related opcodes,
 *  parser uses numbv_s structures instead of the
 *  symbv_s and a mess of unions,
 *  Add the %is/sub instruction.
 *        (Stephan Boettcher)
 *
 * Revision 1.4  2001/05/01 01:09:39  steve
 *  Add support for memory objects. (Stephan Boettcher)
 *
 * Revision 1.3  2001/03/20 06:16:24  steve
 *  Add support for variable vectors.
 *
 * Revision 1.2  2001/03/18 04:35:18  steve
 *  Add support for string constants to VPI.
 *
 * Revision 1.1  2001/03/11 00:29:39  steve
 *  Add the vvp engine to cvs.
 *
 */
#endif
