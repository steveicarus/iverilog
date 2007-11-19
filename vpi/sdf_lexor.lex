
%option never-interactive

%{
/*
 * Copyright (c) 2007 Stephen Williams (steve@icarus.com)
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

# include  "sdf_priv.h"
# include  "sdf_parse_priv.h"
# include  "sdf_parse.h"
# include  <stdlib.h>
# include  <strings.h>

static void process_quoted_string(void);
static int lookup_keyword(const char*text);
const char*sdf_parse_path = 0;

static int yywrap(void)
{
      return 1;
}


# define yylval sdflval
%}

%%

[ \m\t] { /* Skip white space. */; }

  /* Count lines so that the parser can assign line numbers. */
\n { sdflloc.first_line += 1; }

  /* Real values */
[0-9]+(\.[0-9]+)?([Ee][+-]?[0-9]+)? {
      yylval.real_val = strtod(yytext, 0);
      return REAL_NUMBER;
}

[a-zA-Z]+ {
      return lookup_keyword(yytext);
}

\"[^\"]*\" {
      process_quoted_string();
      return QSTRING;
}

. { return yytext[0]; }

%%

static struct {
      const char*name;
      int code;
} keywords[] = {
      { "ABSOLUTE",   K_ABSOLUTE },
      { "CELL",       K_CELL },
      { "CELLTYPE",   K_CELLTYPE },
      { "DATE",       K_DATE },
      { "DELAY",      K_DELAY },
      { "DELAYFILE",  K_DELAYFILE },
      { "DESIGN",     K_DESIGN },
      { "DIVIDER",    K_DIVIDER },
      { "INCREMENT",  K_INCREMENT },
      { "INSTANCE",   K_INSTANCE },
      { "IOPATH",     K_IOPATH },
      { "PROCESS",    K_PROCESS },
      { "PROGRAM",    K_PROGRAM },
      { "SDFVERSION", K_SDFVERSION },
      { "TEMPERATURE",K_TEMPERATURE },
      { "TIMESCALE",  K_TIMESCALE },
      { "VENDOR",     K_VENDOR },
      { "VERSION",    K_VERSION },
      { "VOLTAGE",    K_VOLTAGE },
      { 0, IDENTIFIER }
};

static int lookup_keyword(const char*text)
{
      int idx;
      for (idx = 0 ;  keywords[idx].name ;  idx += 1) {
	    if (strcasecmp(text, keywords[idx].name) == 0)
		  return keywords[idx].code;
      }

      yylval.string_val = strdup(yytext);
      return IDENTIFIER;
}

static void process_quoted_string(void)
{
      yylval.string_val = strdup(yytext);
}

extern int sdfparse(void);
void sdf_process_file(FILE*fd, const char*path)
{
      yyrestart(fd);

      sdf_parse_path = path;
      sdfparse();
      sdf_parse_path = 0;
}
