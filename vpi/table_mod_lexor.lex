%option prefix="tblmod"
%option never-interactive
%option noinput
%option nounput
%option noyywrap

%{
/*
 *  Copyright (C) 2011-2017  Cary R. (cygcary@yahoo.com)
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table_mod.h"
#include "table_mod_parse.h"
#include "ivl_alloc.h"

static double get_scaled_real(const char *text);
static double get_real(const char *text);

#define yylval tblmodlval
%}

SPACE [ \t]
EOL [\r\n]+
SCALE [afpnumkKMGT]

%%

  /* Skip comment lines. */
"#".*{EOL} {
	    tblmodlloc.first_line += 1;
      }

  /* Skip white space. */
{SPACE} { ; }

{EOL} {
	    tblmodlloc.first_line += 1;
	    return END_LINE;
      }

  /* Recognize a real value with a Verilog-AMS scale. */
[+-]?[0-9][0-9_]*(\.[0-9][0-9_]*)?{SCALE} {
	    yylval.real = get_scaled_real(yytext);
	    return REAL;
      }

  /* Recognize and other numeric value. */
[+-]?[0-9][0-9_]*(\.[0-9][0-9_]*)?([eE][+-]?[0-9][0-9_]*)? {
	    yylval.real = get_real(yytext);
	    return REAL;
      }

%%

/*
 * Convert a Verilog-AMS scaled real number to a real value.
 */
double get_scaled_real(const char *text)
{
      double result;
      size_t len = strlen(text);
      char *buf = malloc(len + 5);
      const char *scale;

	/* Copy the number to the buffer. */
      strcpy(buf, text);

	/* Convert the scale character to exponential form. */
      switch (text[len-1]) {
	case 'a': scale = "e-18"; break; /* atto */
	case 'f': scale = "e-15"; break; /* femto */
	case 'p': scale = "e-12"; break; /* pico */
	case 'n': scale = "e-9";  break; /* nano */
	case 'u': scale = "e-6";  break; /* micro */
	case 'm': scale = "e-3";  break; /* milli */
	case 'k':
	case 'K': scale = "e3";   break; /* kilo */
	case 'M': scale = "e6";   break; /* mega */
	case 'G': scale = "e9";   break; /* giga */
	case 'T': scale = "e12";  break; /* tera */
	default:
	    scale = "";
	    assert(0);
	    break;
      }
      buf[len-1] = 0;
      strcat(buf, scale);

	/* Convert the exponential string to a real value, */
      result = get_real(buf);

      free(buf);
      return result;
}

/*
 * Convert a normal number to a real value.
 */
double get_real(const char *text)
{
      double result;
      char *buf = malloc(strlen(text) + 1);
      char *cptr = buf;
      char *end;
      unsigned idx;

	/* We must have a buffer to remove any '_' characters. */
      assert(buf);

	/* Remove any '_' characters from the string. */
      for (idx = 0; text[idx]; idx += 1) {
	    if (text[idx] == '_') continue;
	    *cptr = text[idx];
	    cptr += 1;
      }
      *cptr = 0;

	/* Convert the buffer to a double value and check that the
	 * conversion was done successfully. */
      result = strtod(buf, &end);
      assert(*end == 0);

      free(buf);
      return result;
}

void init_tblmod_lexor(FILE *fp)
{
      yyrestart(fp);
}

/*
 * Modern version of flex (>=2.5.9) can clean up the scanner data.
 */
void destroy_tblmod_lexor(void)
{
# ifdef FLEX_SCANNER
#   if YY_FLEX_MAJOR_VERSION >= 2 && YY_FLEX_MINOR_VERSION >= 5
#     if YY_FLEX_MINOR_VERSION > 5 || defined(YY_FLEX_SUBMINOR_VERSION) && YY_FLEX_SUBMINOR_VERSION >= 9
    yylex_destroy();
#     endif
#   endif
# endif
}
