
%{
/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
#ident "$Id: lexor.lex,v 1.4 1998/11/23 00:20:23 steve Exp $"
#endif

      //# define YYSTYPE lexval

# include  <iostream.h>
# include  "parse_misc.h"
# include  "parse.h"
# include  <ctype.h>

extern FILE*vl_input;
extern const char*vl_file;

# define YY_USER_INIT reset_lexor();
# define yylval VLlval
extern YYLTYPE yylloc;

static void reset_lexor();
static int check_identifier(const char*name);
static verinum*make_sized_binary(const char*txt);
static verinum*make_sized_octal(const char*txt);
static verinum*make_sized_hex(const char*txt);

%}

%option noyywrap

%x CCOMMENT
%x CSTRING
%s UDPTABLE

%%

[ \t\b\f\r] { ; }
\n { yylloc.first_line += 1; }

"//".* { ; }

"/*" { BEGIN(CCOMMENT); }
<CCOMMENT>.    { yymore(); }
<CCOMMENT>\n   { yylloc.first_line += 1; yymore(); }
<CCOMMENT>"*/" { BEGIN(0); }

"<=" { return K_LE; }
">=" { return K_GE; }
"==" { return K_EQ; }
"!=" { return K_NE; }
"===" { return K_CEQ; }
"!==" { return K_CNE; }

[;:\[\],()#=.@&!<|^~+*/-] { return yytext[0]; }

\"            { BEGIN(CSTRING); }
<CSTRING>\\\" { yymore(); }
<CSTRING>\n   { BEGIN(0);
                yylval.text = new string(yytext, strlen(yytext));
		VLerror(yylloc, "Missing close quote of string.");
		return STRING; }
<CSTRING>\"   { BEGIN(0);
		yylval.text = new string(yytext, strlen(yytext)-1);
		return STRING; }
<CSTRING>.    { yymore(); }

<UDPTABLE>[xXbB01\?] { return yytext[0]; }

[a-zA-Z_][a-zA-Z0-9$_]* {
      int rc = check_identifier(yytext);
      if (rc == IDENTIFIER)
	    yylval.text = new string(yytext);
      else
	    yylval.text = 0;

      return rc; }

\\[^ \t\b\f\r]+         {
      yylval.text = new string(yytext);
      return IDENTIFIER; }

\$([a-zA-Z0-9$_]+)        {
      if (strcmp(yytext,"$attribute") == 0)
	    return KK_attribute;
      yylval.text = new string(yytext);
      return SYSTEM_IDENTIFIER; }

([0-9][0-9_])?\'d[0-9][0-9_]*    { yylval.number = 0;
				   return NUMBER; }
([0-9][0-9_])?\'[bB][0-1xz_]+    { yylval.number = make_sized_binary(yytext);
				   return NUMBER; }
([0-9][0-9_])?\'[oO][0-7xz_]+    { yylval.number = make_sized_octal(yytext);
				   return NUMBER; }
([0-9][0-9_])?\'[hH][0-9a-fA-Fxz_]+ { yylval.number = make_sized_hex(yytext);
				      return NUMBER; }

[0-9][0-9_]*		 {
	/* Handle the special case of the unsized decimal number. */
      unsigned long value = 0;
      for (const char*cp = yytext ;  *cp ;  cp += 1) {
	    if (*cp != '_')
		  value = 10 * value + (*cp - '0');
      }

      unsigned nbits = 8 * sizeof value;
      verinum::V*bits = new verinum::V[8 * sizeof value];

      for (unsigned idx = 0 ;  idx < nbits ;  idx += 1, value >>= 1) {
	    bits[idx] = (value&1) ? verinum::V1 : verinum::V0;
      }

      yylval.number = new verinum(bits, nbits);
      delete[]bits;
      return NUMBER; }

. {   cerr << yylloc.first_line << ": unmatched character (";
      if (isgraph(yytext[0]))
	    cerr << yytext[0];
      else
	    cerr << (unsigned)yytext[0];

      cerr << ")" << endl; }

%%

/*
 * The UDP state table needs some slightly different treatment by the
 * lexor. The level characters are normally accepted as other things,
 * so the parser needs to switch my mode when it believes in needs to.
 */
void lex_start_table()
{
      BEGIN(UDPTABLE);
}

void lex_end_table()
{
      BEGIN(INITIAL);
}

static const struct { const char*name; int code; } key_table[] = {
      { "always", K_always },
      { "and", K_and },
      { "assign", K_assign },
      { "begin", K_begin },
      { "buf", K_buf },
      { "bufif0", K_bufif0 },
      { "bufif1", K_bufif1 },
      { "case", K_case },
      { "casex", K_casex },
      { "casez", K_casez },
      { "cmos", K_cmos },
      { "deassign", K_deassign },
      { "default", K_default },
      { "defparam", K_defparam },
      { "disable", K_disable },
      { "edge", K_edge },
      { "else", K_else },
      { "end", K_end },
      { "endcase", K_endcase },
      { "endfunction", K_endfunction },
      { "endmodule", K_endmodule },
      { "endprimitive", K_endprimitive },
      { "endspecify", K_endspecify },
      { "endtable", K_endtable },
      { "event", K_event },
      { "for", K_for },
      { "force", K_force },
      { "forever", K_forever },
      { "fork", K_fork },
      { "function", K_function },
      { "highz0", K_highz0 },
      { "highz1", K_highz1 },
      { "if", K_if },
      { "initial", K_initial },
      { "inout", K_inout },
      { "input", K_input },
      { "integer", K_integer },
      { "join", K_join },
      { "large", K_large },
      { "macromodule", K_macromodule },
      { "medium", K_medium },
      { "module", K_module },
      { "nand", K_nand },
      { "negedge", K_negedge },
      { "nmos", K_nmos },
      { "nor", K_nor },
      { "not", K_not },
      { "notif0", K_notif0 },
      { "notif1", K_notif1 },
      { "or", K_or },
      { "output", K_output },
      { "pmos", K_pmos },
      { "posedge", K_posedge },
      { "primitive", K_primitive },
      { "pull0", K_pull0 },
      { "pull1", K_pull1 },
      { "pulldown", K_pulldown },
      { "pullup", K_pullup },
      { "rcmos", K_rcmos },
      { "reg", K_reg },
      { "release", K_release },
      { "repeat", K_repeat },
      { "rnmos", K_rnmos },
      { "rpmos", K_rpmos },
      { "rtran", K_rtran },
      { "rtranif0", K_rtranif0 },
      { "rtranif1", K_rtranif1 },
      { "scalered", K_scalered },
      { "small", K_small },
      { "specify", K_specify },
      { "specparam", K_specparam },
      { "strong0", K_strong0 },
      { "strong1", K_strong1 },
      { "supply0", K_supply0 },
      { "supply1", K_supply1 },
      { "table", K_table },
      { "task", K_task },
      { "time", K_time },
      { "tran", K_tran },
      { "tranif0", K_tranif0 },
      { "tranif1", K_tranif1 },
      { "tri", K_tri },
      { "tri0", K_tri0 },
      { "tri1", K_tri1 },
      { "triand", K_triand },
      { "trior", K_trior },
      { "vectored", K_vectored },
      { "wait", K_wait },
      { "wand", K_wand },
      { "weak0", K_weak0 },
      { "weak1", K_weak1 },
      { "while", K_while },
      { "wire", K_wire },
      { "wor", K_wor },
      { "xnor", K_xnor },
      { "xor", K_xor },
      { 0, IDENTIFIER }
};

static int check_identifier(const char*name)
{
      for (unsigned idx = 0 ;  key_table[idx].name ;  idx += 1)
	    if (strcmp(key_table[idx].name, name) == 0)
		  return key_table[idx].code;

      return IDENTIFIER;
}

static verinum*make_sized_binary(const char*txt)
{
      char*ptr;
      unsigned size = strtoul(txt,&ptr,10);
      assert(*ptr == '\'');
      ptr += 1;
      assert(tolower(*ptr) == 'b');

      verinum::V*bits = new verinum::V[size];

      unsigned idx = size;
      char*eptr = ptr + strlen(ptr) - 1;
      while ((eptr > ptr) && (idx < size)) {

	    if (*eptr == '_') {
		  eptr -= 1;
		  continue;
	    }

	    switch (*eptr) {
		case '0':
		  bits[idx++] = verinum::V0;
		  break;
		case '1':
		  bits[idx++] = verinum::V1;
		  break;
		case 'z':
		  bits[idx++] = verinum::Vz;
		  break;
		case 'x':
		  bits[idx++] = verinum::Vx;
		  break;
		default:
		  assert(0);
	    }

	    eptr -= 1;
      }

	// Zero-extend binary number, except that z or x is extended
	// if it is the highest supplied digit.
      while (idx > 0) {
	    switch (ptr[1]) {
		case '0':
		case '1':
		  bits[idx++] = verinum::V0;
		  break;
		case 'z':
		  bits[idx++] = verinum::Vz;
		  break;
		case 'x':
		  bits[idx++] = verinum::Vx;
		  break;
		default:
		  assert(0);
	    }
      }

      return new verinum(bits, size);
}

static verinum*make_sized_octal(const char*txt)
{
      char*ptr;
      unsigned size = strtoul(txt,&ptr,10);
      assert(*ptr == '\'');
      ptr += 1;
      assert(tolower(*ptr) == 'o');

      verinum::V*bits = new verinum::V[size];

      unsigned idx = size;
      char*eptr = ptr + strlen(ptr);

      while ((eptr > ptr) && (idx < (size-3))) {
	    switch (*eptr) {
		case 'x':
		  bits[idx++] = verinum::Vx;
		  bits[idx++] = verinum::Vx;
		  bits[idx++] = verinum::Vx;
		  break;
		case 'z':
		  bits[idx++] = verinum::Vz;
		  bits[idx++] = verinum::Vz;
		  bits[idx++] = verinum::Vz;
		  break;
		default: {
			unsigned val = *eptr - '0';
			bits[idx++] = (val&1)? verinum::V1 : verinum::V0;
			bits[idx++] = (val&2)? verinum::V1 : verinum::V0;
			bits[idx++] = (val&4)? verinum::V1 : verinum::V0;
		  }
	    }

	    eptr -= 1;
      }

	// zero extend octal numbers
      while (idx > 0) switch (ptr[1]) {
	  case 'x':
	    bits[idx++] = verinum::Vx;
	    break;
	  case 'z':
	    bits[idx++] = verinum::Vz;
	    break;
	  default:
	    bits[idx++] = verinum::V0;
      }

      return new verinum(bits, size);
}

static verinum*make_sized_hex(const char*txt)
{
      char*ptr;
      unsigned size = strtoul(txt,&ptr,10);
      assert(*ptr == '\'');
      ptr += 1;
      assert(tolower(*ptr) == 'h');

      verinum::V*bits = new verinum::V[size];

      unsigned idx = size;
      char*eptr = ptr + strlen(ptr);

      while ((eptr > ptr) && (idx < (size-4))) {
	    switch (*eptr) {
		case 'x':
		  bits[idx++] = verinum::Vx;
		  bits[idx++] = verinum::Vx;
		  bits[idx++] = verinum::Vx;
		  bits[idx++] = verinum::Vx;
		  break;
		case 'z':
		  bits[idx++] = verinum::Vz;
		  bits[idx++] = verinum::Vz;
		  bits[idx++] = verinum::Vz;
		  bits[idx++] = verinum::Vz;
		  break;
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		    {
			  unsigned val = tolower(*eptr) - 'a' + 10;
			  bits[idx++] = (val&1)? verinum::V1 : verinum::V0;
			  bits[idx++] = (val&2)? verinum::V1 : verinum::V0;
			  bits[idx++] = (val&4)? verinum::V1 : verinum::V0;
			  bits[idx++] = (val&8)? verinum::V1 : verinum::V0;
			  break;
		    }
		default:
		    {
			  unsigned val = *eptr - '0';
			  bits[idx++] = (val&1)? verinum::V1 : verinum::V0;
			  bits[idx++] = (val&2)? verinum::V1 : verinum::V0;
			  bits[idx++] = (val&4)? verinum::V1 : verinum::V0;
			  bits[idx++] = (val&8)? verinum::V1 : verinum::V0;
		    }
	    }

	    eptr -= 1;
      }

	// zero extend octal numbers
      while (idx > 0) switch (ptr[1]) {
	  case 'x':
	    bits[idx++] = verinum::Vx;
	    break;
	  case 'z':
	    bits[idx++] = verinum::Vz;
	    break;
	  default:
	    bits[idx++] = verinum::V0;
      }

      return new verinum(bits, size);
}

static void reset_lexor()
{
      yyrestart(vl_input);
      yylloc.first_line = 1;
      yylloc.text = vl_file;
}
