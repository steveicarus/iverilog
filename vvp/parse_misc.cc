/*
 * Copyright (c) 2001  Stephen Williams (steve@icarus.com)
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
#ident "$Id: parse_misc.cc,v 1.8 2002/08/12 01:35:08 steve Exp $"
#endif

# include  "parse_misc.h"
# include  "compile.h"
# include  <stdio.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>

const char*yypath;
unsigned yyline;

void yyerror(const char*msg)
{
      fprintf(stderr, "%s:%u: %s\n", yypath, yyline, msg);
}

void symbv_init(struct symbv_s*obj)
{
      obj->cnt = 0;
      obj->vect = 0;
}

void symbv_add(struct symbv_s*obj, struct symb_s item)
{
      obj->vect = (struct symb_s*)
	    realloc(obj->vect, (obj->cnt+1) * sizeof(struct symb_s));
      obj->vect[obj->cnt] = item;
      obj->cnt += 1;
}

void numbv_init(struct numbv_s*obj)
{
      obj->cnt = 0;
      obj->nvec = 0;
}

void numbv_add(struct numbv_s*obj, long item)
{
      obj->nvec = (long*) realloc(obj->nvec, (obj->cnt+1) * sizeof(long));
      obj->nvec[obj->cnt] = item;
      obj->cnt += 1;
}

void argv_init(struct argv_s*obj)
{
      obj->argc = 0;
      obj->argv = 0;
      obj->syms = 0;
}

void argv_add(struct argv_s*obj, vpiHandle item)
{
      obj->argv = (vpiHandle*)
	    realloc(obj->argv, (obj->argc+1)*sizeof(vpiHandle));
      obj->argv[obj->argc] = item;
      obj->argc += 1;
}

void argv_sym_add(struct argv_s*obj, char *item)
{
      argv_add(obj, 0x0);
      obj->syms = (char**)
	    realloc(obj->syms, (obj->argc)*sizeof(char*));
      obj->syms[obj->argc-1] = item;
}

void argv_sym_lookup(struct argv_s*obj)
{
      if (!obj->syms)
	    return;
      for (unsigned i=0; i < obj->argc; i++)
	    if (!obj->argv[i])
		  compile_vpi_lookup(&obj->argv[i], obj->syms[i]);
      free(obj->syms);
}

/*
 * $Log: parse_misc.cc,v $
 * Revision 1.8  2002/08/12 01:35:08  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.7  2001/09/15 18:27:05  steve
 *  Make configure detect malloc.h
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

