
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

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include "table_mod.h"
#include "ivl_alloc.h"

static unsigned minimum_columns;
static unsigned indep_columns;
static unsigned dep_column;
static unsigned number_of_columns = 0;
static unsigned cur_columns;
static double *values;
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
  unsigned idx;
  fprintf(stderr, "%#g", values[0]);
  for (idx = 1; idx < indep_columns; idx += 1) {
    fprintf(stderr, ", %#g", values[idx]);
  }
  fprintf(stderr, " => %#g\n", values[indep_columns]);
	    if (number_of_columns) {
		  if (cur_columns < minimum_columns) {
			yyerror("Found %u columns, need at least %u.",
			        cur_columns, minimum_columns);
		  } else if (cur_columns != number_of_columns) {
			yyerror("Found %u columns, expected %u.",
			        cur_columns, number_of_columns);
		  }
	    } else {
		  if (cur_columns < minimum_columns) {
			yyerror("Found %u columns, need at least %u.",
			        cur_columns, minimum_columns);
		  } else number_of_columns = cur_columns;
	    }
      }

columns : REAL
      {
	    values[0] = $1;
	    cur_columns = 1;
      }
       | columns REAL
      {
	    if (cur_columns < indep_columns) values[cur_columns] = $2;
	    else if (cur_columns == dep_column) values[indep_columns] = $2;
	    cur_columns += 1;
      }

%%

int parse_table_model(FILE *fp, const char *file_name, vpiHandle callh,
                      unsigned indep_cols, unsigned dep_col)
{
      assert(indep_cols > 1);
      assert(dep_col > 0);
      indep_columns = indep_cols;
      minimum_columns = indep_cols + dep_col;
      dep_column = minimum_columns - 1;
      values = malloc(sizeof(double)*(indep_cols+1));
      assert(values);
      in_file_name = file_name;
      init_tblmod_lexor(fp);
      yyparse();
      destroy_tblmod_lexor();
      free(values);
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
