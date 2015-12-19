
%{
/*
 *  Copyright (C) 2011-2015  Cary R. (cygcary@yahoo.com)
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

/*
 * The number of parse errors encountered while parsing the data file.
 */
static unsigned errors = 0;

/*
 * The minimum number of columns needed in the data file to satisfy the
 * control string/input variable requirements.
 */
static unsigned minimum_columns;

/*
 * The number of independent values we expect to keep based on the control
 * string/independent variables.
 */
static unsigned indep_values;

/*
 * The total number of independent columns to expect. This includes any
 * ignored columns.
 */
static unsigned indep_columns;

/*
 * The zero based dependent column location. This is just minimum_columns - 1.
 */
static unsigned dep_column;

/*
 * The number of columns to expect in this data file. This must be
 * greater than or equal to the minimum_columns above.
 */
static unsigned number_of_columns = 0;

/*
 * The number of columns that this point currently has.
 */
static unsigned cur_columns;

/*
 * The zero based current independent value we are looking for.
 */
static unsigned cur_value;

/*
 * The usable values for this point. The independent values are first and
 * the dependent value is the last entry.
 */
static double *values;

/*
 * The name of the file we are getting data from.
 */
static const char *in_file_name;

/*
 * The table structure that we are getting data for.
 */
static p_table_mod table_def;

/*
 * The lexor and error routines.
 */
extern int tblmodlex(void);
static void yyerror(const char *fmt, ...);

static void process_point(void)
{
      assert(cur_value == indep_values);
  #if 0
      unsigned idx;
      fprintf(stderr, "%#g", values[0]);
      for (idx = 1; idx < indep_values; idx += 1) {
	    fprintf(stderr, ", %#g", values[idx]);
      }
      fprintf(stderr, " => %#g\n", values[indep_values]);
  #endif
}

%}

%locations

%union {
      double real;
};

%token END_LINE
%token <real> REAL

%%

  /*
   * A table is just a bunch of points. The lexor ignores any comments.
   */
table : point
      | table point
      ;

  /*
   * An individual point is just a bunch of columns followed by one or more
   * end of line characters.
   */
point : columns END_LINE
      {
	      /*
	       * If the number of columns has been defined then we must have
	       * enough columns and we expect every line to have the same
	       * number of columns.
	       */
	    if (number_of_columns) {
		  if (cur_columns < minimum_columns) {
			yyerror("Found %u columns, need at least %u.",
			        cur_columns, minimum_columns);
		  } else if (cur_columns != number_of_columns) {
			yyerror("Found %u columns, expected %u.",
			        cur_columns, number_of_columns);
		  } else process_point();
	      /*
	       * If this is the first point then we must have enough columns.
	       * If there are enough then we define this to be the number of
	       * columns that the file must have.
	       */
	    } else {
		  if (cur_columns < minimum_columns) {
			yyerror("Found %u columns, need at least %u.",
			        cur_columns, minimum_columns);
		  } else {
			number_of_columns = cur_columns;
			process_point();
		  }
	    }
      }
      ;

  /*
   * Each column is a real value. We only save the columns we care about.
   */
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
      ;

%%

/*
 * Setup the various variables, then parse the input file and then cleanup
 * the lexor and the allocated memory.
 */
unsigned parse_table_model(FILE *fp, vpiHandle callh, p_table_mod table)
{
      unsigned rtn = 0;
      assert(table);
      assert(table->fields > 0);
      assert(table->depend > 0);
	/* Initialize the variables used by the parser.  */
      table_def = table;
      indep_values = table->dims;
      indep_columns = table->fields;
      minimum_columns = table->fields + table->depend;
      dep_column = minimum_columns - 1;
      values = malloc(sizeof(double)*(indep_values+1));
      assert(values);
      in_file_name = table->file.name;
	/* Parse the input file. */
      init_tblmod_lexor(fp);
	/* If there are errors then print a message. */
      if (yyparse()) errors += 1;
      if (errors) {
	    vpi_printf("ERROR: %s:%u: ", vpi_get_str(vpiFile, callh),
	               (int) vpi_get(vpiLineNo, callh));
	    vpi_printf("Had %u error(s) while reading table file.\n", errors);
	    rtn = 1;
      }
	/* Cleanup the lexor and allocated memory. */
      destroy_tblmod_lexor();
      free(values);
      return rtn;
}

/*
 * Currently every parse error is reported after the newline so subtract
 * one from the line count.
 */
void yyerror(const char *fmt, ...)
{
      va_list ap;
      va_start(ap, fmt);
      fprintf(stderr, "%s:%d: TABLE ERROR: ", in_file_name,
              yylloc.first_line-1);
      vfprintf(stderr, fmt, ap);
      va_end(ap);
      fprintf(stderr, "\n");
      errors += 1;
}
