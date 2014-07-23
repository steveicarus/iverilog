#ifndef IVL_table_mod_H
#define IVL_table_mod_H
/*
 *  Copyright (C) 2011-2014  Cary R. (cygcary@yahoo.com)
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

#include <stdio.h>
#include <vpi_user.h>

/*
 * Define the allowed interpolation control values.
 */
#define IVL_CLOSEST_POINT    0 /* D */
#define IVL_LINEAR_INTERP    1 /* 1 - Default */
#define IVL_QUADRATIC_INTERP 2 /* 2 */
#define IVL_CUBIC_INTERP     3 /* 3 */
#define IVL_IGNORE_COLUMN    4 /* I */

/*
 * Define the allowed extrapolation control values.
 */
#define IVL_CONSTANT_EXTRAP 0 /* C */
#define IVL_LINEAR_EXTRAP   1 /* L - Default */
#define IVL_ERROR_EXTRAP    2 /* E */

/*
 * Structure that represents an individual iso line element.
 */
typedef struct t_indep {
      double value;
      struct t_indep *left;  /* previous element. */
      struct t_indep *right; /* next element. */
      union {
	    double data;
	    struct t_indep *child;
	    struct t_build *build;
      } data;
} s_indep, *p_indep;

/*
 * This is used as an interface element while building the iso lines. It will
 * be removed when the list is converted to a balanced threaded tree. It is
 * used to make in order insertion faster.
 */
typedef struct t_build {
      p_indep first;
      p_indep last;
      unsigned count;
} s_build, *p_build;

/*
 * This structure is saved for each table model instance.
 */
typedef struct t_table_mod {
      vpiHandle *indep;     /* Independent variable arguments. */
      double *indep_val;    /* Current independent variable values. */
      union {               /* Data file or argument to get the data file. */
	    char *name;
	    vpiHandle arg;
      } file;
      union {               /* Control string argument. */
	    struct {
		  char *interp;
		  char *extrap_low;
		  char *extrap_high;
	    } info;
	    vpiHandle arg;
      } control;
// HERE need a pointer to the dependent data and the final table.
      unsigned dims;        /* The number of independent variables. */
      unsigned fields;      /* The number of control fields. */
      unsigned depend;      /* Where the dependent column is located. */
      char have_fname;      /* Has the file name been allocated? */
      char have_ctl;        /* Has the file name been allocated? */
} s_table_mod, *p_table_mod;

/*
 * Define the non-static table model routines.
 */

extern unsigned parse_table_model(FILE *fp, vpiHandle callh, p_table_mod table);

extern int tblmodlex(void);
extern void destroy_tblmod_lexor(void);
extern void init_tblmod_lexor(FILE *fp);

#endif /* IVL_table_mod_H */
