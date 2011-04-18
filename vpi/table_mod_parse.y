
%{
/*
 *  Copyright (C) 2011  Cary R. (cygcary@yahoo.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdarg.h>
#include <stdio.h>
#include "table_mod.h"

static unsigned minimum_columns;
static unsigned number_of_columns = 0U;
static unsigned cur_columns;
static const char *in_file_name;

extern int tblmodlex(void);
static void yyerror(const char *fmt, ...);

%}

%locations

%union {
      double real;
};

%token EOL
%token <real> REAL

%%

table : point
      | table point

point : columns EOL
      {
  fprintf(stderr, "\n");
	    if (number_of_columns) {
		  if (cur_columns != number_of_columns) {
			yyerror("Found %u columns, expected %u.",
			        cur_columns, number_of_columns);
		  }
	    } else {
		  if (cur_columns < minimum_columns) {
			yyerror("Found %u columns, need at least %u.",
			        cur_columns, minimum_columns);
		  }
		  number_of_columns = cur_columns;
	    }
      }

columns : REAL
      {
  fprintf(stderr, "%#g", $1);
	    cur_columns = 1U;
      }
       | columns REAL
      {
  fprintf(stderr, ", %#g", $2);
	    cur_columns += 1U;
      }

%%

int parse_table_model(FILE *fp, const char *file_name, vpiHandle callh,
                      unsigned min_cols)
{
      minimum_columns = min_cols;
      in_file_name = file_name;
      init_tblmod_lexor(fp);
      yyparse();
      destroy_tblmod_lexor();
// HERE: Need to handle errors.
      return 1;
}

void yyerror(const char *fmt, ...)
{
      va_list ap;
      va_start(ap, fmt);
      fprintf(stderr, "%s:%u: TABLE ERROR: ", in_file_name, yylloc.first_line);
      vfprintf(stderr, fmt, ap);
      fprintf(stderr, "\n");

}
