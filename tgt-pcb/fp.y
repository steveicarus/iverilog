
%pure-parser
%parse-param {const char*file_path}

%{
/*
 * Copyright (c) 2011-2013 Stephen Williams (steve@icarus.com)
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

# include  "fp_api.h"
# include  "pcb_priv.h"
# include  <cstdio>
# include  <cstdarg>


using namespace std;

/* Recent version of bison expect that the user supply a
   YYLLOC_DEFAULT macro that makes up a yylloc value from existing
   values. I need to supply an explicit version to account for the
   text field, that otherwise won't be copied. */
# define YYLLOC_DEFAULT(Current, Rhs, N)  do {       \
  (Current).first_line   = (Rhs)[1].first_line;      \
 } while (0)

static void yyerror(YYLTYPE*, const char*, const char*msg)
{
      fprintf(stderr, "%s\n", msg);
}

extern void init_fp_lexor(FILE*fd);
extern void destroy_fp_lexor();
extern int fplex(union YYSTYPE*yylvalp, YYLTYPE*yylloc);

static fp_pad_t cur_pad;
static fp_element_t cur_element;

%}

%union {
      long integer;
      char*text;
};

%token <text> STRING
%token <integer> INTEGER
%token K_ELEMENT
%token K_PAD

%type <integer> integer

%%

file_items : file_items file_item | file_item ;

file_item : element ;

element
  : K_ELEMENT element_header '(' element_items ')'
      { callback_fp_element(cur_element); }
  ;

element_header
  : '[' INTEGER STRING STRING STRING integer integer integer integer integer integer STRING ']'
      { cur_element.nflags = $2;
	cur_element.description = $3;  delete[]$3;
	cur_element.name = $4;         delete[]$4;
	cur_element.value = $5;        delete[]$5;
	cur_element.mx = $6;
	cur_element.my = $7;
	cur_element.tx = $8;
	cur_element.ty = $9;
	cur_element.tdir = $10;
	cur_element.tscale = $11;
	cur_element.tsflags = $12;     delete[]$12;
	cur_element.pads.clear();
      }
  | '[' error ']'
      { errormsg(@2, "Error in element header\n");
	yyerrok;
        cur_element.nflags = 0;
	cur_element.description = "";
	cur_element.name = "";
	cur_element.value = "";
	cur_element.mx = 0;
	cur_element.my = 0;
	cur_element.tx = 0;
	cur_element.ty = 0;
	cur_element.tdir = 0;
	cur_element.tscale = 0;
	cur_element.tsflags = "";
	cur_element.pads.clear();
      }
  ;

element_items
  : element_items element_item
  | element_item
  ;

element_item
  : pad  { cur_element.pads[cur_pad.name] = cur_pad; }
  ;

integer
  : INTEGER      { $$ = $1; }
  | '-' INTEGER  { $$ = -$2; }
  ;

pad
  : K_PAD '[' integer integer integer integer integer integer integer STRING STRING STRING ']'
      { cur_pad.rx1 = $3;
	cur_pad.ry1 = $4;
	cur_pad.rx2 = $5;
	cur_pad.ry2 = $6;
	cur_pad.thickness = $7;
	cur_pad.clearance = $8;
	cur_pad.mask = $9;
	cur_pad.name = $10;   delete[]$10;
	cur_pad.number = $11; delete[]$11;
	cur_pad.sflags = $12; delete[]$12;
      }

  | K_PAD '[' error ']'
      { errormsg(@3, "Error in pad header\n");
	yyerrok;
	cur_pad.rx1 = 0;
	cur_pad.ry1 = 0;
	cur_pad.rx2 = 0;
	cur_pad.ry2 = 0;
	cur_pad.thickness = 0;
	cur_pad.clearance = 0;
	cur_pad.mask = 0;
	cur_pad.name = "";
	cur_pad.number = "";
	cur_pad.sflags = "";
      }
  ;

%%

static string parse_file_path;
int parse_fp_errors = 0;
int parse_fp_sorrys = 0;

void errormsg(const YYLTYPE&loc, const char*fmt, ...)
{
      va_list ap;
      va_start(ap, fmt);

      fprintf(stderr, "%s:%u: error: ", parse_file_path.c_str(), loc.first_line);
      vfprintf(stderr, fmt, ap);
      va_end(ap);
      parse_fp_errors += 1;
}

void sorrymsg(const YYLTYPE&loc, const char*fmt, ...)
{
      va_list ap;
      va_start(ap, fmt);

      fprintf(stderr, "%s:%u: sorry: ", parse_file_path.c_str(), loc.first_line);
      vfprintf(stderr, fmt, ap);
      va_end(ap);
      parse_fp_sorrys += 1;
}

int parse_fp_file(const string&file_path)
{
      FILE*fd = fopen(file_path.c_str(), "r");
      if (fd == 0) {
	    perror(file_path.c_str());
	    return -1;
      }

      parse_file_path = file_path;
      parse_fp_errors = 0;
      parse_fp_sorrys = 0;
      init_fp_lexor(fd);
      int rc = yyparse(file_path.c_str());
      fclose(fd);
      destroy_fp_lexor();

      return rc;
}
