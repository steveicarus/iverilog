
%{
/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: lexor.lex,v 1.1 2000/10/08 22:36:56 steve Exp $"
#endif

# include  <string.h>
# include  "parse.h"

%}

%x CTOKENS
%x PATTERN

%%

"#".* { /* eat comments */; }

"\n" { /* eat line-ends */; }

"[" { BEGIN(CTOKENS); return '['; }

<CTOKENS>"]" { BEGIN(0); return ']'; }

<CTOKENS>[ \t\b]+ { /* skip white space */; }

<CTOKENS>"-S" { return CT_S; }

<CTOKENS>"-t"[a-zA-Z0-9]+ {
      yylval.text = strdup(yytext+2);
      return CT_t; }

"<"[^>]*">" {
      BEGIN(PATTERN);
      yylval.text = strdup(yytext);
      return PATTERN_NAME; }

<PATTERN>.* {
      BEGIN(0);
      yylval.text = strdup(yytext);
      return PATTERN_TEXT; }

. { fprintf(stderr, "driver lexor: Unmatched character: %c\n", yytext[0]); }

%%

void reset_lexor(FILE*fd)
{
      yyrestart(fd);
}

int yywrap()
{
      return 1;
}
