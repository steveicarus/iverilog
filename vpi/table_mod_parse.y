
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

static unsigned errors = 0;
static unsigned minimum_columns;
static unsigned indep_values;
static unsigned indep_columns;
static unsigned dep_column;
static unsigned number_of_columns = 0;
static unsigned cur_columns;
static unsigned cur_value;
static double *values;
static const char *in_file_name;
static p_table_mod table_def;

extern int tblmodlex(void);
static void yyerror(const char *fmt, ...);

%}

%locations

%union {
      double real;
};

%token END_LINE
%token <real> REAL

%%

table : point
      | table point

point : columns END_LINE
      {
  #if 0
    unsigned idx;
    fprintf(stderr, "%#g", values[0]);
    for (idx = 1; idx < indep_values; idx += 1) {
      fprintf(stderr, ", %#g", values[idx]);
    }
    fprintf(stderr, " => %#g\n", values[indep_values]);
  #endif
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
	    if (table_def->control.info.interp[0] != IVL_IGNORE_COLUMN) {
		  values[0] = $1;
		  cur_value = 1;
	    } else cur_value = 0;
	    cur_columns = 1;
      }
       | columns REAL
      {
	    if (cur_columns < indep_columns) {
		  if (table_def->control.info.interp[cur_columns] != 
		      IVL_IGNORE_COLUMN) {
			values[cur_value] = $2;
			cur_value += 1;
		  }
	    } else if (cur_columns == dep_column) values[indep_values] = $2;
	    cur_columns += 1;
      }

%%

unsigned parse_table_model(FILE *fp, vpiHandle callh, p_table_mod table)
{
      unsigned rtn = 0;
      assert(table);
      assert(table->fields > 0);
      assert(table->depend > 0);
      table_def = table;
      indep_values = table->dims;
      indep_columns = table->fields;
      minimum_columns = table->fields + table->depend;
      dep_column = minimum_columns - 1;
      values = malloc(sizeof(double)*(indep_values+1));
      assert(values);
      in_file_name = table->file.name;
      init_tblmod_lexor(fp);
      if (yyparse()) errors += 1;
      if (errors) {
	    vpi_printf("ERROR: %s:%u: ", vpi_get_str(vpiFile, callh),
	               (int) vpi_get(vpiLineNo, callh));
	    vpi_printf("Had %u error(s) while reading table file.\n", errors);
	    rtn = 1;
      }
      destroy_tblmod_lexor();
      free(values);
      return rtn;
}

/*
 * Because every parse error is reported after the newline subtract one from
 * the line count.
 */
void yyerror(const char *fmt, ...)
{
      va_list ap;
      va_start(ap, fmt);
      fprintf(stderr, "%s:%u: TABLE ERROR: ", in_file_name,
              yylloc.first_line-1);
      vfprintf(stderr, fmt, ap);
      fprintf(stderr, "\n");
      errors += 1;
}
