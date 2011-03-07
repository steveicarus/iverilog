
%option never-interactive
%option nounput

%{
/*
 * Copyright (c) 2001-2010 Stephen Williams (steve@icarus.com)
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

# include  "parse_misc.h"
# include  "compile.h"
# include  "parse.h"
# include  <string.h>
# include  <assert.h>
%}

%%

  /* These are some special header keywords. */
^":ivl_version" { return K_ivl_version; }
^":vpi_module" { return K_vpi_module; }
^":vpi_time_precision" { return K_vpi_time_precision; }


  /* A label is any non-blank text that appears left justified. */
^[.$_a-zA-Z\\][.$_a-zA-Z\\0-9<>/]* {
      yylval.text = strdup(yytext);
      assert(yylval.text);
      return T_LABEL; }

  /* String tokens are parsed here. Return as the token value the
     contents of the string without the enclosing quotes. */
\"([^\"\\]|\\.)*\" {
      yytext[strlen(yytext)-1] = 0;
      yylval.text = strdup(yytext+1);
      assert(yylval.text);
      return T_STRING; }

  /* Binary vector tokens are parsed here. The result of this is a
     string of binary 4-values in the yylval.vect.text string. This is
     preceded by an 's' if the vector is signed. */
[1-9][0-9]*("'b"|"'sb")[01xz]+ {
      yylval.vect.idx = strtoul(yytext, 0, 10);
      yylval.vect.text = (char*)malloc(yylval.vect.idx + 2);
      assert(yylval.vect.text);
      char*dest = yylval.vect.text;

      const char*bits = strchr(yytext, '\'');
      assert(bits);
      bits += 1;

      if (*bits == 's') {
	    *dest++ = 's';
	    bits += 1;
      }

      assert(*bits == 'b');
      bits += 1;
      unsigned pad = 0;
      if (strlen(bits) < yylval.vect.idx)
	    pad = yylval.vect.idx - strlen(bits);

      memset(dest, '0', pad);
      for (unsigned idx = pad ;  idx < yylval.vect.idx ;  idx += 1)
	    dest[idx] = bits[idx-pad];

      dest[yylval.vect.idx] = 0;
      return T_VECTOR; }


  /* These are some keywords that are recognized. */
".arith/div"    { return K_ARITH_DIV; }
".arith/div.s"  { return K_ARITH_DIV_S; }
".arith/mod"  { return K_ARITH_MOD; }
".arith/mult" { return K_ARITH_MULT; }
".arith/sub"  { return K_ARITH_SUB; }
".arith/sum"  { return K_ARITH_SUM; }
".cmp/eq"   { return K_CMP_EQ; }
".cmp/ne"   { return K_CMP_NE; }
".cmp/ge"   { return K_CMP_GE; }
".cmp/ge.s" { return K_CMP_GE_S; }
".cmp/gt"   { return K_CMP_GT; }
".cmp/gt.s" { return K_CMP_GT_S; }
".decode/adr" { return K_DECODE_ADR; }
".decode/en" { return K_DECODE_EN; }
".demux"    { return K_DEMUX; }
".event"    { return K_EVENT; }
".event/or" { return K_EVENT_OR; }
".functor"  { return K_FUNCTOR; }
".net"      { return K_NET; }
".net/s"    { return K_NET_S; }
".param"    { return K_PARAM; }
".resolv"   { return K_RESOLV; }
".scope"    { return K_SCOPE; }
".shift/l"  { return K_SHIFTL; }
".shift/r"  { return K_SHIFTR; }
".thread"   { return K_THREAD; }
".timescale" { return K_TIMESCALE; }
".ufunc"    { return K_UFUNC; }
".var"      { return K_VAR; }
".var/s"    { return K_VAR_S; }
".var/i"    { return K_VAR_I; /* integer */ }
".word"     { return K_WORD; }
".udp"         { return K_UDP; }
".udp/c"(omb)? { return K_UDP_C; }
".udp/s"(equ)? { return K_UDP_S; }
".mem"	       { return K_MEM; }
".mem/p"(ort)? { return K_MEM_P; }
".mem/i"(nit)? { return K_MEM_I; }
".force"     { return K_FORCE; }

  /* instructions start with a % character. The compiler decides what
     kind of instruction this really is. The few exceptions (that have
     exceptional parameter requirements) are listed first. */

"%vpi_call" { return K_vpi_call; }
"%vpi_func" { return K_vpi_func; }
"%vpi_func/r" { return K_vpi_func_r; }
"%disable"  { return K_disable; }
"%fork"     { return K_fork; }

"%"[.$_/a-zA-Z0-9]+ {
      yylval.text = strdup(yytext);
      assert(yylval.text);
      return T_INSTR; }

[0-9][0-9]* {
      yylval.numb = strtol(yytext, 0, 0);
      return T_NUMBER; }

"0x"[0-9a-fA-F]+ {
      yylval.numb = strtol(yytext, 0, 0);
      return T_NUMBER; }



  /* Symbols are pretty much what is left. They are used to refer to
     labels so the rule must match a string that a label would match. */
[.$_a-zA-Z\\][.$_a-zA-Z\\0-9<>/]* {
      yylval.text = strdup(yytext);
      assert(yylval.text);
      return T_SYMBOL; }

  /* Symbols may include komma `,' in certain constructs */

[A-Z]"<"[.$_a-zA-Z0-9/,]*">" {
      yylval.text = strdup(yytext);
      assert(yylval.text);
      return T_SYMBOL; }


  /* Accept the common assembler style comments, treat them as white
     space. Of course, also skip white space. The semi-colon is
     special, though, in that it is also a statement terminator. */
";".* { return ';'; }
"#".* { ; }

[ \t\b\r] { ; }

\n { yyline += 1; }

. { return yytext[0]; }

%%

int yywrap()
{
      return -1;
}
