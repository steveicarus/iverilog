%option prefix="fp"
%option never-interactive
%option noinput
%option nounput
%option noyywrap

%{
/*
 *  Copyright (C) 2011-2017 Stephen Williams (steve@icarus.com)
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

# include  "fp_api.h"
# include  "fp.h"

# define YY_DECL int yylex(YYSTYPE*yylvalp, YYLTYPE*yyllocp)

%}

SPACE [ \t\f\r]

%%

  /* Skip comment lines */
"#".* { ; }


  /* Skip white space */
{SPACE} { ; }
"\n" { yyllocp->first_line += 1; }

"Element" { return K_ELEMENT; }
"Pad"     { return K_PAD; }

"0x"[0-9a-fA-F]+ {
      yylvalp->integer = strtoul(yytext+2,0,10);
      return INTEGER;
}

"0"[0-7]* {
      yylvalp->integer = strtoul(yytext,0,8);
      return INTEGER;
}

[1-9][0-9]* {
      yylvalp->integer = strtoul(yytext,0,10);
      return INTEGER;
}

"\""[^\"]*"\"" {
      size_t len = strlen(yytext)-2;
      char*tmp = new char[len+1];
      memcpy(tmp, yytext+1, len);
      tmp[len] = 0;
      yylvalp->text = tmp;
      return STRING;
}

  /* Isolated characters are tokens */
. { return yytext[0]; }

%%

void init_fp_lexor(FILE*fd)
{
      yyrestart(fd);
}

/*
 * Modern version of flex (>=2.5.9) can clean up the scanner data.
 */
void destroy_fp_lexor()
{
# ifdef FLEX_SCANNER
#   if YY_FLEX_MAJOR_VERSION >= 2 && YY_FLEX_MINOR_VERSION >= 5
#     if YY_FLEX_MINOR_VERSION > 5 || defined(YY_FLEX_SUBMINOR_VERSION) && YY_FLEX_SUBMINOR_VERSION >= 9
      yylex_destroy();
#     endif
#   endif
# endif
}
