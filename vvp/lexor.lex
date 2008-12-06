%option never-interactive
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
#ident "$Id: lexor.lex,v 1.43.2.3 2006/05/08 04:33:36 steve Exp $"
#endif

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
".mem" 	       { return K_MEM; }
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


/*
 * $Log: lexor.lex,v $
 * Revision 1.43.2.3  2006/05/08 04:33:36  steve
 *  Update to use only Mingw for build.
 *
 * Revision 1.43.2.2  2006/03/26 23:09:00  steve
 *  Add the .demux device.
 *
 * Revision 1.43.2.1  2006/02/19 00:11:36  steve
 *  Handle synthesis of FF vectors with l-value decoder.
 *
 * Revision 1.43  2004/06/30 02:15:57  steve
 *  Add signed LPM divide.
 *
 * Revision 1.42  2004/06/16 16:33:26  steve
 *  Add structural equality compare nodes.
 *
 * Revision 1.41  2003/08/26 16:26:02  steve
 *  ifdef idents correctly.
 *
 * Revision 1.40  2003/04/11 05:15:39  steve
 *  Add signed versions of .cmp/gt/ge
 *
 * Revision 1.39  2003/03/10 23:37:07  steve
 *  Direct support for string parameters.
 *
 * Revision 1.38  2003/02/09 23:33:26  steve
 *  Spelling fixes.
 *
 * Revision 1.37  2003/01/27 00:14:37  steve
 *  Support in various contexts the $realtime
 *  system task.
 *
 * Revision 1.36  2003/01/25 23:48:06  steve
 *  Add thread word array, and add the instructions,
 *  %add/wr, %cmp/wr, %load/wr, %mul/wr and %set/wr.
 *
 * Revision 1.35  2002/12/21 00:55:58  steve
 *  The $time system task returns the integer time
 *  scaled to the local units. Change the internal
 *  implementation of vpiSystemTime the $time functions
 *  to properly account for this. Also add $simtime
 *  to get the simulation time.
 *
 * Revision 1.34  2002/06/21 04:58:55  steve
 *  Add support for special integer vectors.
 *
 * Revision 1.33  2002/04/14 03:53:20  steve
 *  Allow signed constant vectors for call_vpi parameters.
 *
 * Revision 1.32  2002/03/18 00:19:34  steve
 *  Add the .ufunc statement.
 *
 * Revision 1.31  2002/03/01 05:42:50  steve
 *  out-of-memory asserts.
 *
 * Revision 1.30  2002/02/27 05:46:33  steve
 *  carriage return is white space.
 *
 * Revision 1.29  2002/01/03 04:19:02  steve
 *  Add structural modulus support down to vvp.
 *
 * Revision 1.28  2001/11/01 03:00:19  steve
 *  Add force/cassign/release/deassign support. (Stephan Boettcher)
 *
 * Revision 1.27  2001/10/16 02:47:37  steve
 *  Add arith/div object.
 *
 * Revision 1.26  2001/07/07 02:57:33  steve
 *  Add the .shift/r functor.
 *
 * Revision 1.25  2001/07/06 04:46:44  steve
 *  Add structural left shift (.shift/l)
 *
 * Revision 1.24  2001/06/30 23:03:17  steve
 *  support fast programming by only writing the bits
 *  that are listed in the input file.
 *
 * Revision 1.23  2001/06/18 03:10:34  steve
 *   1. Logic with more than 4 inputs
 *   2. Id and name mangling
 *   3. A memory leak in draw_net_in_scope()
 *   (Stephan Boettcher)
 *
 * Revision 1.22  2001/06/16 23:45:05  steve
 *  Add support for structural multiply in t-dll.
 *  Add code generators and vvp support for both
 *  structural and behavioral multiply.
 *
 * Revision 1.21  2001/06/15 04:07:58  steve
 *  Add .cmp statements for structural comparison.
 *
 * Revision 1.20  2001/06/07 03:09:03  steve
 *  Implement .arith/sub subtraction.
 *
 * Revision 1.19  2001/06/05 03:05:41  steve
 *  Add structural addition.
 *
 * Revision 1.18  2001/05/20 00:46:12  steve
 *  Add support for system function calls.
 *
 * Revision 1.17  2001/05/10 00:26:53  steve
 *  VVP support for memories in expressions,
 *  including general support for thread bit
 *  vectors as system task parameters.
 *  (Stephan Boettcher)
 *
 * Revision 1.16  2001/05/09 02:53:25  steve
 *  Implement the .resolv syntax.
 *
 * Revision 1.15  2001/05/01 01:09:39  steve
 *  Add support for memory objects. (Stephan Boettcher)
 *
 * Revision 1.14  2001/04/24 02:23:59  steve
 *  Support for UDP devices in VVP (Stephen Boettcher)
 *
 * Revision 1.13  2001/04/18 04:21:23  steve
 *  Put threads into scopes.
 *
 * Revision 1.12  2001/04/14 05:10:56  steve
 *  support the .event/or statement.
 *
 * Revision 1.11  2001/04/05 01:34:26  steve
 *  Add the .var/s and .net/s statements for VPI support.
 *
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
