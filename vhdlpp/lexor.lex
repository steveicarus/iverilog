
%option never-interactive
%option nounput
%option noyywrap

%{
/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
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

# include  "parse_api.h"
# include  "lexor_keyword.h"

extern int lexor_keyword_code (const char*str, unsigned len);

/*
 * Lexical location information is passed in the yylloc variable to th
 * parser. The file names, strings, are kept in a list so that I can
 * re-use them. The set_file_name function will return a pointer to
 * the name as it exists in the list (and delete the passed string.)
 * If the name is new, it will be added to the list.
 */
YYLTYPE yylloc;

static int comment_enter;

%}

%x CCOMMENT
%x LCOMMENT

W [ \t\b\f\r]+

%%

[ \t\b\f\r] { ; }
\n { yylloc.first_line += 1; }

  /* Single-line comments start with - - and run to the end of the
     current line. These are very easy to handle. */

"--".* { comment_enter = YY_START; BEGIN(LCOMMENT); }
<LCOMMENT>.    { yymore(); }
<LCOMMENT>\n   { yylloc.first_line += 1; BEGIN(comment_enter); }



  /* The contents of C-style comments are ignored, like white space. */

"/*" { comment_enter = YY_START; BEGIN(CCOMMENT); }
<CCOMMENT>.    { ; }
<CCOMMENT>\n   { yylloc.first_line += 1; }
<CCOMMENT>"*/" { BEGIN(comment_enter); }

[a-zA-Z_][a-zA-Z0-9_]* {
      int rc = lexor_keyword_code(yytext, yyleng);
      return rc;
  }
