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
#if !defined(WINNT)
#ident "$Id: main.c,v 1.1 1999/07/03 17:24:11 steve Exp $"
#endif

# include  <stdio.h>

extern void reset_lexor(const char*);
extern int yylex();

int main(int argc, char*argv[])
{
      if (argc != 2) {
	    fprintf(stderr, "Usage: %s <file>\n", argv[0]);
	    return 1;
      }

      reset_lexor(argv[1]);

      return yyparse();
}

/*
 * $Log: main.c,v $
 * Revision 1.1  1999/07/03 17:24:11  steve
 *  Add a preprocessor.
 *
 */

