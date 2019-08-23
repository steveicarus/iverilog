%option prefix="sdf"
%option never-interactive
%option nounput
%option noinput

%{
/*
 * Copyright (c) 2007-2019 Stephen Williams (steve@icarus.com)
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
%x COND_EDGE_ID
%x EDGE_ID

%%

  /* Skip C++-style comments. */
"//".* { ; }

  /* Skip C-style comments. */
"/*"           { BEGIN(CCOMMENT); }
<CCOMMENT>.    { ; }
<CCOMMENT>\n   { sdflloc.first_line += 1; }
<CCOMMENT>"*/" { BEGIN(0); }

[ \m\t] { /* Skip white space. */; }

  /* Count lines so that the parser can assign line numbers. */
\n { sdflloc.first_line += 1; }

  /* The other edge identifiers. */
<COND_EDGE_ID,EDGE_ID>"01"    {return K_01; }
<COND_EDGE_ID,EDGE_ID>"10"    {return K_10; }
<COND_EDGE_ID,EDGE_ID>"0"[zZ] {return K_0Z; }
<COND_EDGE_ID,EDGE_ID>[zZ]"1" {return K_Z1; }
<COND_EDGE_ID,EDGE_ID>"1"[zZ] {return K_1Z; }
<COND_EDGE_ID,EDGE_ID>[zZ]"0" {return K_Z0; }
<COND_EDGE_ID,EDGE_ID>[pP][oO][sS][eE][dD][gG][eE] {return K_POSEDGE; }
<COND_EDGE_ID,EDGE_ID>[nN][eE][gG][eE][dD][gG][eE] {return K_NEGEDGE; }
<COND_EDGE_ID>[cC][oO][nN][dD] {return K_COND; }

  /* Integer values */
[0-9]+ {
      yylval.int_val = strtoul(yytext, 0, 10);
      return INTEGER;
}

  /* Real values */
[0-9]+(\.[0-9]+)?([Ee][+-]?[0-9]+)? {
      yylval.real_val = strtod(yytext, 0);
      return REAL_NUMBER;
}

([a-zA-Z_]|(\\[^ \t\b\f\r\n]))([a-zA-Z0-9$_]|(\\[^ \t\b\f\r\n]))* {
      return lookup_keyword(yytext);
}

\"[^\"]*\" {
      process_quoted_string();
      return QSTRING;
}

  /* Scalar constants. */
("1"?"'"[bB])?"0" { return K_LOGICAL_ZERO; }
("1"?"'"[bB])?"1" { return K_LOGICAL_ONE; }

  /* Equality operators. */

"=="  { return K_EQ; }
"!="  { return K_NE; }
"==="  { return K_CEQ; }
"!=="  { return K_CNE; }

  /* Other operators. */

"&&"  { return K_LAND; }
"||"  { return K_LOR; }

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
      { "ABSOLUTE",     K_ABSOLUTE },
      { "CELL",         K_CELL },
      { "CELLTYPE",     K_CELLTYPE },
      { "DATE",         K_DATE },
      { "COND",         K_COND },
      { "CONDELSE",     K_CONDELSE },
      { "DELAY",        K_DELAY },
      { "DELAYFILE",    K_DELAYFILE },
      { "DESIGN",       K_DESIGN },
      { "DIVIDER",      K_DIVIDER },
      { "HOLD",         K_HOLD },
      { "INCREMENT",    K_INCREMENT },
      { "INTERCONNECT", K_INTERCONNECT },
      { "INSTANCE",     K_INSTANCE },
      { "IOPATH",       K_IOPATH },
      { "PATHPULSE",    K_PATHPULSE },
      { "PATHPULSEPERCENT", K_PATHPULSEPERCENT },
      { "PERIOD",       K_PERIOD },
      { "PROCESS",      K_PROCESS },
      { "PROGRAM",      K_PROGRAM },
      { "RECREM",       K_RECREM },
      { "RECOVERY",     K_RECOVERY },
      { "REMOVAL",      K_REMOVAL },
      { "SDFVERSION",   K_SDFVERSION },
      { "SETUP",        K_SETUP },
      { "SETUPHOLD",    K_SETUPHOLD },
      { "TEMPERATURE",  K_TEMPERATURE },
      { "TIMESCALE",    K_TIMESCALE },
      { "TIMINGCHECK",  K_TIMINGCHECK },
      { "VENDOR",       K_VENDOR },
      { "VERSION",      K_VERSION },
      { "VOLTAGE",      K_VOLTAGE },
      { "WIDTH",        K_WIDTH },
      { 0, IDENTIFIER }
};

void start_edge_id(unsigned cond)
{
      if (cond) BEGIN(COND_EDGE_ID);
      else BEGIN(EDGE_ID);
}

void stop_edge_id(void)
{
      BEGIN(0);
}

static int lookup_keyword(const char*text)
{
      unsigned idx, len, skip;
      for (idx = 0 ;  keywords[idx].name ;  idx += 1) {
	    if (strcasecmp(text, keywords[idx].name) == 0)
		  return keywords[idx].code;
      }

	/* Process any escaped characters. */
      skip = 0;
      len = strlen(yytext);
      for (idx = 0; idx < len; idx += 1) {
	    if (yytext[idx] == '\\') {
		  skip += 1;
		  idx += 1;
	    }
	    yytext[idx-skip] = yytext[idx];
      }
      yytext[idx-skip] = 0;

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

/*
 * Modern version of flex (>=2.5.9) can clean up the scanner data.
 */
static void destroy_sdf_lexor(void)
{
# ifdef FLEX_SCANNER
#   if YY_FLEX_MAJOR_VERSION >= 2 && YY_FLEX_MINOR_VERSION >= 5
#     if YY_FLEX_MINOR_VERSION > 5 || defined(YY_FLEX_SUBMINOR_VERSION) && YY_FLEX_SUBMINOR_VERSION >= 9
    yylex_destroy();
#     endif
#   endif
# endif
}

extern int sdfparse(void);
void sdf_process_file(FILE*fd, const char*path)
{
      yyrestart(fd);

      sdf_parse_path = path;
      sdfparse();
      destroy_sdf_lexor();
      sdf_parse_path = 0;
}
