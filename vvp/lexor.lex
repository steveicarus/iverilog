%option prefix="yy"
%option never-interactive
%option nounput

%{
/*
 * Copyright (c) 2001-2018 Stephen Williams (steve@icarus.com)
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

# include  "parse_misc.h"
# include  "compile.h"
# include  "parse.h"
# include  <cstring>
# include  <cassert>
# include  "ivl_alloc.h"

# define YY_NO_INPUT

static char* strdupnew(char const *str)
{
      return str ? strcpy(new char [strlen(str)+1], str) : 0;
}

 inline uint64_t strtouint64(const char*str, char**endptr, int base)
{
      if (sizeof(unsigned long) >= sizeof(uint64_t))
	    return strtoul(str, endptr, base);
      else
	    return strtoull(str, endptr, base);
}

%}

%%

  /* These are some special header/footer keywords. */
^":ivl_version" { return K_ivl_version; }
^":ivl_delay_selection" { return K_ivl_delay_selection; }
^":vpi_module" { return K_vpi_module; }
^":vpi_time_precision" { return K_vpi_time_precision; }
^":file_names" { return K_file_names; }


  /* A label is any non-blank text that appears left justified. */
^[.$_a-zA-Z\\][.$_a-zA-Z\\0-9<>/]* {
      yylval.text = strdup(yytext);
      assert(yylval.text);
      return T_LABEL; }

  /* String tokens are parsed here. Return as the token value the
     contents of the string without the enclosing quotes. */
\"([^\"\\]|\\.)*\" {
      yytext[strlen(yytext)-1] = 0;
      yylval.text = strdupnew(yytext+1);
      assert(yylval.text);
      return T_STRING; }

  /* Binary vector tokens are parsed here. The result of this is a
     string of binary 4-values in the yylval.vect.text string. This is
     preceded by an 's' if the vector is signed. */
[1-9][0-9]*("'b"|"'sb")[01xz]+ {
      yylval.vect.idx = strtoul(yytext, 0, 10);
      yylval.vect.text = (char*)malloc(yylval.vect.idx + 2);
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
".abs"          { return K_ARITH_ABS; }
".arith/div"    { return K_ARITH_DIV; }
".arith/div.r"  { return K_ARITH_DIV_R; }
".arith/div.s"  { return K_ARITH_DIV_S; }
".arith/mod"  { return K_ARITH_MOD; }
".arith/mod.r"  { return K_ARITH_MOD_R; }
".arith/mod.s"  { return K_ARITH_MOD_S; }
".arith/mult" { return K_ARITH_MULT; }
".arith/mult.r" { return K_ARITH_MULT_R; }
".arith/pow" { return K_ARITH_POW; }
".arith/pow.r" { return K_ARITH_POW_R; }
".arith/pow.s" { return K_ARITH_POW_S; }
".arith/sub"  { return K_ARITH_SUB; }
".arith/sub.r" { return K_ARITH_SUB_R; }
".arith/sum"  { return K_ARITH_SUM; }
".arith/sum.r"  { return K_ARITH_SUM_R; }
".array"    { return K_ARRAY; }
".array/2s" { return K_ARRAY_2S; }
".array/2u"  { return K_ARRAY_2U; }
".array/i"  { return K_ARRAY_I; }
".array/obj" { return K_ARRAY_OBJ; }
".array/real" { return K_ARRAY_R; }
".array/s" { return K_ARRAY_S; }
".array/str" { return K_ARRAY_STR; }
".array/port" { return K_ARRAY_PORT; }
".cast/2"   { return K_CAST_2; }
".cast/int" { return K_CAST_INT; }
".cast/real" { return K_CAST_REAL; }
".cast/real.s" { return K_CAST_REAL_S; }
".class" { return K_CLASS; }
".cmp/eeq"  { return K_CMP_EEQ; }
".cmp/eqx"  { return K_CMP_EQX; }
".cmp/eqz"  { return K_CMP_EQZ; }
".cmp/eq"   { return K_CMP_EQ; }
".cmp/eq.r" { return K_CMP_EQ_R; }
".cmp/nee"  { return K_CMP_NEE; }
".cmp/ne"   { return K_CMP_NE; }
".cmp/ne.r" { return K_CMP_NE_R; }
".cmp/ge"   { return K_CMP_GE; }
".cmp/ge.r" { return K_CMP_GE_R; }
".cmp/ge.s" { return K_CMP_GE_S; }
".cmp/gt"   { return K_CMP_GT; }
".cmp/gt.r" { return K_CMP_GT_R; }
".cmp/gt.s" { return K_CMP_GT_S; }
".cmp/weq"  { return K_CMP_WEQ; }
".cmp/wne"  { return K_CMP_WNE; }
".concat"   { return K_CONCAT; }
".concat8"  { return K_CONCAT8; }
".delay"    { return K_DELAY; }
".dff/n"      { return K_DFF_N; }
".dff/n/aclr" { return K_DFF_N_ACLR; }
".dff/n/aset" { return K_DFF_N_ASET; }
".dff/p"      { return K_DFF_P; }
".dff/p/aclr" { return K_DFF_P_ACLR; }
".dff/p/aset" { return K_DFF_P_ASET; }
".enum2"    { return K_ENUM2; }
".enum2/s"  { return K_ENUM2_S; }
".enum4"    { return K_ENUM4; }
".enum4/s"  { return K_ENUM4_S; }
".event"    { return K_EVENT; }
".event/or" { return K_EVENT_OR; }
".export"   { return K_EXPORT; }
".extend/s" { return K_EXTEND_S; }
".functor"  { return K_FUNCTOR; }
".import"   { return K_IMPORT; }
".island"   { return K_ISLAND; }
".latch"    { return K_LATCH; }
".modpath"  { return K_MODPATH; }
".net"      { return K_NET; }
".net/2s"   { return K_NET_2S; }
".net/2u"   { return K_NET_2U; }
".net8"     { return K_NET8; }
".net8/2s"  { return K_NET8_2S; }
".net8/2u"  { return K_NET8_2U; }
".net8/s"   { return K_NET8_S; }
".net/real" { return K_NET_R; }
".net/s"    { return K_NET_S; }
".param/l"  { return K_PARAM_L; }
".param/str" { return K_PARAM_STR; }
".param/real" { return K_PARAM_REAL; }
".part"     { return K_PART; }
".part/pv"  { return K_PART_PV; }
".part/v"   { return K_PART_V; }
".part/v.s" { return K_PART_V_S; }
".port"     { return K_PORT; }
".port_info"     { return K_PORT_INFO; }
".reduce/and" { return K_REDUCE_AND; }
".reduce/or"  { return K_REDUCE_OR; }
".reduce/xor" { return K_REDUCE_XOR; }
".reduce/nand" { return K_REDUCE_NAND; }
".reduce/nor"  { return K_REDUCE_NOR; }
".reduce/xnor" { return K_REDUCE_XNOR; }
".repeat"   { return K_REPEAT; }
".resolv"   { return K_RESOLV; }
".rtran"    { return K_RTRAN; }
".rtranif0" { return K_RTRANIF0; }
".rtranif1" { return K_RTRANIF1; }
".scope"    { return K_SCOPE; }
".sfunc"    { return K_SFUNC; }
".sfunc/e"  { return K_SFUNC_E; }
".shift/l"  { return K_SHIFTL; }
".shift/r"  { return K_SHIFTR; }
".shift/rs" { return K_SHIFTRS; }
".substitute" { return K_SUBSTITUTE; }
".thread"   { return K_THREAD; }
".timescale" { return K_TIMESCALE; }
".tran"     { return K_TRAN; }
".tranif0"  { return K_TRANIF0; }
".tranif1"  { return K_TRANIF1; }
".tranvp"   { return K_TRANVP; }
".ufunc/real" { return K_UFUNC_REAL; }
".ufunc/vec4" { return K_UFUNC_VEC4; }
".ufunc/e"  { return K_UFUNC_E; }
".var"      { return K_VAR; }
".var/cobj" { return K_VAR_COBJECT; }
".var/darray" { return K_VAR_DARRAY; }
".var/queue"  { return K_VAR_QUEUE; }
".var/real" { return K_VAR_R; }
".var/s"    { return K_VAR_S; }
".var/str"  { return K_VAR_STR; }
".var/i"    { return K_VAR_I; /* integer */ }
".var/2s"    { return K_VAR_2S; /* byte/shortint/int/longint signed */ }
".var/2u"    { return K_VAR_2U; /* byte/shortint/int/longint unsigned */ }
".udp"         { return K_UDP; }
".udp/c"(omb)? { return K_UDP_C; }
".udp/s"(equ)? { return K_UDP_S; }
"-debug" { return K_DEBUG; }

  /* instructions start with a % character. The compiler decides what
     kind of instruction this really is. The few exceptions (that have
     exceptional parameter requirements) are listed first. */

"%vpi_call"   { return K_vpi_call; }
"%vpi_call/w" { return K_vpi_call_w; }
"%vpi_call/i" { return K_vpi_call_i; }
"%vpi_func"   { return K_vpi_func; }
"%vpi_func/r" { return K_vpi_func_r; }
"%vpi_func/s" { return K_vpi_func_s; }
"%file_line"  { return K_file_line; }

  /* Handle the specialized variable access functions. */

"&A" { return K_A; }
"&APV" { return K_APV; }
"&PV" { return K_PV; }

"%"[.$_/a-zA-Z0-9]+ {
      yylval.text = strdup(yytext);
      assert(yylval.text);
      return T_INSTR; }

[0-9][0-9]* {
      yylval.numb = strtouint64(yytext, 0, 0);
      return T_NUMBER; }

"0x"[0-9a-fA-F]+ {
      yylval.numb = strtouint64(yytext, 0, 0);
      return T_NUMBER; }

  /* Handle some specialized constant/literals as symbols. */

"C4<"[01xz]*">" {
      yylval.text = strdup(yytext);
      assert(yylval.text);
      return T_SYMBOL; }

"C8<"[01234567xz]*">" {
      yylval.text = strdup(yytext);
      assert(yylval.text);
      return T_SYMBOL; }

"Cr<m"[a-f0-9]*"g"[a-f0-9]*">" {
      yylval.text = strdup(yytext);
      assert(yylval.text);
      return T_SYMBOL; }

"S<"[0-9]*",str>" {
      yylval.text = strdup(yytext);
      assert(yylval.text);
      return T_SYMBOL; }

"S<"[0-9]*",vec4,"[us][0-9]+">" {
      yylval.text = strdup(yytext);
      assert(yylval.text);
      return T_SYMBOL; }

"T<"[0-9]*","[0-9]*","[us]">" {
      yylval.text = strdup(yytext);
      assert(yylval.text);
      return T_SYMBOL; }

"W<"[0-9]*","[r]">" {
      yylval.text = strdup(yytext);
      assert(yylval.text);
      return T_SYMBOL; }

 "/INPUT"  { return K_PORT_INPUT; }
 "/OUTPUT" { return K_PORT_OUTPUT; }
 "/INOUT"  { return K_PORT_INOUT; }
 "/MIXED"  { return K_PORT_MIXED; }
 "/NODIR"  { return K_PORT_NODIR; }

  /* Symbols are pretty much what is left. They are used to refer to
     labels so the rule must match a string that a label would match. */
[.$_a-zA-Z\\]([.$_a-zA-Z\\0-9/]|(\\.))* {
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
 * Modern version of flex (>=2.5.9) can clean up the scanner data.
 */
void destroy_lexor()
{
# ifdef FLEX_SCANNER
#   if YY_FLEX_MAJOR_VERSION >= 2 && YY_FLEX_MINOR_VERSION >= 5
#     if YY_FLEX_MINOR_VERSION > 5 || defined(YY_FLEX_SUBMINOR_VERSION) && YY_FLEX_SUBMINOR_VERSION >= 9
    yylex_destroy();
#     endif
#   endif
# endif
}
