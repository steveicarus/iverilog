
%option never-interactive
%option nounput
%option noinput

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
# include  <string.h>
# include  <strings.h>
# include  <assert.h>

static void process_quoted_string(void);
static int lookup_keyword(const char*text);
const char*sdf_parse_path = 0;

static int yywrap(void)
{
      return 1;
}


# define yylval sdflval
%}

%x CCOMMENT

%%

  /* Skip C++-style comments. */
"//".* { sdflloc.first_line += 1; }

  /* Skip C-style comments. */
"/*"           { BEGIN(CCOMMENT); }
<CCOMMENT>.    { yymore(); }
<CCOMMENT>\n   { sdflloc.first_line += 1; yymore(); }
<CCOMMENT>"*/" { BEGIN(0); }

[ \m\t] { /* Skip white space. */; }

  /* Count lines so that the parser can assign line numbers. */
\n { sdflloc.first_line += 1; }

  /* Real values */
[0-9]+(\.[0-9]+)?([Ee][+-]?[0-9]+)? {
      yylval.real_val = strtod(yytext, 0);
      return REAL_NUMBER;
}

[a-zA-Z_][a-zA-Z0-9$_]* {
      return lookup_keyword(yytext);
}

\"[^\"]*\" {
      process_quoted_string();
      return QSTRING;
}

  /* The HCHAR (hierarchy separator) is set by the SDF file itself. We
     recognize here the HCHAR. */
[./] {
      if (sdf_use_hchar==yytext[0])
	    return HCHAR;
      else
	    return yytext[0];
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
      { "HOLD",       K_HOLD },
      { "INCREMENT",  K_INCREMENT },
      { "INTERCONNECT",K_INTERCONNECT },
      { "INSTANCE",   K_INSTANCE },
      { "IOPATH",     K_IOPATH },
      { "NEGEDGE",    K_NEGEDGE },
      { "POSEDGE",    K_POSEDGE },
      { "PROCESS",    K_PROCESS },
      { "PROGRAM",    K_PROGRAM },
      { "RECOVERY",   K_RECOVERY },
      { "REMOVAL",    K_REMOVAL },
      { "SDFVERSION", K_SDFVERSION },
      { "SETUP",      K_SETUP },
      { "SETUPHOLD",  K_SETUPHOLD },
      { "TEMPERATURE",K_TEMPERATURE },
      { "TIMESCALE",  K_TIMESCALE },
      { "TIMINGCHECK",K_TIMINGCHECK },
      { "VENDOR",     K_VENDOR },
      { "VERSION",    K_VERSION },
      { "VOLTAGE",    K_VOLTAGE },
      { "WIDTH",      K_WIDTH },
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

/*
 * Create a string without the leading and trailing quotes.
 */
static void process_quoted_string(void)
{
      char*endp;

      yylval.string_val = strdup(yytext+1);
      endp = yylval.string_val+strlen(yylval.string_val);
      assert(endp[-1] == '"');
      endp[-1] = 0;
}

extern int sdfparse(void);
void sdf_process_file(FILE*fd, const char*path)
{
      yyrestart(fd);

      sdf_parse_path = path;
      sdfparse();
      sdf_parse_path = 0;
}
