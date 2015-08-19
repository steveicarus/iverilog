
%option never-interactive
%option nounput

%{
/*
 * Copyright (c) 1998-2010 Stephen Williams (steve@icarus.com)
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

# include "config.h"

      //# define YYSTYPE lexval

# include  <iostream>
# include  "compiler.h"
# include  "parse_misc.h"
# include  "parse_api.h"
# include  "parse.h"
# include  <ctype.h>
# include  <string.h>
# include  "lexor_keyword.h"


# define YY_USER_INIT reset_lexor();
# define yylval VLlval

#define YY_NO_INPUT

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


extern void pform_set_timescale(int, int, const char*file, unsigned line);

void reset_lexor();
static void line_directive();
static void line_directive2();

static verinum*make_unsized_binary(const char*txt);
static verinum*make_unsized_dec(const char*txt);
static verinum*make_unsized_octal(const char*txt);
static verinum*make_unsized_hex(const char*txt);

static int dec_buf_div2(char *buf);

static void process_timescale(const char*txt);

static int comment_enter;
static bool in_module = false;
%}

%x CCOMMENT
%x PCOMMENT
%x LCOMMENT
%x CSTRING
%s UDPTABLE
%x PPTIMESCALE
%x PPDEFAULT_NETTYPE
%s EDGES

W [ \t\b\f\r]+

%%

^"#line"[ ]+\"[^\"]*\"[ ]+[0-9]+.* { line_directive(); }
^"`line"[ ]+[0-9]+[ ]+\"[^\"]*\".* { line_directive2(); }

[ \t\b\f\r] { ; }
\n { yylloc.first_line += 1; }

  /* C++ style comments start with / / and run to the end of the
     current line. These are very easy to handle. */

"//".* { comment_enter = YY_START; BEGIN(LCOMMENT); }
<LCOMMENT>.    { yymore(); }
<LCOMMENT>\n   { yylloc.first_line += 1; BEGIN(comment_enter); }


  /* The contents of C-style comments are ignored, like white space. */

"/*" { comment_enter = YY_START; BEGIN(CCOMMENT); }
<CCOMMENT>.    { yymore(); }
<CCOMMENT>\n   { yylloc.first_line += 1; yymore(); }
<CCOMMENT>"*/" { BEGIN(comment_enter); }


"(*" { return K_PSTAR; }
"*)" { return K_STARP; }
"<<" { return K_LS; }
"<<<" { return K_LS; /* Note: Functionally, <<< is the same as <<. */}
">>"  { return K_RS; }
">>>" { return K_RSS; }
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

  /* Watch out for the tricky case of (*). Cannot parse this as "(*"
     and ")", but since I know that this is really ( * ), replace it
     with "*" and return that. */
"(*"{W}*")" { return '*'; }

<EDGES>"]" { BEGIN(0); return yytext[0]; }
[}{;:\[\],()#=.@&!?<>%|^~+*/-] { return yytext[0]; }

\"            { BEGIN(CSTRING); }
<CSTRING>\\\\ { yymore(); /* Catch \\, which is a \ escaping itself */ }
<CSTRING>\\\" { yymore(); /* Catch \", which is an escaped quote */ }
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
<UDPTABLE>\(b[xX]\) { return 'q'; }
<UDPTABLE>\(b0\)    { return 'f'; /* b0 is 10|00, but only 10 is meaningful */}
<UDPTABLE>\(b1\)    { return 'r'; /* b1 is 11|01, but only 01 is meaningful */}
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

<EDGES>"01" { return K_edge_descriptor; }
<EDGES>"0x" { return K_edge_descriptor; }
<EDGES>"0z" { return K_edge_descriptor; }
<EDGES>"10" { return K_edge_descriptor; }
<EDGES>"1x" { return K_edge_descriptor; }
<EDGES>"1z" { return K_edge_descriptor; }
<EDGES>"x0" { return K_edge_descriptor; }
<EDGES>"x1" { return K_edge_descriptor; }
<EDGES>"z0" { return K_edge_descriptor; }
<EDGES>"z1" { return K_edge_descriptor; }

[a-zA-Z_][a-zA-Z0-9$_]* {
      int rc = lexor_keyword_code(yytext, yyleng);
      switch (rc) {
	  case IDENTIFIER:
	    yylval.text = strdup(yytext);
	    if (strncmp(yylval.text,"PATHPULSE$", 10) == 0)
		  rc = PATHPULSE_IDENTIFIER;
	    break;

	  case K_edge:
	    BEGIN(EDGES);
	    break;

	  case K_module:
	  case K_macromodule:
	    in_module = true;
	    break;

	  case K_endmodule:
	    in_module = false;
	    break;

	  default:
	    yylval.text = 0;
	    break;
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
      if (strcmp(yytext,"$recrem") == 0)
	    return K_Srecrem;
      if (strcmp(yytext,"$setup") == 0)
	    return K_Ssetup;
      if (strcmp(yytext,"$width") == 0)
	    return K_Swidth;
      yylval.text = strdup(yytext);
      return SYSTEM_IDENTIFIER; }


\'[sS]?[dD][ \t]*[0-9][0-9_]*  { yylval.number = make_unsized_dec(yytext);
                            return BASED_NUMBER; }
\'[sS]?[bB][ \t]*[0-1xzXZ_\?]+ { yylval.number = make_unsized_binary(yytext);
                        return BASED_NUMBER; }
\'[sS]?[oO][ \t]*[0-7xzXZ_\?]+ { yylval.number = make_unsized_octal(yytext);
                        return BASED_NUMBER; }
\'[sS]?[hH][ \t]*[0-9a-fA-FxzXZ_\?]+ { yylval.number = make_unsized_hex(yytext);
                              return BASED_NUMBER; }

[0-9][0-9_]* {
      yylval.number = make_unsized_dec(yytext);
      return DEC_NUMBER; }

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
      if (in_module) {
	    cerr << yylloc.text << ":" << yylloc.first_line << ": error: "
	            "`timescale directive can not be inside a module "
	            "definition." << endl;
	    error_count += 1;
      }
      yylloc.first_line += 1;
      BEGIN(0); }


  /* These are directives that I do not yet support. I think that IVL
     should handle these, not an external preprocessor. */

^{W}?`celldefine{W}?.*           {  }
^{W}?`delay_mode_distributed{W}?.*  {  }
^{W}?`delay_mode_unit{W}?.*      {  }
^{W}?`delay_mode_path{W}?.*      {  }
^{W}?`disable_portfaults{W}?.*   {  }
^{W}?`enable_portfaults{W}?.*    {  }
^{W}?`endcelldefine{W}?.*        {  }
`endprotect                      {  }
^{W}?`nosuppress_faults{W}?.*    {  }
^{W}?`nounconnected_drive{W}?.*  {  }
`protect                         {  }
^{W}?`resetall{W}?.*             {  }
^{W}?`suppress_faults{W}?.*      {  }
^{W}?`unconnected_drive{W}?.*    {  }
^{W}?`uselib{W}?.*               {  }

  /* Notice and handle the default_nettype directive. The lexor
     detects the default_nettype keyword, and the second part of the
     rule collects the rest of the line and processes it. We only need
     to look for the first work, and interpret it. */

`default_nettype{W}? { BEGIN(PPDEFAULT_NETTYPE); }
<PPDEFAULT_NETTYPE>.* {
      NetNet::Type net_type;
      size_t wordlen = strcspn(yytext, " \t\f\r\n");
      yytext[wordlen] = 0;
      if (strcmp(yytext,"wire") == 0) {
	    net_type = NetNet::WIRE;

      } else if (strcmp(yytext,"none") == 0) {
	    net_type = NetNet::NONE;

      } else {
	    cerr << yylloc.text << ":" << yylloc.first_line
		 << " error: Net type " << yytext
		 << " is not a valid (and supported)"
		 << " default net type." << endl;
	    net_type = NetNet::WIRE;
	    error_count += 1;
      }
      pform_set_default_nettype(net_type, yylloc.text, yylloc.first_line);
 }
<PPDEFAULT_NETTYPE>\n {
      yylloc.first_line += 1;
      BEGIN(0); }


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


`{W} { cerr << yylloc.text << ":" << yylloc.first_line << ": error: "
	    << "Stray tic (`) here. Perhaps you put white space" << endl;
       cerr << yylloc.text << ":" << yylloc.first_line << ":      : "
	    << "between the tic and preprocessor directive?"
	    << endl;
       error_count += 1; }

  /* Final catchall. something got lost or mishandled. */

<*>.|\n {   cerr << yylloc.text << ":" << yylloc.first_line
	   << ": error: unmatched character (";
      if (isgraph(yytext[0]))
	    cerr << yytext[0];
      else
	    cerr << "hex " << hex << (0xffU & ((unsigned) (yytext[0])));

      cerr << ")" << endl;
      error_count += 1; }

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
      ptr += 1;

      while (*ptr && ((*ptr == ' ') || (*ptr == '\t')))
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


/* Divide the integer given by the string by 2. Return the remainder bit. */
static int dec_buf_div2(char *buf)
{
    int partial;
    int len = strlen(buf);
    char *dst_ptr;
    int pos;

    partial = 0;
    pos = 0;

    /* dst_ptr overwrites buf, but all characters that are overwritten
       were already used by the reader. */
    dst_ptr = buf;

    while(buf[pos] == '0')
	++pos;

    for(; pos<len; ++pos){
	if (buf[pos]=='_')
	    continue;

	assert(isdigit(buf[pos]));

	partial= partial*10 + (buf[pos]-'0');

	if (partial >= 2){
	    *dst_ptr = partial/2 + '0';
	    partial = partial & 1;

	    ++dst_ptr;
	}
	else{
	    *dst_ptr = '0';
	    ++dst_ptr;
	}
    }

    // If result of division was zero string, it should remain that way.
    // Don't eat the last zero...
    if (dst_ptr == buf){
	*dst_ptr = '0';
	++dst_ptr;
    }
    *dst_ptr = 0;

    return partial;
}

/*
 * Making a decimal number is much easier then the other base numbers
 * because there are no z or x values to worry about. It is much
 * harder then other base numbers because the width needed in bits is
 * hard to calculate.
 */

static verinum*make_unsized_dec(const char*ptr)
{
      char buf[4096];
      bool signed_flag = false;
      unsigned idx;

      if (ptr[0] == '\'') {
	      /* The number has decorations of the form 'sd<digits>,
		 possibly with space between the d and the <digits>.
		 Also, the 's' is optional, and marks the number as
		 signed. */
	    ptr += 1;

	    if (tolower(*ptr) == 's') {
		  signed_flag = true;
		  ptr += 1;
	    }

	    assert(tolower(*ptr) == 'd');
	    ptr += 1;

	    while (*ptr && ((*ptr == ' ') || (*ptr == '\t')))
		  ptr += 1;

      } else {
	      /* ... or an undecorated decimal number is passed
		 it. These numbers are treated as signed decimal. */
	    assert(isdigit(*ptr));
	    signed_flag = true;
      }


	/* Copy the digits into a buffer that I can use to do in-place
	   decimal divides. */
      idx = 0;
      while ((idx < sizeof buf) && (*ptr != 0)) {
	    if (*ptr == '_') {
		  ptr += 1;
		  continue;
	    }

	    buf[idx++] = *ptr++;
      }

      if (idx == sizeof buf) {
	    fprintf(stderr, "Ridiculously long"
		    " decimal constant will be truncated!\n");
	    idx -= 1;
      }

      buf[idx] = 0;
      unsigned tmp_size = idx * 4 + 1;
      verinum::V *bits = new verinum::V[tmp_size];

      idx = 0;
      while (idx < tmp_size) {
	    int rem = dec_buf_div2(buf);
	    bits[idx++] = (rem == 1) ? verinum::V1 : verinum::V0;
      }

      assert(strcmp(buf, "0") == 0);

	/* Now calculate the minimum number of bits needed to
	   represent this unsigned number. */
      unsigned size = tmp_size;
      while ((size > 1) && (bits[size-1] == verinum::V0))
	    size -= 1;

	/* Now account for the signedness. Don't leave a 1 in the high
	   bit if this is a signed number. */
      if (signed_flag && (bits[size-1] == verinum::V1)) {
	    size += 1;
	    assert(size <= tmp_size);
      }

      verinum*res = new verinum(bits, size, false);
      res->has_sign(signed_flag);

      delete[]bits;
      return res;
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

      pform_set_timescale(unit, prec, yylloc.text, yylloc.first_line);
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
