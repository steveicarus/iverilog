
%{
/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#ident "$Id: cflexor.lex,v 1.1 2001/11/12 01:26:36 steve Exp $"
#endif

# include  "cfparse.h"
# include  "cfparse_misc.h"
# include  <string.h>

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

%%

  /* Accept C++ style comments. */
"//".* { comment_enter = YY_START; BEGIN(LCOMMENT); }
<LCOMMENT>.    { yymore(); }
<LCOMMENT>\n   { yylloc.first_line += 1; BEGIN(comment_enter); }

  /* Accept C style comments. */
"/*" { comment_enter = YY_START; BEGIN(CCOMMENT); }
<CCOMMENT>.    { yymore(); }
<CCOMMENT>\n   { yylloc.first_line += 1; yymore(); }
<CCOMMENT>"*/" { BEGIN(comment_enter); }

  /* Accept shell type comments. */
^"#".* { ; }

  /* Skip white space. */
[ \t\f] { ; }
  /* Skip line ends, but also count the line. */
\n { yylloc.first_line += 1; }

"-y" { return TOK_Dy; }

"/"[^\*\/].* { cflval.text = strdup(yytext);
             return TOK_STRING; } 

[^/\n \t\b\r-].* { cflval.text = strdup(yytext);
          return TOK_STRING; } 

  /* Fallback match. */
. { return yytext[0]; }

%%

int yywrap()
{
      return 1;
}

void cfreset(FILE*fd)
{
      yyin = fd;
      yyrestart(fd);
      yylloc.first_line = 1;
      yylloc.text = "";
}
