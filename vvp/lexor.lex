
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
#if !defined(WINNT)
#ident "$Id: lexor.lex,v 1.10 2001/04/04 04:33:08 steve Exp $"
#endif

# include  "parse_misc.h"
# include  "compile.h"
# include  "parse.h"
# include  <string.h>
%}

%%

  /* These are some special header keywords. */
^":vpi_module" { return K_vpi_module; }


  /* A label is any non-blank text that appears left justified. */
^[.$_a-zA-Z][.$_a-zA-Z0-9<>]* {
      yylval.text = strdup(yytext);
      return T_LABEL; }

  /* String tokens are parsed here. Return as the token value the
     contents of the string without the enclosing quotes. */
\"[^\"]*\" {
      yytext[strlen(yytext)-1] = 0;
      yylval.text = strdup(yytext+1);
      return T_STRING; }

[1-9][0-9]*"'b"[01xz]+ {
      yylval.vect.idx = strtoul(yytext, 0, 10);
      yylval.vect.text = (char*)malloc(yylval.vect.idx + 1);

      const char*bits = strchr(yytext, 'b');
      bits += 1;
      unsigned pad = 0;
      if (strlen(bits) < yylval.vect.idx)
	    pad = yylval.vect.idx - strlen(bits);

      memset(yylval.vect.text, '0', pad);
      for (unsigned idx = pad ;  idx < yylval.vect.idx ;  idx += 1)
	    yylval.vect.text[idx] = bits[idx-pad];

      yylval.vect.text[yylval.vect.idx] = 0;
      return T_VECTOR; }


  /* These are some keywords that are recognized. */
".event"   { return K_EVENT; }
".functor" { return K_FUNCTOR; }
".net"     { return K_NET; }
".scope"   { return K_SCOPE; }
".thread"  { return K_THREAD; }
".var"     { return K_VAR; }


  /* instructions start with a % character. The compiler decides what
     kind of instruction this really is. The few exceptions (that have
     exceptional parameter requirements) are listed first. */

"%vpi_call" {
      return K_vpi_call; }

"%"[.$_/a-zA-Z0-9]+ {
      yylval.text = strdup(yytext);
      return T_INSTR; }

[0-9][0-9]* {
      yylval.numb = strtol(yytext, 0, 0);
      return T_NUMBER; }

"0x"[0-9a-fA-F]+ {
      yylval.numb = strtol(yytext, 0, 0);
      return T_NUMBER; }



  /* Symbols are pretty much what is left. They are used to refer to
     labels so the rule must match a string that a label would match. */
[.$_a-zA-Z][.$_a-zA-Z0-9<>]* {
      yylval.text = strdup(yytext);
      return T_SYMBOL; }


  /* Accept the common assembler style comments, treat them as white
     space. Of course, also skip white space. The semi-colon is
     special, though, in that it is also a statement terminator. */
";".* { return ';'; }
"#".* { ; }

[ \t\b] { ; }

\n { yyline += 1; }

. { return yytext[0]; }

%%

int yywrap()
{
      return -1;
}


/*
 * $Log: lexor.lex,v $
 * Revision 1.10  2001/04/04 04:33:08  steve
 *  Take vector form as parameters to vpi_call.
 *
 * Revision 1.9  2001/03/26 04:00:39  steve
 *  Add the .event statement and the %wait instruction.
 *
 * Revision 1.8  2001/03/25 19:36:45  steve
 *  Accept <> characters in labels and symbols.
 *
 * Revision 1.7  2001/03/25 00:35:35  steve
 *  Add the .net statement.
 *
 * Revision 1.6  2001/03/23 02:40:22  steve
 *  Add the :module header statement.
 *
 * Revision 1.5  2001/03/20 02:48:40  steve
 *  Copyright notices.
 *
 */
