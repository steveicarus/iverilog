
%{
/*
 * Copyright (c) 1998-2007 Stephen Williams (steve@icarus.com)
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

extern int sdflex(void);
static void yyerror(const char*msg);
# include  "vpi_user.h"
# include  "sdf_parse_priv.h"
# include  "sdf_priv.h"
# include  <stdio.h>

/* This is the hierarchy separator to use. */
static char use_hchar = '.';

%}

%union {
      unsigned long int_val;
      double real_val;
      char*  string_val;

      struct sdf_delval_list_s delval_list;
};

%token K_ABSOLUTE K_CELL K_CELLTYPE K_DATE K_DELAYFILE K_DELAY K_DESIGN
%token K_DIVIDER K_INCREMENT K_INSTANCE K_IOPATH
%token K_PROCESS K_PROGRAM K_SDFVERSION K_TEMPERATURE K_TIMESCALE
%token K_VENDOR K_VERSION K_VOLTAGE

%token <string_val> QSTRING IDENTIFIER
%token <real_val> REAL_NUMBER
%token <int_val> INTEGER

%type <string_val> celltype
%type <string_val> cell_instance
%type <string_val> hierarchical_identifier
%type <string_val> port port_instance port_spec

%type <real_val> rvalue rtriple signed_real_number
%type <real_val> delval

%type <delval_list> delval_list

%%

source_file
  : '(' K_DELAYFILE sdf_header_list cell_list ')'
  ;

sdf_header_list
  : sdf_header_list sdf_header_item
  | sdf_header_item
  ;

sdf_header_item
  : sdfversion
  | design_name
  | date
  | vendor
  | program_name
  | program_version
  | hierarchy_divider
  | voltage
  | process
  | temperature
  | time_scale
  ;

sdfversion
  : '(' K_SDFVERSION QSTRING ')'
    { free($3);
    }
  ;

design_name
  : '(' K_DESIGN QSTRING ')'
    { vpi_printf("SDF Design: %s\n", $3);
      free($3);
    }
  ;

date
  : '(' K_DATE QSTRING ')'
    { vpi_printf("SDF Date: %s\n", $3);
      free($3);
    }
  ;

vendor : '(' K_VENDOR QSTRING ')'
    { vpi_printf("SDF Vendor: %s\n", $3);
      free($3);
    }
;

program_name : '(' K_PROGRAM QSTRING ')'
    { vpi_printf("SDF Program: %s\n", $3);
      free($3);
    }
;

program_version : '(' K_VERSION QSTRING ')'
    { vpi_printf("SDF Program Version: %s\n", $3);
      free($3);
    }
;

hierarchy_divider
  : '(' K_DIVIDER '.' ')' { use_hchar = '.'; }
  | '(' K_DIVIDER '/' ')' { use_hchar = '/'; }
  ;

voltage
  : '(' K_VOLTAGE rtriple ')'
  | '(' K_VOLTAGE signed_real_number ')'
  ;

process : '(' K_PROCESS QSTRING ')'
    { vpi_printf("SDF Process: %s\n", $3);
      free($3);
    }
;

temperature
  : '(' K_TEMPERATURE rtriple ')'
  | '(' K_TEMPERATURE signed_real_number ')'
  ;

time_scale
  : '(' K_TIMESCALE REAL_NUMBER IDENTIFIER ')'
    { vpi_printf("SDF TIMESCALE : %f%s\n", $3, $4);
      free($4);
    }
  ;

cell_list
  : cell_list cell
  | cell
  ;

cell
  : '(' K_CELL celltype cell_instance
      { sdf_select_instance($3, $4); /* find the instance in the design */}
    timing_spec_list
    ')'
      { free($3);
	free($4); }
  | '(' K_CELL error ')'
      { vpi_printf("%s:%d: Syntax error in CELL\n",
		   sdf_parse_path, @2.first_line); }
  ;

celltype
  : '(' K_CELLTYPE QSTRING ')'
      { $$ = $3; }
  ;

cell_instance
  : '(' K_INSTANCE hierarchical_identifier ')'
      { $$ = $3; }
  | '(' K_INSTANCE '*' ')'
      { $$ = 0; }
  ;

timing_spec_list
  : timing_spec_list timing_spec
  | timing_spec
  ;

timing_spec
  : '(' K_DELAY deltype_list ')'
  | '(' K_DELAY error ')'
    { vpi_printf("%s:%d: Syntax error in CELL DELAY SPEC\n",
		 sdf_parse_path, @2.first_line); }
  ;

deltype_list
  : deltype_list deltype
  | deltype
  ;

deltype
  : '(' K_ABSOLUTE del_def_list ')'
  | '(' K_INCREMENT del_def_list ')'
  | '(' error ')'
    { vpi_printf("%s:%d: Invalid/malformed delay type\n",
		 sdf_parse_path, @1.first_line); }
  ;

del_def_list
  : del_def_list del_def
  | del_def
  ;

del_def
  : '(' K_IOPATH port_spec port_instance delval_list ')'
      { sdf_iopath_delays($3, $4, &$5);
	free($3);
	free($4);
      }
  | '(' K_IOPATH error ')'
      { vpi_printf("%s:%d: Invalid/malformed IOPATH\n",
		   sdf_parse_path, @2.first_line); }
  ;

port_spec
  : port_instance
    /*  | port_edge */
  ;

port_instance
  : port { $$ = $1; }
  ;

port
  : hierarchical_identifier
      { $$ = $1; }
    /* | hierarchical_identifier '[' INTEGER ']' */
  ;

delval_list
  : delval_list delval
      { int idx;
	$$.count = $1.count;
	for (idx = 0 ; idx < $$.count ; idx += 1)
	      $$.val[idx] = $1.val[idx];
	if ($$.count < 12) {
	      $$.val[$$.count] = $2;
	      $$.count += 1;
	}
      }
  | delval
      { $$.count = 1;
	$$.val[0] = $1;
      }
  ;

delval
  : rvalue
      { $$ = $1; }
  | '(' rvalue rvalue ')'
      { $$ = $2;
	vpi_printf("%s:%d: SDF WARNING: Pulse rejection limits ignored\n",
		   sdf_parse_path, @3.first_line);
      }
  | '(' rvalue rvalue rvalue ')'
      { $$ = $2;
	vpi_printf("%s:%d: SDF WARNING: Pulse rejection limits ignored\n",
		   sdf_parse_path, @3.first_line);
      }
  ;

rvalue
  : '(' signed_real_number ')'
      { $$ = $2; }
  | '(' rtriple ')'
      { $$ = $2; }
  ;

hierarchical_identifier
  : IDENTIFIER
    { $$ = $1; }
  ;

rtriple
  : signed_real_number ':' signed_real_number ':' signed_real_number
      { $$ = $3; /* XXXX Assume typical value. */ }
  ;

signed_real_number
  :     REAL_NUMBER { $$ = $1; }
  | '+' REAL_NUMBER { $$ = $2; }
  | '-' REAL_NUMBER { $$ = -$2; }
  ;

%%

void yyerror(const char*msg)
{
      fprintf(stderr, "SDF ERROR: %s\n", msg);
}
