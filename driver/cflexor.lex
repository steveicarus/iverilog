
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: cflexor.lex,v 1.11 2007/03/22 16:08:18 steve Exp $"
#endif

# include  "cfparse.h"
# include  "cfparse_misc.h"
# include  "globals.h"
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
 static char* trim_trailing_white(char*txt, int trim);

%}

%x CCOMMENT
%x LCOMMENT
%x PLUS_ARGS
%x FILE_NAME

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
[ \t\f\r] { ; }
  /* Skip line ends, but also count the line. */
\n { yylloc.first_line += 1; }


"+define+" { BEGIN(PLUS_ARGS); return TOK_DEFINE; }

"+incdir+" { BEGIN(PLUS_ARGS); return TOK_INCDIR; }

"+libdir+" { BEGIN(PLUS_ARGS); return TOK_LIBDIR; }

"+libdir-nocase+" { BEGIN(PLUS_ARGS); return TOK_LIBDIR_NOCASE; }

"+libext+" { BEGIN(PLUS_ARGS); return TOK_LIBEXT; }

"+integer-width+" { BEGIN(PLUS_ARGS); return TOK_INTEGER_WIDTH; }

  /* If it is not any known plus-flag, return the generic form. */
"+"[^\n \t\b\f\r+]* {
      cflval.text = strdup(yytext);
      BEGIN(PLUS_ARGS);
      return TOK_PLUSWORD; }

  /* Once in PLUS_ARGS mode, words are delimited by +
     characters. White space and line end terminate PLUS_ARGS mode,
     but + terminates only the word. */
<PLUS_ARGS>[^\n \t\b\f\r+]* {
      cflval.text = strdup(yytext);
      return TOK_PLUSARG; }

  /* Within plusargs, this is a delimiter. */
<PLUS_ARGS>"+" { }

  /* White space end plus_args mode. */
<PLUS_ARGS>[ \t\b\f\r] { BEGIN(0); }

<PLUS_ARGS>\n {
      yylloc.first_line += 1;
      BEGIN(0); }

  /* Notice the -a flag. */
"-a" { return TOK_Da; }

  /* Notice the -v flag. */
"-v" { return TOK_Dv; }

  /* Notice the -y flag. */
"-y" { return TOK_Dy; }

  /* This rule matches paths and strings that may be file names. This
     is a little bit tricky, as we don't want to mistake a comment for
     a string word. */
"/"[\r\n] { /* Special case of file name "/" */
      cflval.text = trim_trailing_white(yytext, 0);
      return TOK_STRING; }
"/"[^\*\/] { /* A file name that starts with "/". */
      yymore();
      BEGIN(FILE_NAME); }
[^/\n \t\b\r+-][^/\n\r]* { /* A file name that starts with other than "/" */
      yymore();
      BEGIN(FILE_NAME); }

<FILE_NAME>"//" {
	/* Found a trailing comment. Returning the terminated name. */
      cflval.text = trim_trailing_white(yytext, 2);
      BEGIN(LCOMMENT);
      return TOK_STRING; }
<FILE_NAME>"/"?[^/\n\r]* {
      yymore();
      /* not a comment... continuing */; }
<FILE_NAME>[\n\r] {
	/* No trailing comment. Return the file name. */
      cflval.text = trim_trailing_white(yytext, 0);
      BEGIN(0);
      return TOK_STRING; }
       
  /* Fallback match. */
. { return yytext[0]; }

%%

static char* trim_trailing_white(char*text, int trim)
{
      char*cp = text + strlen(text);
      while (cp > text && trim > 0) {
	    trim -= 1;
	    cp -= 1;
	    *cp = 0;
      }
      while (cp > text && strchr("\n\r\t\b", cp[-1]))
	    cp -= 1;

      cp[0] = 0;
      return strdup(text);
}

int yywrap()
{
      return 1;
}

void cfreset(FILE*fd, const char*path)
{
      yyin = fd;
      yyrestart(fd);
      yylloc.first_line = 1;
      yylloc.text = (char*)path;
}
