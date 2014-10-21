
%{
/*
 * Copyright (c) 1998-2014 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

extern int sdflex(void);
static void yyerror(const char*msg);
# include  "vpi_user.h"
# include  "sdf_parse_priv.h"
# include  "sdf_priv.h"
# include  <stdio.h>
# include  <string.h>
# include  <stdlib.h>
# include  "ivl_alloc.h"

/* This is the hierarchy separator to use. */
char sdf_use_hchar = '.';

%}

%union {
      unsigned long int_val;
      double real_val;
      char*  string_val;

      struct sdf_delay_s delay;
      struct port_with_edge_s port_with_edge;
      struct sdf_delval_list_s delval_list;
};

%token K_ABSOLUTE K_CELL K_CELLTYPE K_COND K_CONDELSE K_DATE K_DELAYFILE
%token K_DELAY K_DESIGN K_DIVIDER K_HOLD K_INCREMENT K_INSTANCE
%token K_INTERCONNECT K_IOPATH K_NEGEDGE K_POSEDGE K_PROCESS K_PROGRAM
%token K_RECREM K_RECOVERY K_REMOVAL K_SDFVERSION K_SETUP K_SETUPHOLD
%token K_TEMPERATURE K_TIMESCALE K_TIMINGCHECK K_VENDOR K_VERSION
%token K_VOLTAGE K_WIDTH
%token K_01 K_10 K_0Z K_Z1 K_1Z K_Z0
%token K_EQ K_NE K_CEQ K_CNE K_LOGICAL_ONE K_LOGICAL_ZERO
%token K_LAND K_LOR

%token HCHAR
%token <string_val> QSTRING IDENTIFIER
%token <real_val> REAL_NUMBER
%token <int_val> INTEGER

%type <string_val> celltype
%type <string_val> cell_instance
%type <string_val> hierarchical_identifier
%type <string_val> port port_instance port_interconnect

%type <real_val> signed_real_number
%type <delay> delval rvalue rtriple signed_real_number_opt

%type <int_val> edge_identifier
%type <port_with_edge> port_edge port_spec

%type <delval_list> delval_list

%left K_LOR
%left K_LAND
%left K_EQ K_NE K_CEQ K_CNE

%%

source_file
  : '(' K_DELAYFILE sdf_header_list cell_list ')'
  | '(' K_DELAYFILE error ')'
      { vpi_printf("%s:%d:SDF ERROR: Invalid DELAYFILE format\n",
		   sdf_parse_path, @2.first_line);
      }
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
      { if (sdf_flag_inform) vpi_printf("%s:%d:SDF INFO: Design: %s\n",
					sdf_parse_path, @2.first_line, $3);
        free($3);
      }
  ;

date
  : '(' K_DATE QSTRING ')'
      { if (sdf_flag_inform) vpi_printf("%s:%d:SDF INFO: Date: %s\n",
					sdf_parse_path, @2.first_line, $3);
        free($3);
      }
  ;

vendor
  : '(' K_VENDOR QSTRING ')'
      { if (sdf_flag_inform) vpi_printf("%s:%d:SDF INFO: Vendor: %s\n",
					sdf_parse_path, @2.first_line, $3);
        free($3);
      }
  ;

program_name
  : '(' K_PROGRAM QSTRING ')'
      { if (sdf_flag_inform) vpi_printf("%s:%d:SDF INFO: Program: %s\n",
					sdf_parse_path, @2.first_line, $3);
        free($3);
      }
  ;

program_version
  : '(' K_VERSION QSTRING ')'
      { if (sdf_flag_inform) vpi_printf("%s:%d:SDF INFO: Program Version: %s\n",
					sdf_parse_path, @2.first_line, $3);
	free($3);
      }
  ;

hierarchy_divider
  : '(' K_DIVIDER '.' ')' { sdf_use_hchar = '.'; }
  | '(' K_DIVIDER '/' ')' { sdf_use_hchar = '/'; }
  | '(' K_DIVIDER HCHAR ')' { /* sdf_use_hchar no-change */; }
  ;

voltage
  : '(' K_VOLTAGE rtriple ')'
  | '(' K_VOLTAGE signed_real_number ')'
  ;

process
  : '(' K_PROCESS QSTRING ')'
      { if (sdf_flag_inform) vpi_printf("%s:%d:SDF INFO: Process: %s\n",
					sdf_parse_path, @2.first_line, $3);
	free($3);
      }
  ;

temperature
  : '(' K_TEMPERATURE rtriple ')'
  | '(' K_TEMPERATURE signed_real_number ')'
  ;

time_scale
  : '(' K_TIMESCALE REAL_NUMBER IDENTIFIER ')'
      { if (sdf_flag_inform) vpi_printf("%s:%d:SDF INFO: TIMESCALE : %f%s\n",
					sdf_parse_path, @2.first_line, $3, $4);
	free($4);
      }
  | '(' K_TIMESCALE INTEGER IDENTIFIER ')'
      { if (sdf_flag_inform) vpi_printf("%s:%d:SDF INFO: TIMESCALE : %lu%s\n",
					sdf_parse_path, @2.first_line, $3, $4);
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
    timing_spec_list_opt
    ')'
      { free($3);
	if ($4) free($4);
      }
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
  | '(' K_INSTANCE ')'
      { $$ = strdup(""); }
  | '(' K_INSTANCE '*' ')'
      { $$ = 0; }
  | '(' K_INSTANCE error ')'
      { vpi_printf("%s:%d:SDF ERROR: Invalid/malformed INSTANCE argument\n",
		   sdf_parse_path, @2.first_line);
	    $$ = strdup(""); }
  ;

timing_spec_list_opt
  : /* Empty */
  | timing_spec_list_opt timing_spec
  ;

timing_spec
  : '(' K_DELAY deltype_list ')'
  | '(' K_DELAY error ')'
      { vpi_printf("%s:%d: Syntax error in CELL DELAY SPEC\n",
		   sdf_parse_path, @2.first_line); }
  | '(' K_TIMINGCHECK tchk_def_list ')'
  | '(' K_TIMINGCHECK error ')'
      { vpi_printf("%s:%d: Syntax error in TIMINGCHECK SPEC\n",
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
    { vpi_printf("%s:%d: SDF ERROR: Invalid/malformed delay type\n",
		 sdf_parse_path, @1.first_line); }
  ;

del_def_list
  : del_def_list del_def
  | del_def
  ;

del_def
  : '(' K_IOPATH port_spec port_instance delval_list ')'
      { sdf_iopath_delays($3.vpi_edge, $3.string_val, $4, &$5);
	free($3.string_val);
	free($4);
      }
  | '(' K_IOPATH error ')'
      { vpi_printf("%s:%d: SDF ERROR: Invalid/malformed IOPATH\n",
		   sdf_parse_path, @2.first_line); }
  | '(' K_COND conditional_port_expr
    '(' K_IOPATH port_spec port_instance delval_list ')' ')'
      { if (sdf_flag_warning) vpi_printf("%s:%d: SDF WARNING: "
					 "COND not supported.\n",
					 sdf_parse_path, @2.first_line);
	free($6.string_val);
	free($7);
      }
  | '(' K_COND QSTRING conditional_port_expr
    '(' K_IOPATH port_spec port_instance delval_list ')' ')'
      { if (sdf_flag_warning) vpi_printf("%s:%d: SDF WARNING: "
					 "COND not supported.\n",
					 sdf_parse_path, @2.first_line);
	free($3);
	free($7.string_val);
	free($8);
      }
  | '(' K_COND error ')'
      { vpi_printf("%s:%d: SDF ERROR: Invalid/malformed COND\n",
		   sdf_parse_path, @2.first_line); }
  | '(' K_CONDELSE '(' K_IOPATH port_spec port_instance delval_list ')' ')'
      { if (sdf_flag_warning) vpi_printf("%s:%d: SDF WARNING: "
					 "CONDELSE not supported.\n",
					 sdf_parse_path, @2.first_line);
	free($5.string_val);
	free($6);
      }
  | '(' K_CONDELSE error ')'
      { vpi_printf("%s:%d: SDF ERROR: Invalid/malformed CONDELSE\n",
		   sdf_parse_path, @2.first_line); }
  /* | '(' K_INTERCONNECT port_instance port_instance delval_list ')' */
  | '(' K_INTERCONNECT port_interconnect port_interconnect delval_list ')'
      { if (sdf_flag_warning) vpi_printf("%s:%d: SDF WARNING: "
					 "INTERCONNECT not supported.\n",
					 sdf_parse_path, @2.first_line);
	free($3);
	free($4);
      }
  | '(' K_INTERCONNECT error ')'
      { vpi_printf("%s:%d: SDF ERROR: Invalid/malformed INTERCONNECT\n",
		   sdf_parse_path, @2.first_line); }
  ;

tchk_def_list
  : tchk_def_list tchk_def
  | tchk_def
  ;

  /* Timing checks are ignored. */
tchk_def
  : '(' K_SETUP port_tchk port_tchk rvalue ')'
  | '(' K_HOLD port_tchk port_tchk rvalue ')'
  | '(' K_SETUPHOLD port_tchk port_tchk rvalue rvalue ')'
  | '(' K_RECOVERY port_tchk port_tchk rvalue ')'
  | '(' K_RECREM port_tchk port_tchk rvalue rvalue ')'
  | '(' K_REMOVAL port_tchk port_tchk rvalue ')'
  | '(' K_WIDTH port_tchk rvalue ')'
  ;

port_tchk
  : port_instance
      { free($1); }
  /* This must only be an edge. For now we just accept everything. */
  | cond_edge_start port_instance ')'
      { free($2); }
  /* These must only be a cond. For now we just accept everything. */
  | cond_edge_start timing_check_condition port_spec ')'
      { free($3.string_val); }
  | cond_edge_start QSTRING timing_check_condition port_spec ')'
      { free($2);
	free($4.string_val);
      }
  ;

cond_edge_start
  : '(' { start_edge_id(1); } cond_edge_identifier { stop_edge_id(); }
  ;

cond_edge_identifier
  : K_POSEDGE
  | K_NEGEDGE
  | K_01
  | K_10
  | K_0Z
  | K_Z1
  | K_1Z
  | K_Z0
  | K_COND
  ;

timing_check_condition
  : port_interconnect
      { free($1); }
  | '~' port_interconnect
      { free($2); }
  | '!' port_interconnect
      { free($2); }
  | port_interconnect equality_operator scalar_constant
      { free($1); }
  ;

  /* This is not complete! */
conditional_port_expr
  : port
      { free($1); }
  | scalar_constant
  | '(' conditional_port_expr ')'
  | conditional_port_expr K_LAND conditional_port_expr
  | conditional_port_expr K_LOR conditional_port_expr
  | conditional_port_expr K_EQ conditional_port_expr
  | conditional_port_expr K_NE conditional_port_expr
  | conditional_port_expr K_CEQ conditional_port_expr
  | conditional_port_expr K_CNE conditional_port_expr
  ;

equality_operator
  : K_EQ
  | K_NE
  | K_CEQ
  | K_CNE
  ;

scalar_constant
  : K_LOGICAL_ONE
  | K_LOGICAL_ZERO
  ;

port_spec
  : port_instance { $$.vpi_edge = vpiNoEdge; $$.string_val = $1; }
  | port_edge     { $$ = $1; }
  ;

port_instance
  : port { $$ = $1; }
  ;

port
  : hierarchical_identifier
      { $$ = $1; }
    /* | hierarchical_identifier '[' INTEGER ']' */
  ;

  /* Since INTERCONNECT is ignored we can also ignore a vector bit. */
port_interconnect
  : hierarchical_identifier
      { $$ = $1; }
  | hierarchical_identifier '[' INTEGER ']'
      { $$ = $1;}
  ;

port_edge
  : '(' {start_edge_id(0);} edge_identifier {stop_edge_id();} port_instance ')'
      { $$.vpi_edge = $3; $$.string_val = $5; }
  ;

edge_identifier
  : K_POSEDGE { $$ = vpiPosedge; }
  | K_NEGEDGE { $$ = vpiNegedge; }
  | K_01      { $$ = vpiEdge01; }
  | K_10      { $$ = vpiEdge10; }
  | K_0Z      { $$ = vpiEdge0x; }
  | K_Z1      { $$ = vpiEdgex1; }
  | K_1Z      { $$ = vpiEdge1x; }
  | K_Z0      { $$ = vpiEdgex0; }
  ;

delval_list
  : delval_list delval
      { int idx;
	$$.count = $1.count;
	for (idx = 0 ; idx < $$.count ; idx += 1)
	      $$.val[idx] = $1.val[idx];
// Is this correct?
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
      { $$.defined = 1;
        $$.value = $2; }
  | '(' rtriple ')'
      { $$ = $2; }
  | '(' ')'
      { $$.defined = 0;
        $$.value = 0.0; }
  ;

hierarchical_identifier
  : IDENTIFIER
      { $$ = $1; }
  | hierarchical_identifier HCHAR IDENTIFIER
      { int len = strlen($1) + strlen($3) + 2;
	char*tmp = realloc($1, len);
	strcat(tmp, ".");
	strcat(tmp, $3);
	free($3);
	$$ = tmp;
      }
  ;

rtriple
  : signed_real_number_opt ':' signed_real_number_opt ':' signed_real_number_opt
      { switch(sdf_min_typ_max) {
	    case _vpiDelaySelMinimum:
	       $$ = $1;
	       break;
	    case _vpiDelaySelTypical:
	       $$ = $3;
	       break;
	    case _vpiDelaySelMaximum:
	       $$ = $5;
	       break;
	}
	  /* At least one of the values must be defined. */
	if (! ($1.defined || $3.defined || $5.defined)) {
	      vpi_printf("%s:%d: SDF ERROR: rtriple must have at least one "
	                 "defined value.\n", sdf_parse_path, @1.first_line);
	}
      }
  ;

signed_real_number_opt
  : /* When missing. */
      { $$.value = 0.0;
	$$.defined = 0;
      }
  | signed_real_number
      { $$.value = $1;
	$$.defined = 1;
      }
  ;

signed_real_number
  :     REAL_NUMBER { $$ = $1; }
  | '+' REAL_NUMBER { $$ = $2; }
  | '-' REAL_NUMBER { $$ = -$2; }
  |     INTEGER { $$ = $1; }
  | '+' INTEGER { $$ = $2; }
  | '-' INTEGER { $$ = -$2; }
  ;

%%

void yyerror(const char*msg)
{
      vpi_printf("%s:SDF ERROR: Too many errors: %s\n", sdf_parse_path, msg);
}
