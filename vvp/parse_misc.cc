/*
 * Copyright (c) 2000  Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: parse_misc.cc,v 1.1 2001/03/11 00:29:39 steve Exp $"
#endif

# include  "parse_misc.h"
# include  <stdio.h>
# include  <malloc.h>

const char*yypath;
unsigned yyline;

void yyerror(const char*msg)
{
      fprintf(stderr, "%s:%u: %s\n", yypath, yyline, msg);
}

void textv_init(struct textv_s*obj)
{
      obj->cnt = 0;
      obj->text = 0;
}

void textv_add(struct textv_s*obj, char*item)
{
      obj->text = realloc(obj->text, (obj->cnt+1) * sizeof(char*));
      obj->text[obj->cnt] = item;
      obj->cnt += 1;
}

/*
 * $Log: parse_misc.cc,v $
 * Revision 1.1  2001/03/11 00:29:39  steve
 *  Add the vvp engine to cvs.
 *
 */

