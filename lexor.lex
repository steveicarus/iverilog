
%{
/*
 * Copyright (c) 1998-2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: lexor.lex,v 1.69 2002/02/15 05:20:58 steve Exp $"
#endif

# include "config.h"

      //# define YYSTYPE lexval

# include  <iostream.h>
# include  "compiler.h"
# include  "parse_misc.h"
# include  "parse_api.h"
# include  "parse.h"
# include  <ctype.h>
# include  <string.h>
# include  "lexor_keyword.h"


# define YY_USER_INIT reset_lexor();
# define yylval VLlval

/*
 * Lexical location information is passed in the yylloc variable to th
 * parser. The file names, strings, are kept in a list so that I can
 * re-use them. The set_file_name function will return a pointer to
 * the name as it exists in the list (and delete the passed string.)
 * If the name is new, it will be added to the list.
 */
extern YYLTYPE yylloc;

struct file_name_cell {
      const char*text;
      struct file_name_cell*next;
};

static struct file_name_cell*file_names = 0;

static const char* set_file_name(char*text)
{
      struct file_name_cell*cur = file_names;
      while (cur) {
	    if (strcmp(cur->text, text) == 0) {
		  delete[]text;
		  return cur->text;
	    }

	    cur = cur->next;
      }

      cur = new struct file_name_cell;
      cur->text = text;
      cur->next = file_names;
      return text;
}


extern void pform_set_timescale(int, int);

void reset_lexor();
static void line_directive();
static void line_directive2();

static verinum*make_sized_binary(const char*txt);
static verinum*make_sized_dec(const char*txt);
static verinum*make_sized_octal(const char*txt);
static verinum*make_sized_hex(const char*txt);
static verinum*make_unsized_binary(const char*txt);
static verinum*make_unsized_dec(const char*txt);
static verinum*make_unsized_octal(const char*txt);
static verinum*make_unsized_hex(const char*txt);

static void process_timescale(const char*txt);

static int comment_enter;
%}

%x CCOMMENT
%x PCOMMENT
%x LCOMMENT
%x CSTRING
%s UDPTABLE
%x PPTIMESCALE

W [ \t\b\f\r]+

%%

^"#line"[ ]+\"[^\"]*\"[ ]+[0-9]+.* { line_directive(); }
^"`line"[ ]+[0-9]+[ ]+\"[^\"]*\".* { line_directive2(); }

[ \t\b\f\r] { ; }
\n { yylloc.first_line += 1; }

  /* C++ style comments start with / / and run to the ene of the
     current line. These are very easy to handle. */

"//".* { comment_enter = YY_START; BEGIN(LCOMMENT); }
<LCOMMENT>.    { yymore(); }
<LCOMMENT>\n   { yylloc.first_line += 1; BEGIN(comment_enter); }


  /* The contents of C-style comments are ignored, like white space. */

"/*" { comment_enter = YY_START; BEGIN(CCOMMENT); }
<CCOMMENT>.    { yymore(); }
<CCOMMENT>\n   { yylloc.first_line += 1; yymore(); }
<CCOMMENT>"*/" { BEGIN(comment_enter); }

  /* Pragma comments are very similar to C-style comments, except that
     they are allowed to carry tool-specific pragma strings. */

"(*" { comment_enter = YY_START; BEGIN(PCOMMENT); }
<PCOMMENT>.    { yymore(); }
<PCOMMENT>\n   { yylloc.first_line += 1; yymore(); }
<PCOMMENT>"*)" { BEGIN(comment_enter); }

"<<" { return K_LS; }
">>" { return K_RS; }
"<=" { return K_LE; }
">=" { return K_GE; }
"=>" { return K_EG; }
"*>" { return K_SG; }
"==" { return K_EQ; }
"!=" { return K_NE; }
"===" { return K_CEQ; }
"!==" { return K_CNE; }
"||" { return K_LOR; }
"&&" { return K_LAND; }
"&&&" { return K_TAND; }
"~|" { return K_NOR; }
"~^" { return K_NXOR; }
"^~" { return K_NXOR; }
"~&" { return K_NAND; }
"->" { return K_TRIGGER; }
"+:" { return K_PO_POS; }
"-:" { return K_PO_NEG; }


[}{;:\[\],()#=.@&!?<>%|^~+*/-] { return yytext[0]; }

\"            { BEGIN(CSTRING); }
<CSTRING>\\\" { yymore(); }
<CSTRING>\n   { BEGIN(0);
                yylval.text = strdup(yytext);
		VLerror(yylloc, "Missing close quote of string.");
		yylloc.first_line += 1;
		return STRING; }
<CSTRING>\"   { BEGIN(0);
                yylval.text = strdup(yytext);
		yylval.text[strlen(yytext)-1] = 0;
		return STRING; }
<CSTRING>.    { yymore(); }

<UDPTABLE>\(\?0\)    { return '_'; }
<UDPTABLE>\(\?1\)    { return '+'; }
<UDPTABLE>\(\?[xX]\) { return '%'; }
<UDPTABLE>\(\?\?\)  { return '*'; }
<UDPTABLE>\(01\)    { return 'r'; }
<UDPTABLE>\(0[xX]\) { return 'Q'; }
<UDPTABLE>\(0\?\)   { return 'P'; }
<UDPTABLE>\(10\)    { return 'f'; }
<UDPTABLE>\(1[xX]\) { return 'M'; }
<UDPTABLE>\(1\?\)   { return 'N'; }
<UDPTABLE>\([xX]0\) { return 'F'; }
<UDPTABLE>\([xX]1\) { return 'R'; }
<UDPTABLE>\([xX]\?\) { return 'B'; }
<UDPTABLE>[bB]     { return 'b'; }
<UDPTABLE>[lL]     { return 'l'; /* IVL extension */ }
<UDPTABLE>[hH]     { return 'h'; /* IVL extension */ }
<UDPTABLE>[fF]     { return 'f'; }
<UDPTABLE>[rR]     { return 'r'; }
<UDPTABLE>[xX]     { return 'x'; }
<UDPTABLE>[nN]     { return 'n'; }
<UDPTABLE>[pP]     { return 'p'; }
<UDPTABLE>[01\?\*\-] { return yytext[0]; }

[a-zA-Z_][a-zA-Z0-9$_]* {
      int rc = lexor_keyword_code(yytext, yyleng);
      if (rc == IDENTIFIER) {
	    yylval.text = strdup(yytext);
	    if (strncmp(yylval.text,"PATHPULSE$", 10) == 0)
		  rc = PATHPULSE_IDENTIFIER;
      } else {
	    yylval.text = 0;
      }

      return rc;
 }


\\[^ \t\b\f\r\n]+         {
      yylval.text = strdup(yytext+1);
      return IDENTIFIER; }

\$([a-zA-Z0-9$_]+)        {
      if (strcmp(yytext,"$setuphold") == 0)
	    return K_Ssetuphold;
      if (strcmp(yytext,"$attribute") == 0)
	    return KK_attribute;
      if (strcmp(yytext,"$hold") == 0)
	    return K_Shold;
      if (strcmp(yytext,"$period") == 0)
	    return K_Speriod;
      if (strcmp(yytext,"$recovery") == 0)
	    return K_Srecovery;
      if (strcmp(yytext,"$setup") == 0)
	    return K_Ssetup;
      if (strcmp(yytext,"$width") == 0)
	    return K_Swidth;
      yylval.text = strdup(yytext);
      return SYSTEM_IDENTIFIER; }

[0-9][0-9_]*[ \t]*\'[sS]?[dD][ \t]*[0-9][0-9_]* {
      yylval.number = make_sized_dec(yytext);
      return NUMBER; }
[0-9][0-9_]*[ \t]*\'[sS]?[bB][ \t]*[0-1xzXZ_\?]+ {
      yylval.number = make_sized_binary(yytext);
      return NUMBER; }
[0-9][0-9_]*[ \t]*\'[sS]?[oO][ \t]*[0-7xzXZ_\?]+ {
      yylval.number = make_sized_octal(yytext);
      return NUMBER; }
[0-9][0-9_]*[ \t]*\'[sS]?[hH][ \t]*[0-9a-fA-FxzXZ_\?]+ {
      yylval.number = make_sized_hex(yytext);
      return NUMBER; }

\'[sS]?[dD][ \t]*[0-9][0-9_]*  { yylval.number = make_unsized_dec(yytext);
                            return NUMBER; }
\'[sS]?[bB][ \t]*[0-1xzXZ_\?]+ { yylval.number = make_unsized_binary(yytext);
                        return NUMBER; }
\'[sS]?[oO][ \t]*[0-7xzXZ_\?]+ { yylval.number = make_unsized_octal(yytext);
                        return NUMBER; }
\'[sS]?[hH][ \t]*[0-9a-fA-FxzXZ_\?]+ { yylval.number = make_unsized_hex(yytext);
                              return NUMBER; }

[0-9][0-9_]*		 {
	/* Handle the special case of the unsized decimal number. */
      unsigned long value = 0;
      for (const char*cp = yytext ;  *cp ;  cp += 1) {
	    if (*cp != '_')
		  value = 10 * value + (*cp - '0');
      }

      assert(INTEGER_WIDTH <= (8*sizeof(value)));
      unsigned nbits = INTEGER_WIDTH;
      verinum::V*bits = new verinum::V[8 * sizeof value];

      for (unsigned idx = 0 ;  idx < nbits ;  idx += 1, value >>= 1) {
	    bits[idx] = (value&1) ? verinum::V1 : verinum::V0;
      }

      yylval.number = new verinum(bits, nbits, false);
      yylval.number->has_sign(true);
      delete[]bits;
      return NUMBER; }

[0-9][0-9_]*\.[0-9][0-9_]*([Ee][+-]?[0-9][0-9_]*)? {
      yylval.realtime = new verireal(yytext);
      return REALTIME; }

[0-9][0-9_]*[Ee][+-]?[0-9][0-9_]* {
      yylval.realtime = new verireal(yytext);
      return REALTIME; }


  /* Notice and handle the timescale directive. */

^{W}?`timescale { BEGIN(PPTIMESCALE); }
<PPTIMESCALE>.* { process_timescale(yytext); }
<PPTIMESCALE>\n {
      yylloc.first_line += 1;
      BEGIN(0); }


  /* These are directives that I do not yet support. I think that IVL
     should handle these, not an external preprocessor. */

^{W}?`celldefine{W}?.*           {  }
^{W}?`default_nettype{W}?.*      {  }
^{W}?`delay_mode_distributed{W}?.*  {  }
^{W}?`delay_mode_unit{W}?.*      {  }
^{W}?`delay_mode_path{W}?.*      {  }
^{W}?`disable_portfaults{W}?.*   {  }
^{W}?`enable_portfaults{W}?.*    {  }
^{W}?`endcelldefine{W}?.*        {  }
^{W}?`endprotect{W}?.*           {  }
^{W}?`nosuppress_faults{W}?.*    {  }
^{W}?`nounconnected_drive{W}?.*  {  }
^{W}?`protect{W}?.*              {  }
^{W}?`resetall{W}?.*             {  }
^{W}?`suppress_faults{W}?.*      {  }
^{W}?`unconnected_drive{W}?.*    {  }
^{W}?`uselib{W}?.*               {  }


  /* These are directives that are not supported by me and should have
     been handled by an external preprocessor such as ivlpp. */

^{W}?`define{W}?.* {
      cerr << yylloc.text << ":" << yylloc.first_line <<
	    ": `define not supported. Use an external preprocessor."
	   << endl;
  }

^{W}?`else{W}?.* {
      cerr << yylloc.text << ":" << yylloc.first_line <<
	    ": `else not supported. Use an external preprocessor."
	   << endl;
  }

^{W}?`endif{W}?.* {
      cerr << yylloc.text << ":" << yylloc.first_line <<
	    ": `endif not supported. Use an external preprocessor."
	   << endl;
  }

^{W}?`ifdef{W}?.* {
      cerr << yylloc.text << ":" << yylloc.first_line <<
	    ": `ifdef not supported. Use an external preprocessor."
	   << endl;
  }

^`include{W}?.* {
      cerr << yylloc.text << ":" << yylloc.first_line <<
	    ": `include not supported. Use an external preprocessor."
	   << endl;
  }

^`undef{W}?.* {
      cerr << yylloc.text << ":" << yylloc.first_line <<
	    ": `undef not supported. Use an external preprocessor."
	   << endl;
  }



  /* Final catchall. something got lost or mishandled. */

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

static verinum*make_binary_with_size(unsigned size, bool fixed, const char*ptr)
{
      bool sign_flag = false;
      verinum::V*bits = new verinum::V[size];

      if (tolower(ptr[0]) == 's') {
	    ptr += 1;
	    sign_flag = true;
      }
      assert(tolower(*ptr) == 'b');
      ptr += 1;

      while (*ptr && ((*ptr == ' ') || (*ptr == '\t')))
	    ptr += 1;

      unsigned idx = 0;
      const char*eptr = ptr + strlen(ptr) - 1;

      while ((eptr >= ptr) && (idx < size)) {

	    switch (*eptr) {
		case '_':
		  break;
		case '0':
		  bits[idx++] = verinum::V0;
		  break;
		case '1':
		  bits[idx++] = verinum::V1;
		  break;
		case 'z': case 'Z': case '?':
		  bits[idx++] = verinum::Vz;
		  break;
		case 'x': case 'X':
		  bits[idx++] = verinum::Vx;
		  break;
		default:
		  assert(0);
	    }

	    eptr -= 1;
      }


	/* If we filled up the expected number of bits, but there are
	   still characters of the number part left, then report a
	   warning that we are truncating. */

      if ((idx >= size) && (eptr >= ptr))
        cerr << yylloc.text << ":" << yylloc.first_line <<
          ": warning: Numeric binary constant ``" << ptr <<
           "'' truncated to " << size << " bits." << endl;


	// Zero-extend binary number, except that z or x is extended
	// if it is the highest supplied digit.
      while (idx < size) {
	    switch (ptr[0]) {
		case '0':
		case '1':
		  bits[idx++] = verinum::V0;
		  break;
		case 'z': case 'Z': case '?':
		  bits[idx++] = verinum::Vz;
		  break;
		case 'x': case 'X':
		  bits[idx++] = verinum::Vx;
		  break;
		default:
		  assert(0);
	    }
      }

      verinum*out = new verinum(bits, size, fixed);
      delete[]bits;
      out->has_sign(sign_flag);
      return out;
}

static verinum*make_sized_binary(const char*txt)
{
      char*ptr;
      unsigned size = strtoul(txt,&ptr,10);
      while (*ptr && ((*ptr == ' ') || (*ptr == '\t')))
	    ptr += 1;
      assert(*ptr == '\'');
      ptr += 1;

      return make_binary_with_size(size, true, ptr);
}

static verinum*make_unsized_binary(const char*txt)
{
      bool sign_flag = false;
      const char*ptr = txt;
      assert(*ptr == '\'');
      ptr += 1;

      if (tolower(*ptr) == 's') {
	    sign_flag = true;
	    ptr += 1;
      }
      assert(tolower(*ptr) == 'b');
      while (*ptr && ((*ptr == 'b') || (*ptr == ' ') || (*ptr == '\t')))
	    ptr += 1;

      unsigned size = 0;
      for (const char*idx = ptr ;  *idx ;  idx += 1)
	    if (*idx != '_') size += 1;

      verinum::V*bits = new verinum::V[size];

      unsigned idx = size;
      while (*ptr) {
	    switch (ptr[0]) {
		case '0':
		  bits[--idx] = verinum::V0;
		  break;
		case '1':
		  bits[--idx] = verinum::V1;
		  break;
		case 'z': case 'Z': case '?':
		  bits[--idx] = verinum::Vz;
		  break;
		case 'x': case 'X':
		  bits[--idx] = verinum::Vx;
		  break;
		  case '_':
		  break;
		default:
		  fprintf(stderr, "%c\n", ptr[0]);
		  assert(0);
	    }
	    ptr += 1;
      }

      verinum*out = new verinum(bits, size);
      out->has_sign(sign_flag);
      delete[]bits;
      return out;
}

static verinum*make_sized_octal(const char*txt)
{
      bool sign_flag = false;
      char*ptr;

      unsigned size = strtoul(txt,&ptr,10);
      while (*ptr && ((*ptr == ' ') || (*ptr == '\t')))
	    ptr += 1;
      assert(*ptr == '\'');
      ptr += 1;

      if (tolower(*ptr) == 's') {
	    sign_flag = true;
	    ptr += 1;
      }
      assert(tolower(*ptr) == 'o');
      ptr += 1;

	/* We know from the size number how bit to make the verinum
	   array, so make it now. */
      verinum::V*bits = new verinum::V[(size+2)/3 * 3];

	/* skip white space between size and the base token. */
      while (*ptr && ((*ptr == ' ') || (*ptr == '\t')))
	    ptr += 1;

	/* Get a pointer to the last character of the number string,
	   then work back from there (the least significant digit)
	   forward until I run out of digits or space to put them. */
      char*eptr = ptr + strlen(ptr) - 1;
      unsigned idx = 0;

      while ((eptr >= ptr) && (idx < size)) {
	    switch (*eptr) {
		case 'x': case 'X':
		  bits[idx++] = verinum::Vx;
		  bits[idx++] = verinum::Vx;
		  bits[idx++] = verinum::Vx;
		  break;
		case 'z': case 'Z': case '?':
		  bits[idx++] = verinum::Vz;
		  bits[idx++] = verinum::Vz;
		  bits[idx++] = verinum::Vz;
		  break;
		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
		    {
			  unsigned val = *eptr - '0';
			  bits[idx++] = (val&1)? verinum::V1 : verinum::V0;
			  bits[idx++] = (val&2)? verinum::V1 : verinum::V0;
			  bits[idx++] = (val&4)? verinum::V1 : verinum::V0;
			  break;
		    }
		case '_':
		  break;
		default:
		  assert(0);
	    }

	    eptr -= 1;
      }

	/* If we filled up all the bits and there are still characters
	   in the number string, then we overflowed. Report a warning
	   that we are truncating. */
      if ((idx >= size) && (eptr >= ptr))
	    cerr << yylloc.text << ":" << yylloc.first_line <<
		  ": warning: Numeric octal constant ``" << ptr <<
		  "'' truncated to " << size << " bits." << endl;


	/* If we did not fill up all the bits from the string, then
	   zero-extend the number. */
      while (idx < size) switch (ptr[1]) {
	  case 'x': case 'X':
	    bits[idx++] = verinum::Vx;
	    break;
	  case 'z': case 'Z': case '?':
	    bits[idx++] = verinum::Vz;
	    break;
	  default:
	    bits[idx++] = verinum::V0;
      }

      verinum*out = new verinum(bits, size, true);
      delete[]bits;
      out->has_sign(sign_flag);
      return out;
}

static verinum*make_unsized_octal(const char*txt)
{
      bool sign_flag = false;
      const char*ptr = txt;
      assert(*ptr == '\'');
      ptr += 1;

      if (tolower(*ptr) == 's') {
	    sign_flag = true;
	    ptr += 1;
      }

      assert(tolower(*ptr) == 'o');
      ptr += 1;

      while (*ptr && ((*ptr == ' ') || (*ptr == '\t')))
	    ptr += 1;

      unsigned size = 0;
      for (const char*idx = ptr ;  *idx ;  idx += 1)
	    if (*idx != '_') size += 3;

      verinum::V*bits = new verinum::V[size];

      unsigned idx = size;
      while (*ptr) {
	    unsigned val;
	    switch (ptr[0]) {
		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
		  val = *ptr - '0';
		  bits[--idx] = (val&4) ? verinum::V1 : verinum::V0;
		  bits[--idx] = (val&2) ? verinum::V1 : verinum::V0;
		  bits[--idx] = (val&1) ? verinum::V1 : verinum::V0;
		  break;
		case 'x': case 'X':
		  bits[--idx] = verinum::Vx;
		  bits[--idx] = verinum::Vx;
		  bits[--idx] = verinum::Vx;
		  break;
		case 'z': case 'Z': case '?':
		  bits[--idx] = verinum::Vz;
		  bits[--idx] = verinum::Vz;
		  bits[--idx] = verinum::Vz;
		  break;
		case '_':
		  break;
		default:
		  assert(0);
	    }
	    ptr += 1;
      }

      verinum*out = new verinum(bits, size);
      out->has_sign(sign_flag);
      delete[]bits;
      return out;
}

static verinum*make_sized_hex(const char*txt)
{
      bool sign_flag = false;
      char*ptr;
      unsigned size = strtoul(txt,&ptr,10);

      while (*ptr && ((*ptr == ' ') || (*ptr == '\t')))
	    ptr += 1;

      assert(*ptr == '\'');
      ptr += 1;

      if (tolower(*ptr) == 's') {
	    sign_flag = true;
	    ptr += 1;
      }

      assert(tolower(*ptr) == 'h');

      ptr += 1;
      while (*ptr && ((*ptr == ' ') || (*ptr == '\t')))
	    ptr += 1;

      verinum::V*bits = new verinum::V[(size+3)&~3];

      unsigned idx = 0;
      char*eptr = ptr + strlen(ptr) - 1;

      while ((eptr >= ptr) && (idx < size)) {
	    switch (*eptr) {
		case 'x': case 'X':
		  bits[idx++] = verinum::Vx;
		  bits[idx++] = verinum::Vx;
		  bits[idx++] = verinum::Vx;
		  bits[idx++] = verinum::Vx;
		  break;
		case 'z': case 'Z': case '?':
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
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		    {
			  unsigned val = *eptr - '0';
			  bits[idx++] = (val&1)? verinum::V1 : verinum::V0;
			  bits[idx++] = (val&2)? verinum::V1 : verinum::V0;
			  bits[idx++] = (val&4)? verinum::V1 : verinum::V0;
			  bits[idx++] = (val&8)? verinum::V1 : verinum::V0;
			  break;
		    }
		case '_':
		  break;
		default:
		  assert(0);
	    }

	    eptr -= 1;
      }

	/* If we filled up the expected number of bits, but there are
	   still characters of the number part left, then report a
	   warning that we are truncating. */
      if ((idx >= size) && (eptr >= ptr))
        cerr << yylloc.text << ":" << yylloc.first_line <<
          ": warning: Numeric hex constant ``" << ptr <<
           "'' truncated to " << size << " bits." << endl;

	// zero extend hex numbers
      while (idx < size) switch (ptr[0]) {
	  case 'x': case 'X':
	    bits[idx++] = verinum::Vx;
	    break;
	  case 'z': case 'Z': case '?':
	    bits[idx++] = verinum::Vz;
	    break;
	  default:
	    bits[idx++] = verinum::V0;
      }

      verinum*out = new verinum(bits, size, true);
      out->has_sign(sign_flag);
      delete[]bits;
      return out;
}

static verinum*make_unsized_hex(const char*txt)
{
      bool sign_flag = false;
      const char*ptr = txt;
      assert(*ptr == '\'');
      ptr += 1;

      if (tolower(*ptr) == 's') {
	    sign_flag = true;
	    ptr += 1;
      }
      assert(tolower(*ptr) == 'h');

      ptr += 1;
      while (*ptr && ((*ptr == ' ') || (*ptr == '\t')))
	    ptr += 1;

      unsigned size = 0;
      for (const char*idx = ptr ;  *idx ;  idx += 1)
	    if (*idx != '_') size += 4;

      verinum::V*bits = new verinum::V[size];

      unsigned idx = size;
      while (*ptr) {
	    unsigned val;
	    switch (ptr[0]) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		  val = *ptr - '0';
		  bits[--idx] = (val&8) ? verinum::V1 : verinum::V0;
		  bits[--idx] = (val&4) ? verinum::V1 : verinum::V0;
		  bits[--idx] = (val&2) ? verinum::V1 : verinum::V0;
		  bits[--idx] = (val&1) ? verinum::V1 : verinum::V0;
		  break;
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		  val = tolower(*ptr) - 'a' + 10;
		  bits[--idx] = (val&8) ? verinum::V1 : verinum::V0;
		  bits[--idx] = (val&4) ? verinum::V1 : verinum::V0;
		  bits[--idx] = (val&2) ? verinum::V1 : verinum::V0;
		  bits[--idx] = (val&1) ? verinum::V1 : verinum::V0;
		  break;
		case 'x': case 'X':
		  bits[--idx] = verinum::Vx;
		  bits[--idx] = verinum::Vx;
		  bits[--idx] = verinum::Vx;
		  bits[--idx] = verinum::Vx;
		  break;
		case 'z': case 'Z': case '?':
		  bits[--idx] = verinum::Vz;
		  bits[--idx] = verinum::Vz;
		  bits[--idx] = verinum::Vz;
		  bits[--idx] = verinum::Vz;
		  break;
		case '_':
		  break;
		default:
		  assert(0);
	    }
	    ptr += 1;
      }

      verinum*out = new verinum(bits, size);
      out->has_sign(sign_flag);
      delete[]bits;
      return out;
}

/*
 * Making a decimal number is much easier then the other base numbers
 * because there are no z or x values to worry about.
 */
static verinum*make_dec_with_size(unsigned size, bool fixed, const char*ptr)
{
      bool signed_flag = false;
      if (tolower(*ptr) == 's') {
	    signed_flag = true;
	    ptr += 1;
      }
      assert(tolower(*ptr) == 'd');

      ptr += 1;
      while (*ptr && ((*ptr == ' ') || (*ptr == '\t')))
	    ptr += 1;

      const char*digits = ptr;

	/* Convert the decimal number to a binary value, one digit at
	   a time. Watch out for overflow. */

      unsigned long value = 0;
      for ( ; *ptr ; ptr += 1)
	    if (isdigit(*ptr)) {
		  unsigned long tmp = value * 10 + (*ptr - '0');
		  if (tmp < value)
			cerr << yylloc.text << ":" << yylloc.first_line <<
			      ": warning: Numeric decimal constant ``"
			     << digits << "'' is too large." << endl;
		  value *= 10;
		  value += *ptr - '0';
	    } else  {
		  assert(*ptr == '_');
	    }


      verinum::V*bits = new verinum::V[size];

      for (unsigned idx = 0 ;  idx < size ;  idx += 1) {
	    bits[idx] = (value&1)? verinum::V1 : verinum::V0;
	    value /= 2;
      }

	/* If we run out of bits to hold the value, but there are
	   still valueable bits in the number, print a warning. */

      if (value != 0)
        cerr << yylloc.text << ":" << yylloc.first_line <<
          ": warning: Numeric decimal constant ``" << digits <<
           "'' truncated to " << size << " bits." << endl;

      verinum*out = new verinum(bits, size, fixed);
      out->has_sign(signed_flag);
      delete[]bits;
      return out;
}

static verinum*make_sized_dec(const char*txt)
{
      char*ptr;
      unsigned size = strtoul(txt,&ptr,10);

      while (*ptr && ((*ptr == ' ') || (*ptr == '\t')))
	    ptr += 1;

      assert(*ptr == '\'');
      ptr += 1;

      return make_dec_with_size(size, true, ptr);
}

static verinum*make_unsized_dec(const char*txt)
{
      return make_dec_with_size(INTEGER_WIDTH, false, txt+1);
}

/*
 * The timescale parameter has the form:
 *      " <num> xs / <num> xs"
 */
static void process_timescale(const char*txt)
{
      unsigned num;
      const char*cp = txt + strspn(txt, " \t");
      char*tmp;
      const char*ctmp;

      int unit = 0;
      int prec = 0;

      num = strtoul(cp, &tmp, 10);
      if (num == 0) {
	    VLerror(yylloc, "Invalid timescale string.");
	    return;
      }

      while (num >= 10) {
	    unit += 1;
	    num  /= 10;
      }
      if (num != 1) {
	    VLerror(yylloc, "Invalid timescale unit number.");
	    return;
      }

      cp = tmp;
      cp += strspn(cp, " \t");
      ctmp = cp + strcspn(cp, " \t/");

      if (strncmp("s", cp, ctmp-cp) == 0) {
	    unit -= 0;

      } else if (strncmp("ms", cp, ctmp-cp) == 0) {
	    unit -= 3;

      } else if (strncmp("us", cp, ctmp-cp) == 0) {
	    unit -= 6;

      } else if (strncmp("ns", cp, ctmp-cp) == 0) {
	    unit -= 9;

      } else if (strncmp("ps", cp, ctmp-cp) == 0) {
	    unit -= 12;

      } else if (strncmp("fs", cp, ctmp-cp) == 0) {
	    unit -= 15;

      } else {
	    VLerror(yylloc, "Invalid timescale unit of measurement");
	    return;
      }

      cp = ctmp;
      cp += strspn(cp, " \t/");

      num = strtoul(cp, &tmp, 10);
      if (num == 0) {
	    VLerror(yylloc, "Invalid timescale string.");
	    return;
      }
      assert(num);
      while (num >= 10) {
	    prec += 1;
	    num  /= 10;
      }
      if (num != 1) {
	    VLerror(yylloc, "Invalid timescale precision number.");
	    return;
      }

      cp = tmp;
      cp += strspn(cp, " \t");
      ctmp = cp + strcspn(cp, " \t\r");

      if (strncmp("s", cp, ctmp-cp) == 0) {
	    prec -= 0;

      } else if (strncmp("ms", cp, ctmp-cp) == 0) {
	    prec -= 3;

      } else if (strncmp("us", cp, ctmp-cp) == 0) {
	    prec -= 6;

      } else if (strncmp("ns", cp, ctmp-cp) == 0) {
	    prec -= 9;

      } else if (strncmp("ps", cp, ctmp-cp) == 0) {
	    prec -= 12;

      } else if (strncmp("fs", cp, ctmp-cp) == 0) {
	    prec -= 15;

      } else {
	    VLerror(yylloc, "Invalid timescale precision units of measurement");
	    return;
      }

      pform_set_timescale(unit, prec);
}

int yywrap()
{
      return 1;
}

/*
 * The line directive matches lines of the form #line "foo" N and
 * calls this function. Here I parse out the file name and line
 * number, and change the yylloc to suite.
 */
static void line_directive()
{
      char*qt1 = strchr(yytext, '"');
      assert(qt1);
      qt1 += 1;

      char*qt2 = strchr(qt1, '"');
      assert(qt2);

      char*buf = new char[qt2-qt1+1];
      strncpy(buf, qt1, qt2-qt1);
      buf[qt2-qt1] = 0;

      yylloc.text = set_file_name(buf);

      qt2 += 1;
      yylloc.first_line = strtoul(qt2,0,0);
}

static void line_directive2()
{
      assert(strncmp(yytext,"`line",5) == 0);
      char*cp = yytext + strlen("`line");
      cp += strspn(cp, " ");
      yylloc.first_line = strtoul(cp,&cp,10);

      yylloc.first_line -= 1;

      cp += strspn(cp, " ");
      if (*cp == 0) return;

      char*qt1 = strchr(yytext, '"');
      assert(qt1);
      qt1 += 1;

      char*qt2 = strchr(qt1, '"');
      assert(qt2);

      char*buf = new char[qt2-qt1+1];
      strncpy(buf, qt1, qt2-qt1);
      buf[qt2-qt1] = 0;

      yylloc.text = set_file_name(buf);
}

extern FILE*vl_input;
void reset_lexor()
{
      yyrestart(vl_input);
      yylloc.first_line = 1;

	/* Start the file_names list. From here on, as I get a file
	   name, I will add it to this list. Only add the name if it
	   is not already in the list. */
      file_names = new struct file_name_cell;
      file_names->text = strdup(vl_file.c_str());
      file_names->next = 0;
      yylloc.text = file_names->text;
}
