%{
/*
 * Copyright (c) 20001 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: cfparse.y,v 1.2 2001/11/12 18:47:32 steve Exp $"
#endif


# include  "globals.h"

%}

%union {
      char*text;
};

%token TOK_Da TOK_Dv TOK_Dy
%token <text> TOK_INCDIR TOK_PLUSARG TOK_STRING

%%

start
	:
	| item_list
	;

item_list
	: item_list item
	| item
	;

item    : TOK_STRING
		{ process_file_name($1);
		  free($1);
		}
        | TOK_Da
		{ }
        | TOK_Dv TOK_STRING
		{ process_file_name($2);
		  fprintf(stderr, "%s:%u: Ignoring -v in front of %s\n",
			  @1.text, @1.first_line, $2);
		  free($2);
		}
        | TOK_Dy TOK_STRING
		{ process_library_switch($2);
		  free($2);
		}
	| TOK_INCDIR
		{ process_include_dir($1);
		  free($1);
		}
	| TOK_PLUSARG
		{ fprintf(stderr, "%s:%u: Ignoring %s\n",
			  @1.text, @1.first_line, $1);
		  free($1);
		}
	;

%%

int yyerror(const char*msg)
{
}
