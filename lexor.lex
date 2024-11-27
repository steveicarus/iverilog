%option prefix="VL"
%option never-interactive
%option nounput

%{
/*
 * Copyright (c) 1998-2024 Stephen Williams (steve@icarus.com)
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

# include "config.h"

      //# define YYSTYPE lexval

# include  <climits>
# include  <cstdarg>
# include  <iostream>
# include  "compiler.h"
# include  "parse_misc.h"
# include  "parse_api.h"
# include  "parse.h"
# include  <cctype>
# include  <cstring>
# include  "lexor_keyword.h"
# include  "discipline.h"
# include  <list>

using namespace std;

# define YY_USER_INIT do { reset_lexor(); yylloc.lexical_pos = 0; } while (0);
# define yylval VLlval

# define YY_NO_INPUT

/*
 * Lexical location information is passed in the yylloc variable to th
 * parser. The file names, strings, are kept in a list so that I can
 * re-use them. The set_file_name function will return a pointer to
 * the name as it exists in the list (and delete the passed string.)
 * If the name is new, it will be added to the list.
 */
extern YYLTYPE yylloc;

char* yytext_string_filter(const char*str, size_t str_len)
{
      if (str == 0) return 0;
      char*buf = new char[str_len+1];
      size_t didx = 0;
      for (size_t sidx = 0 ; sidx < str_len ; sidx += 1, didx += 1) {
	    if (str[sidx] == 0) {
		  VLerror(yylloc, "error: Found nil (\\000) in string literal, replacing with space (\\015) character.");
		  buf[didx] = ' ';
	    } else if (str[sidx] == '\\') { /* Skip \\\n */
		  if ((sidx+1 < str_len) && (str[sidx+1] == '\n')) {
			sidx += 1;
			didx -= 1;
		  } else {
			buf[didx] = str[sidx];
		  }
	    } else {
		  buf[didx] = str[sidx];
	    }
      }
      buf[didx] = 0;
      return buf;
}

char* strdupnew(char const *str)
{
       return str ? strcpy(new char [strlen(str)+1], str) : NULL;
}

static const char* set_file_name(char*text)
{
      perm_string path = filename_strings.make(text);
      delete[]text;

	/* Check this file name with the list of library file
	   names. If there is a match, then turn on the
	   pform_library_flag. This is how the parser knows that
	   modules declared in this file are library modules. */
      pform_library_flag = library_file_map[path];
      return path;
}

void reset_lexor();
static void line_directive();
static void line_directive2();
static void reset_all();

verinum*make_unsized_binary(const char*txt);
verinum*make_undef_highz_dec(const char*txt);
verinum*make_unsized_dec(const char*txt);
verinum*make_unsized_octal(const char*txt);
verinum*make_unsized_hex(const char*txt);

static int dec_buf_div2(char *buf);

static int get_timescale_scale(const char*cp);
static int get_timescale_const(int scale, const char*units);

static void process_ucdrive(const char*txt);

static list<int> keyword_mask_stack;

static int comment_enter;
static bool in_module = false;
static bool in_UDP = false;
bool in_celldefine = false;
UCDriveType uc_drive = UCD_NONE;
static int ts_state = 0;
static int ts_scale = 0;
static int ts_unit = 0;
static int ts_prec = 0;

/*
 * The parser sometimes needs to indicate to the lexor that the next
 * identifier needs to be understood in the context of a package. The
 * parser feeds back that left context with calls to the
 * lex_in_package_scope.
 */
static PPackage* in_package_scope = 0;
void lex_in_package_scope(PPackage*pkg)
{
      in_package_scope = pkg;
}

%}

%x CCOMMENT
%x PCOMMENT
%x LCOMMENT
%x CSTRING
%s UDPTABLE
%x PPTIMESCALE_SCALE
%x PPTIMESCALE_UNITS
%x PPTIMESCALE_SLASH
%x PPTIMESCALE_ERROR
%x PPUCDRIVE
%x PPUCDRIVE_ERROR
%x PPDEFAULT_NETTYPE
%x PPDEFAULT_NETTYPE_ERROR
%x PPBEGIN_KEYWORDS
%x PPBEGIN_KEYWORDS_ERROR
%s EDGES
%x REAL_SCALE

W [ \t\b\f\r]+

S [afpnumkKMGT]

TU [munpf]

%%

  /* Recognize the various line directives. */
^"#line"[ \t]+.+ { line_directive(); }
^[ \t]*"`line"[ \t]+.+ { line_directive2(); }

[ \t\b\f\r] { ; }
\n { yylloc.first_line += 1; }

  /* C++ style comments start with / / and run to the end of the
     current line. These are very easy to handle. The meta-comments
     format is a little more tricky to handle, but do what we can. */

  /* The lexor detects "// synthesis translate_on/off" meta-comments,
     we handle them here by turning on/off a flag. The pform uses
     that flag to attach implicit attributes to "initial" and
     "always" statements. */

"// Icarus preprocessor had ("[0-9]+") errors."\n { pre_process_failed(yytext); }
"//"{W}*"synthesis"{W}+"translate_on"{W}*\n { pform_mc_translate_on(true); }
"//"{W}*"synthesis"{W}+"translate_off"{W}*\n { pform_mc_translate_on(false); }
"//" { comment_enter = YY_START; BEGIN(LCOMMENT); }
<LCOMMENT>.    { yymore(); }
<LCOMMENT>\n   { yylloc.first_line += 1; BEGIN(comment_enter); }


  /* The contents of C-style comments are ignored, like white space. */

"/*" { comment_enter = YY_START; BEGIN(CCOMMENT); }
<CCOMMENT>.    { ; }
  /* Check for a possible nested comment. */
<CCOMMENT>"/*" { VLerror(yylloc, "error: Possible nested comment."); }
<CCOMMENT>\n   { yylloc.first_line += 1; }
<CCOMMENT>"*/" { BEGIN(comment_enter); }


"(*" { return K_PSTAR; }
"*)" { return K_STARP; }
".*" { return K_DOTSTAR; }
"<<" { return K_LS; }
"<<<" { return K_LS; /* Note: Functionally, <<< is the same as <<. */}
">>"  { return K_RS; }
">>>" { return K_RSS; }
"**" { return K_POW; }
"<=" { return K_LE; }
">=" { return K_GE; }
"=>" { return K_EG; }
"+=>"|"-=>"	{
			/*
			 * Resolve the ambiguity between the += assignment
			 * operator and +=> polarity edge path operator
			 *
			 * +=> should be treated as two separate tokens '+' and
			 * '=>' (K_EG), therefore we only consume the first
			 * character of the matched pattern i.e. either + or -
			 * and push back the rest of the matches text (=>) in
			 * the input stream.
			 */
			yyless(1);
			return yytext[0];
		}
"*>" { return K_SG; }
"==" { return K_EQ; }
"!=" { return K_NE; }
"===" { return K_CEQ; }
"!==" { return K_CNE; }
"==?" { return K_WEQ; }
"!=?" { return K_WNE; }
"||" { return K_LOR; }
"&&" { return K_LAND; }
"<->" { return K_LEQUIV; }
"&&&" { return K_TAND; }
"~|" { return K_NOR; }
"~^" { return K_NXOR; }
"^~" { return K_NXOR; }
"~&" { return K_NAND; }
"->" { return K_TRIGGER; }
"->>" { return K_NB_TRIGGER; }
"+:" { return K_PO_POS; }
"-:" { return K_PO_NEG; }
"<+" { return K_CONTRIBUTE; }
"+=" { return K_PLUS_EQ; }
"-=" { return K_MINUS_EQ; }
"*=" { return K_MUL_EQ; }
"/=" { return K_DIV_EQ; }
"%=" { return K_MOD_EQ; }
"&=" { return K_AND_EQ; }
"|=" { return K_OR_EQ; }
"^=" { return K_XOR_EQ; }
"<<=" { return K_LS_EQ; }
">>=" { return K_RS_EQ; }
"<<<=" { return K_LS_EQ; }
">>>=" { return K_RSS_EQ; }
"++" { return K_INCR; }
"--" {return K_DECR; }
"'{" { return K_LP; }
"::" { return K_SCOPE_RES; }

  /* This is horrible. The Verilog systax uses "->" in a lot of places.
     The trickiest is in constraints, where it is not an operator at all
     but a constraint implication. This only turns up as a problem when
     the "->" is followed by a constraint_expression_list. If that shows
     up, then there will be a "{" to the right of the "->". In that case,
     turn the "->" into a K_CONSTRAINT_IMPL so that the parser can be
     written without the shift/reduce conflict. Ugh! */
"->"/{W}*"{" { return gn_system_verilog()? K_CONSTRAINT_IMPL :  K_TRIGGER; }

  /* Watch out for the tricky case of (*). Cannot parse this as "(*"
     and ")", but since I know that this is really ( * ), replace it
     with "*" and return that. */
"("{W}*"*"{W}*")" { return '*'; }

<EDGES>"]" { BEGIN(0); return yytext[0]; }
[}{;:\[\],()#=.@&!?<>%|^~+*/-] { return yytext[0]; }

\"            { BEGIN(CSTRING); }
<CSTRING>\\\\ { yymore(); /* Catch \\, which is a \ escaping itself */ }
<CSTRING>\\\" { yymore(); /* Catch \", which is an escaped quote */ }
<CSTRING>\\\n { yymore(); /* Catch \\n, which will be filtered out */
		yylloc.first_line += 1; }
<CSTRING>\n   { BEGIN(0);
		yylval.text = yytext_string_filter(yytext, yyleng);
		VLerror(yylloc, "error: Missing closing quote for string.");
		yylloc.first_line += 1;
		return STRING; }
<CSTRING>\"   { BEGIN(0);
		yylval.text = yytext_string_filter(yytext, yyleng);
		yylval.text[strlen(yylval.text)-1] = 0;
		return STRING; }
<CSTRING>.    { yymore(); }

  /* The UDP Table is a unique lexical environment. These are most
     tokens that we can expect in a table. */
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
<UDPTABLE>[01\?\*\-:;] { return yytext[0]; }

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
	    assert(yylloc.lexical_pos != UINT_MAX);
	    yylloc.lexical_pos += 1;
	    yylval.text = strdupnew(yytext);
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

	  case K_primitive:
	    in_UDP = true;
	    break;

	  case K_endprimitive:
	    in_UDP = false;
	    break;

	  case K_table:
	    BEGIN(UDPTABLE);
	    break;

	  default:
	    yylval.text = 0;
	    break;
      }

	/* Special case: If this is part of a scoped name, then check
	   the package for identifier details. For example, if the
	   source file is  foo::bar, the parse.y will note the
	   PACKAGE_IDENTIFIER and "::" token and mark the
	   "in_package_scope" variable. Then this lexor will see the
	   identifier here and interpret it in the package scope. */
      if (in_package_scope) {
	    if (rc == IDENTIFIER) {
		  if (typedef_t*type = pform_test_type_identifier(in_package_scope, yylval.text)) {
			yylval.type_identifier.text = yylval.text;
			yylval.type_identifier.type = type;
			rc = TYPE_IDENTIFIER;
		  }
	    }
	    in_package_scope = 0;
	    return rc;
      }

	/* If this identifier names a discipline, then return this as
	   a DISCIPLINE_IDENTIFIER and return the discipline as the
	   value instead. */
      if (rc == IDENTIFIER && gn_verilog_ams_flag) {
	    perm_string tmp = lex_strings.make(yylval.text);
	    map<perm_string,ivl_discipline_t>::iterator cur = disciplines.find(tmp);
	    if (cur != disciplines.end()) {
		  delete[]yylval.text;
		  yylval.discipline = (*cur).second;
		  rc = DISCIPLINE_IDENTIFIER;
	    }
      }

	/* If this identifier names a previously declared package, then
	   return this as a PACKAGE_IDENTIFIER instead. */
      if (rc == IDENTIFIER && gn_system_verilog()) {
	    if (PPackage*pkg = pform_test_package_identifier(yylval.text)) {
		  delete[]yylval.text;
		  yylval.package = pkg;
		  rc = PACKAGE_IDENTIFIER;
	    }
      }

	/* If this identifier names a previously declared type, then
	   return this as a TYPE_IDENTIFIER instead. */
      if (rc == IDENTIFIER && gn_system_verilog()) {
	    if (typedef_t*type = pform_test_type_identifier(yylloc, yylval.text)) {
		  yylval.type_identifier.text = yylval.text;
		  yylval.type_identifier.type = type;
		  rc = TYPE_IDENTIFIER;
	    }
      }

      return rc;
  }


\\[^ \t\b\f\r\n]+         {
      assert(yylloc.lexical_pos != UINT_MAX);
      yylloc.lexical_pos += 1;
      yylval.text = strdupnew(yytext+1);
      if (gn_system_verilog()) {
	    if (PPackage*pkg = pform_test_package_identifier(yylval.text)) {
		  delete[]yylval.text;
		  yylval.package = pkg;
		  return PACKAGE_IDENTIFIER;
	    }
      }
      if (gn_system_verilog()) {
	    if (typedef_t*type = pform_test_type_identifier(yylloc, yylval.text)) {
		  yylval.type_identifier.text = yylval.text;
		  yylval.type_identifier.type = type;
		  return TYPE_IDENTIFIER;
	    }
      }
      return IDENTIFIER;
  }

\$([a-zA-Z0-9$_]+)        {
	/* The 1364-1995 timing checks. */
      if (strcmp(yytext,"$hold") == 0)
	    return K_Shold;
      if (strcmp(yytext,"$nochange") == 0)
	    return K_Snochange;
      if (strcmp(yytext,"$period") == 0)
	    return K_Speriod;
      if (strcmp(yytext,"$recovery") == 0)
	    return K_Srecovery;
      if (strcmp(yytext,"$setup") == 0)
	    return K_Ssetup;
      if (strcmp(yytext,"$setuphold") == 0)
	    return K_Ssetuphold;
      if (strcmp(yytext,"$skew") == 0)
	    return K_Sskew;
      if (strcmp(yytext,"$width") == 0)
	    return K_Swidth;
	/* The new 1364-2001 timing checks. */
      if (strcmp(yytext,"$fullskew") == 0)
	    return K_Sfullskew;
      if (strcmp(yytext,"$recrem") == 0)
	    return K_Srecrem;
      if (strcmp(yytext,"$removal") == 0)
	    return K_Sremoval;
      if (strcmp(yytext,"$timeskew") == 0)
	    return K_Stimeskew;

      if (strcmp(yytext,"$attribute") == 0)
	    return KK_attribute;

      if (gn_system_verilog() && strcmp(yytext,"$unit") == 0) {
	    yylval.package = pform_units.back();
	    return PACKAGE_IDENTIFIER;
      }

      yylval.text = strdupnew(yytext);
      return SYSTEM_IDENTIFIER; }


\'[sS]?[dD][ \t]*[0-9][0-9_]* {
      yylval.number = make_unsized_dec(yytext);
      return BASED_NUMBER;
}
\'[sS]?[dD][ \t]*[xzXZ?]_* {
      yylval.number = make_undef_highz_dec(yytext);
      return BASED_NUMBER;
}
\'[sS]?[bB][ \t]*[0-1xzXZ?][0-1xzXZ?_]* {
      yylval.number = make_unsized_binary(yytext);
      return BASED_NUMBER;
}
\'[sS]?[oO][ \t]*[0-7xzXZ?][0-7xzXZ?_]* {
      yylval.number = make_unsized_octal(yytext);
      return BASED_NUMBER;
}
\'[sS]?[hH][ \t]*[0-9a-fA-FxzXZ?][0-9a-fA-FxzXZ?_]* {
      yylval.number = make_unsized_hex(yytext);
      return BASED_NUMBER;
}
\'[01xzXZ] {
      if (!gn_system_verilog()) {
	    VLwarn(yylloc, "warning: Using SystemVerilog 'N bit vector. "
                           "Use at least -g2005-sv to remove this warning.");
      }
      generation_t generation_save = generation_flag;
      generation_flag = GN_VER2005_SV;
      yylval.number = make_unsized_binary(yytext);
      generation_flag = generation_save;
      return UNBASED_NUMBER; }

  /* Decimal numbers are the usual. But watch out for the UDPTABLE
     mode, where there are no decimal numbers. Reject the match if we
     are in the UDPTABLE state. */
[0-9][0-9_]* {
      if (YY_START==UDPTABLE) {
	    REJECT;
      } else {
	    yylval.number = make_unsized_dec(yytext);
	    based_size = yylval.number->as_ulong();
	    return DEC_NUMBER;
      }
}

  /* This rule handles scaled time values for SystemVerilog. */
[0-9][0-9_]*(\.[0-9][0-9_]*)?{TU}?s {
      if (gn_system_verilog()) {
	    yylval.text = strdupnew(yytext);
	    return TIME_LITERAL;
      } else REJECT; }

  /* These rules handle the scaled real literals from Verilog-AMS. The
     value is a number with a single letter scale factor. If
     verilog-ams is not enabled, then reject this rule. If it is
     enabled, then collect the scale and use it to scale the value. */
[0-9][0-9_]*\.[0-9][0-9_]*/{S} {
      if (!gn_verilog_ams_flag) REJECT;
      BEGIN(REAL_SCALE);
      yymore();  }

[0-9][0-9_]*/{S} {
      if (!gn_verilog_ams_flag) REJECT;
      BEGIN(REAL_SCALE);
      yymore();  }

<REAL_SCALE>{S} {
      size_t token_len = strlen(yytext);
      char*tmp = new char[token_len + 5];
      int scale = 0;
      strcpy(tmp, yytext);
      switch (tmp[token_len-1]) {
	  case 'a': scale = -18; break; /* atto- */
	  case 'f': scale = -15; break; /* femto- */
	  case 'p': scale = -12; break; /* pico- */
	  case 'n': scale = -9;  break; /* nano- */
	  case 'u': scale = -6;  break; /* micro- */
	  case 'm': scale = -3;  break; /* milli- */
	  case 'k': scale = 3;  break; /* kilo- */
	  case 'K': scale = 3;  break; /* kilo- */
	  case 'M': scale = 6;  break; /* mega- */
	  case 'G': scale = 9;  break; /* giga- */
	  case 'T': scale = 12; break; /* tera- */
	  default: assert(0); break;
      }
      snprintf(tmp+token_len-1, 5, "e%d", scale);
      yylval.realtime = new verireal(tmp);
      delete[]tmp;

      BEGIN(0);
      return REALTIME;  }

[0-9][0-9_]*\.[0-9][0-9_]*([Ee][+-]?[0-9][0-9_]*)? {
      yylval.realtime = new verireal(yytext);
      return REALTIME; }

[0-9][0-9_]*[Ee][+-]?[0-9][0-9_]* {
      yylval.realtime = new verireal(yytext);
      return REALTIME; }


  /* Notice and handle the `timescale directive. */

`timescale { ts_state = 0; BEGIN(PPTIMESCALE_SCALE); }

<PPTIMESCALE_SCALE>10?0? {
      ts_scale = get_timescale_scale(yytext);
      BEGIN(PPTIMESCALE_UNITS); }

<PPTIMESCALE_SCALE>"//" { comment_enter = PPTIMESCALE_SCALE; BEGIN(LCOMMENT); }
<PPTIMESCALE_SCALE>"/*" { comment_enter = PPTIMESCALE_SCALE; BEGIN(CCOMMENT); }
<PPTIMESCALE_SCALE>"\n" { yylloc.first_line += 1; }
<PPTIMESCALE_SCALE>{W}  { ; }
<PPTIMESCALE_SCALE>.    { BEGIN(PPTIMESCALE_ERROR); }

<PPTIMESCALE_UNITS>[munpf]?s {
      if (++ts_state == 1) {
            ts_unit = get_timescale_const(ts_scale, yytext);
            BEGIN(PPTIMESCALE_SLASH);
      } else {
            ts_prec = get_timescale_const(ts_scale, yytext);
            if (in_module) {
                  VLerror(yylloc, "error: timescale directive cannot be inside "
                          "a module definition.");
            }
            if (ts_unit < ts_prec) {
                  VLerror(yylloc, "error: timescale unit must not be less than "
                          "the precision.");
            } else {
                  pform_set_timescale(ts_unit, ts_prec, yylloc.text, yylloc.first_line);
            }
            BEGIN(0);
      } }

<PPTIMESCALE_UNITS>"//" { comment_enter = PPTIMESCALE_UNITS; BEGIN(LCOMMENT); }
<PPTIMESCALE_UNITS>"/*" { comment_enter = PPTIMESCALE_UNITS; BEGIN(CCOMMENT); }
<PPTIMESCALE_UNITS>"\n" { yylloc.first_line += 1; }
<PPTIMESCALE_UNITS>{W}  { ; }
<PPTIMESCALE_UNITS>.    { BEGIN(PPTIMESCALE_ERROR); }

<PPTIMESCALE_SLASH>"/"  { BEGIN(PPTIMESCALE_SCALE); }

<PPTIMESCALE_SLASH>"//" { comment_enter = PPTIMESCALE_SLASH; BEGIN(LCOMMENT); }
<PPTIMESCALE_SLASH>"/*" { comment_enter = PPTIMESCALE_SLASH; BEGIN(CCOMMENT); }
<PPTIMESCALE_SLASH>"\n" { yylloc.first_line += 1; }
<PPTIMESCALE_SLASH>{W}  { ; }
<PPTIMESCALE_SLASH>.    { BEGIN(PPTIMESCALE_ERROR); }

  /* On error, try to recover by skipping to the end of the line. */
<PPTIMESCALE_ERROR>[^\n]+ {
      VLerror(yylloc, "error: Invalid `timescale directive.");
      BEGIN(0); }

  /* Notice and handle the `celldefine and `endcelldefine directives. */

`celldefine    { in_celldefine = true; }
`endcelldefine { in_celldefine = false; }

  /* Notice and handle the `resetall directive. */

`resetall {
      if (in_module) {
	    VLerror(yylloc, "error: `resetall directive cannot be inside a "
		    "module definition.");
      } else if (in_UDP) {
	    VLerror(yylloc, "error: `resetall directive cannot be inside a "
		    "UDP definition.");
      } else {
	    reset_all();
      } }

  /* Notice and handle the `unconnected_drive directive. */

`unconnected_drive { BEGIN(PPUCDRIVE); }

<PPUCDRIVE>[a-zA-Z0-9_]+ {
      process_ucdrive(yytext);
      if (in_module) {
	    VLerror(yylloc, "error: `unconnected_drive directive cannot be "
		    "inside a module definition.");
      }
      BEGIN(0); }

<PPUCDRIVE>"//" { comment_enter = PPUCDRIVE; BEGIN(LCOMMENT); }
<PPUCDRIVE>"/*" { comment_enter = PPUCDRIVE; BEGIN(CCOMMENT); }
<PPUCDRIVE>"\n" { yylloc.first_line += 1; }
<PPUCDRIVE>{W}  { ; }
<PPUCDRIVE>.    { BEGIN(PPUCDRIVE_ERROR); }

  /* On error, try to recover by skipping to the end of the line. */
<PPUCDRIVE_ERROR>[^\n]+ {
      VLerror(yylloc, "error: Invalid `unconnected_drive directive.");
      BEGIN(0); }

  /* Notice and handle the `nounconnected_drive directive. */

`nounconnected_drive {
      if (in_module) {
	    VLerror(yylloc, "error: `nounconnected_drive directive cannot be "
                    "inside a module definition.");
      }
      uc_drive = UCD_NONE; }

  /* These are directives that I do not yet support. I think that IVL
     should handle these, not an external preprocessor. */
  /* From 1364-2005 Chapter 19. */
^{W}?`pragma{W}?.*                  {  }

  /* From 1364-2005 Annex D. */
^{W}?`default_decay_time{W}?.*      {  }
^{W}?`default_trireg_strength{W}?.* {  }
^{W}?`delay_mode_distributed{W}?.*  {  }
^{W}?`delay_mode_path{W}?.*         {  }
^{W}?`delay_mode_unit{W}?.*         {  }
^{W}?`delay_mode_zero{W}?.*         {  }

  /* From other places. */
^{W}?`disable_portfaults{W}?.*      {  }
^{W}?`enable_portfaults{W}?.*       {  }
`endprotect                         {  }
^{W}?`nosuppress_faults{W}?.*       {  }
`protect                            {  }
^{W}?`suppress_faults{W}?.*         {  }
^{W}?`uselib{W}?.*                  {  }

  /* Notice and handle the `begin_keywords directive. */

`begin_keywords { BEGIN(PPBEGIN_KEYWORDS); }

<PPBEGIN_KEYWORDS>\"[a-zA-Z0-9 -\.]*\" {
      keyword_mask_stack.push_front(lexor_keyword_mask);

      char*word = yytext+1;
      char*tail = strchr(word, '"');
      tail[0] = 0;
      if (strcmp(word,"1364-1995") == 0) {
	    lexor_keyword_mask = GN_KEYWORDS_1364_1995;
      } else if (strcmp(word,"1364-2001") == 0) {
	    lexor_keyword_mask = GN_KEYWORDS_1364_1995
		                |GN_KEYWORDS_1364_2001
		                |GN_KEYWORDS_1364_2001_CONFIG;
      } else if (strcmp(word,"1364-2001-noconfig") == 0) {
	    lexor_keyword_mask = GN_KEYWORDS_1364_1995
		                |GN_KEYWORDS_1364_2001;
      } else if (strcmp(word,"1364-2005") == 0) {
	    lexor_keyword_mask = GN_KEYWORDS_1364_1995
		                |GN_KEYWORDS_1364_2001
		                |GN_KEYWORDS_1364_2001_CONFIG
		                |GN_KEYWORDS_1364_2005;
      } else if (strcmp(word,"1800-2005") == 0) {
	    lexor_keyword_mask = GN_KEYWORDS_1364_1995
		                |GN_KEYWORDS_1364_2001
		                |GN_KEYWORDS_1364_2001_CONFIG
		                |GN_KEYWORDS_1364_2005
		                |GN_KEYWORDS_1800_2005;
      } else if (strcmp(word,"1800-2009") == 0) {
	    lexor_keyword_mask = GN_KEYWORDS_1364_1995
		                |GN_KEYWORDS_1364_2001
		                |GN_KEYWORDS_1364_2001_CONFIG
		                |GN_KEYWORDS_1364_2005
		                |GN_KEYWORDS_1800_2005
		                |GN_KEYWORDS_1800_2009;
      } else if (strcmp(word,"1800-2012") == 0) {
	    lexor_keyword_mask = GN_KEYWORDS_1364_1995
		                |GN_KEYWORDS_1364_2001
		                |GN_KEYWORDS_1364_2001_CONFIG
		                |GN_KEYWORDS_1364_2005
		                |GN_KEYWORDS_1800_2005
		                |GN_KEYWORDS_1800_2009
		                |GN_KEYWORDS_1800_2012;
      } else if (strcmp(word,"VAMS-2.3") == 0) {
	    lexor_keyword_mask = GN_KEYWORDS_1364_1995
		                |GN_KEYWORDS_1364_2001
		                |GN_KEYWORDS_1364_2001_CONFIG
		                |GN_KEYWORDS_1364_2005
		                |GN_KEYWORDS_VAMS_2_3;
      } else {
	    fprintf(stderr, "%s:%d: Ignoring unknown keywords string: %s\n",
		    yylloc.text, yylloc.first_line, word);
      }
      BEGIN(0);
 }

<PPBEGIN_KEYWORDS>"//" { comment_enter = PPBEGIN_KEYWORDS; BEGIN(LCOMMENT); }
<PPBEGIN_KEYWORDS>"/*" { comment_enter = PPBEGIN_KEYWORDS; BEGIN(CCOMMENT); }
<PPBEGIN_KEYWORDS>"\n" { yylloc.first_line += 1; }
<PPBEGIN_KEYWORDS>{W}  { ; }
<PPBEGIN_KEYWORDS>.    { BEGIN(PPBEGIN_KEYWORDS_ERROR); }

  /* On error, try to recover by skipping to the end of the line. */
<PPBEGIN_KEYWORDS_ERROR>[^\n]+ {
      VLerror(yylloc, "error: Invalid `begin_keywords directive.");
      BEGIN(0); }

  /* Notice and handle the `end_keywords directive. */

`end_keywords {
      if (!keyword_mask_stack.empty()) {
	    lexor_keyword_mask = keyword_mask_stack.front();
	    keyword_mask_stack.pop_front();
      } else {
	    VLwarn(yylloc, "warning: Mismatched `end_keywords directive");
      }
 }

  /* Notice and handle the `default_nettype directive. */

`default_nettype { BEGIN(PPDEFAULT_NETTYPE); }

<PPDEFAULT_NETTYPE>[a-zA-Z0-9_]+ {
      NetNet::Type net_type;
  /* Add support for other wire types and better error detection. */
      if (strcmp(yytext,"wire") == 0) {
	    net_type = NetNet::WIRE;

      } else if (strcmp(yytext,"tri") == 0) {
	    net_type = NetNet::TRI;

      } else if (strcmp(yytext,"tri0") == 0) {
	    net_type = NetNet::TRI0;

      } else if (strcmp(yytext,"tri1") == 0) {
	    net_type = NetNet::TRI1;

      } else if (strcmp(yytext,"wand") == 0) {
	    net_type = NetNet::WAND;

      } else if (strcmp(yytext,"triand") == 0) {
	    net_type = NetNet::TRIAND;

      } else if (strcmp(yytext,"wor") == 0) {
	    net_type = NetNet::WOR;

      } else if (strcmp(yytext,"trior") == 0) {
	    net_type = NetNet::TRIOR;

      } else if (strcmp(yytext,"none") == 0) {
	    net_type = NetNet::NONE;

      } else {
	    VLerror(yylloc, "error: Net type '%s' is not a valid (or supported) "
		            "default net type.", yytext);
	    net_type = NetNet::WIRE;
	    error_count += 1;
      }
      pform_set_default_nettype(net_type, yylloc.text, yylloc.first_line);
      BEGIN(0);
  }

<PPDEFAULT_NETTYPE>"//" { comment_enter = PPDEFAULT_NETTYPE; BEGIN(LCOMMENT); }
<PPDEFAULT_NETTYPE>"/*" { comment_enter = PPDEFAULT_NETTYPE; BEGIN(CCOMMENT); }
<PPDEFAULT_NETTYPE>"\n" { yylloc.first_line += 1; }
<PPDEFAULT_NETTYPE>{W}  { ; }
<PPDEFAULT_NETTYPE>.    { BEGIN(PPDEFAULT_NETTYPE_ERROR); }

  /* On error, try to recover by skipping to the end of the line. */
<PPDEFAULT_NETTYPE_ERROR>[^\n]+ {
      VLerror(yylloc, "error: Invalid `default_nettype directive.");
      BEGIN(0); }

  /* These are directives that are not supported by me and should have
     been handled by an external preprocessor such as ivlpp. */

^{W}?`define{W}?.* {
      VLwarn(yylloc, "warning: `define not supported. Use an external preprocessor.");
  }

^{W}?`else{W}?.* {
      VLwarn(yylloc, "warning: `else not supported. Use an external preprocessor.");
  }

^{W}?`elsif{W}?.* {
      VLwarn(yylloc, "warning: `elsif not supported. Use an external preprocessor.");
  }

^{W}?`endif{W}?.* {
      VLwarn(yylloc, "warning: `endif not supported. Use an external preprocessor.");
  }

^{W}?`ifdef{W}?.* {
      VLwarn(yylloc, "warning: `ifdef not supported. Use an external preprocessor.");
  }

^{W}?`ifndef{W}?.* {
      VLwarn(yylloc, "warning: `ifndef not supported. Use an external preprocessor.");
  }

^`include{W}?.* {
      VLwarn(yylloc, "warning: `include not supported. Use an external preprocessor.");
  }

^`undef{W}?.* {
      VLwarn(yylloc, "warning: `undef not supported. Use an external preprocessor.");
  }

`[a-zA-Z_]+ {
      VLwarn(yylloc, "warning: Macro replacement not supported. "
             "Use an external preprocessor.");
  }


`{W} { VLerror(yylloc, "error: Stray tic (`) here. Perhaps you put white "
                       "space between the tic and preprocessor directive?"); }

. { return yytext[0]; }

  /* Final catchall. something got lost or mishandled. */
  /* XXX Should we tell the user something about the lexical state? */

<*>.|\n {
      if (isprint(yytext[0]))
            VLerror(yylloc, "error: Unmatched character (%c).", yytext[0]);
      else
            VLerror(yylloc, "error: Unmatched character (0x%x).",
			    (unsigned char) yytext[0]);
}

%%

/*
 * The UDP state table needs some slightly different treatment by the
 * lexor. The level characters are normally accepted as other things,
 * so the parser needs to switch my mode when it believes in needs to.
 */
void lex_end_table()
{
      BEGIN(INITIAL);
}

static unsigned truncate_to_integer_width(verinum::V*bits, unsigned size)
{
      if (size <= integer_width) return size;

      verinum::V pad = bits[size-1];
      if (pad == verinum::V1) pad = verinum::V0;

      for (unsigned idx = integer_width; idx < size; idx += 1) {
	    if (bits[idx] != pad) {
		  VLwarn(yylloc, "warning: Unsized numeric constant truncated "
                                 "to integer width.");
		  break;
	    }
      }
      return integer_width;
}

verinum*make_unsized_binary(const char*txt)
{
      bool sign_flag = false;
      bool single_flag = false;
      const char*ptr = txt;
      assert(*ptr == '\'');
      ptr += 1;

      if (tolower(*ptr) == 's') {
	    sign_flag = true;
	    ptr += 1;
      }

      assert((tolower(*ptr) == 'b') || gn_system_verilog());
      if (tolower(*ptr) == 'b') {
	    ptr += 1;
      } else {
	    assert(sign_flag == false);
	    single_flag = true;
      }

      while (*ptr && ((*ptr == ' ') || (*ptr == '\t')))
	    ptr += 1;

      unsigned size = 0;
      for (const char*idx = ptr ;  *idx ;  idx += 1)
	    if (*idx != '_') size += 1;

      if (size == 0) {
	    VLerror(yylloc, "error: Numeric literal has no digits in it.");
	    verinum*out = new verinum();
	    out->has_sign(sign_flag);
	    out->is_single(single_flag);
	    return out;
      }

      if ((based_size > 0) && (size > based_size)) VLwarn(yylloc,
          "warning: Extra digits given for sized binary constant.");

	// Allocate one extra bit in case we need to zero-extend.
      verinum::V*bits = new verinum::V[size+1];

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

      if (gn_strict_expr_width_flag && (based_size == 0))
	    size = truncate_to_integer_width(bits, size);

      if (sign_flag && (size < integer_width) && (bits[size-1] == verinum::V1))
	    bits[size++] = verinum::V0;

      verinum*out = new verinum(bits, size, false);
      out->has_sign(sign_flag);
      out->is_single(single_flag);
      delete[]bits;
      return out;
}


verinum*make_unsized_octal(const char*txt)
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

      if (based_size > 0) {
            int rem = based_size % 3;
	    if (rem != 0) based_size += 3 - rem;
	    if (size > based_size) VLwarn(yylloc,
	        "warning: Extra digits given for sized octal constant.");
      }

	// Allocate one extra bit in case we need to zero-extend.
      verinum::V*bits = new verinum::V[size+1];

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

      if (gn_strict_expr_width_flag && (based_size == 0))
	    size = truncate_to_integer_width(bits, size);

      if (sign_flag && (size < integer_width) && (bits[size-1] == verinum::V1))
	    bits[size++] = verinum::V0;

      verinum*out = new verinum(bits, size, false);
      out->has_sign(sign_flag);
      delete[]bits;
      return out;
}


verinum*make_unsized_hex(const char*txt)
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

      if (based_size > 0) {
            int rem = based_size % 4;
	    if (rem != 0) based_size += 4 - rem;
	    if (size > based_size) VLwarn(yylloc,
	        "warning: Extra digits given for sized hex constant.");
      }

	// Allocate one extra bit in case we need to zero-extend.
      verinum::V*bits = new verinum::V[size+1];

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

      if (gn_strict_expr_width_flag && (based_size == 0))
	    size = truncate_to_integer_width(bits, size);

      if (sign_flag && (size < integer_width) && (bits[size-1] == verinum::V1))
	    bits[size++] = verinum::V0;

      verinum*out = new verinum(bits, size, false);
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

/* Support a single x, z or ? as a decimal constant (from 1364-2005). */
verinum* make_undef_highz_dec(const char* ptr)
{
      bool signed_flag = false;

      assert(*ptr == '\'');
      /* The number may have decorations of the form 'sd<code>,
         possibly with space between the d and the <code>.
         Also, the 's' is optional, and marks the number as signed. */
      ptr += 1;

      if (tolower(*ptr) == 's') {
	  signed_flag = true;
	  ptr += 1;
      }

      assert(tolower(*ptr) == 'd');
      ptr += 1;

      while (*ptr && ((*ptr == ' ') || (*ptr == '\t')))
	  ptr += 1;

	/* Process the code. */
      verinum::V* bits = new verinum::V[1];
      switch (*ptr) {
	  case 'x':
	  case 'X':
	    bits[0] = verinum::Vx;
	    break;
	  case 'z':
	  case 'Z':
	  case '?':
	    bits[0] = verinum::Vz;
	    break;
	  default:
	    assert(0);
      }
      ptr += 1;
      while (*ptr == '_') ptr += 1;
      assert(*ptr == 0);

      verinum*out = new verinum(bits, 1, false);
      out->has_sign(signed_flag);
      delete[]bits;
      return out;
}

/*
 * Making a decimal number is much easier than the other base numbers
 * because there are no z or x values to worry about. It is much
 * harder than other base numbers because the width needed in bits is
 * hard to calculate.
 */

verinum*make_unsized_dec(const char*ptr)
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

        /* Since we never have the real number of bits that a decimal
           number represents we do not check for extra bits. */
//      if (based_size > 0) { }

      if (gn_strict_expr_width_flag && (based_size == 0))
	    size = truncate_to_integer_width(bits, size);

      verinum*res = new verinum(bits, size, false);
      res->has_sign(signed_flag);

      delete[]bits;
      return res;
}

/*
 * Convert a string to a scale value ("1" -> 0, "10" -> 1, "100" -> 2).
 * We have already checked the string is valid.
 */
static int get_timescale_scale(const char *cp)
{
	/* Skip the 1 digit. */
      assert(*cp == '1');
      cp += 1;

	/* Check the number of zeros after the 1. */
      int scale = strspn(cp, "0");
      assert(scale < 3);
      cp += scale;

      assert(*cp == '\0');
      return scale;
}

/*
 * Convert a scale and a units string to a time unit or precision.
 * We have already checked the string is valid.
 */
static int get_timescale_const(int scale, const char *units)
{
      if (strncmp("s", units, 1) == 0) {
	    return scale;

      } else if (strncmp("ms", units, 2) == 0) {
	    return scale - 3;

      } else if (strncmp("us", units, 2) == 0) {
	    return scale - 6;

      } else if (strncmp("ns", units, 2) == 0) {
	    return scale - 9;

      } else if (strncmp("ps", units, 2) == 0) {
	    return scale - 12;

      } else if (strncmp("fs", units, 2) == 0) {
	    return scale - 15;

      }
      assert(0);
      return 0;
}

/*
 * Process either a pull0 or a pull1.
 */
static void process_ucdrive(const char*txt)
{
      UCDriveType ucd = UCD_NONE;

      const char*cp = txt;
      if (strncmp("pull", cp, 4) != 0) {
	    VLerror(yylloc, "error: pull required for `unconnected_drive "
	                    "directive.");
	    return;
      }
      cp += 4;
      if (*cp == '0') ucd = UCD_PULL0;
      else if (*cp == '1') ucd = UCD_PULL1;
      else {
	    VLerror(yylloc, "error: `unconnected_drive does not support "
                            "'pull%c'.", *cp);
	    return;
      }
      cp += 1;
      if (*cp != '\0') {
	    VLerror(yylloc, "error: Invalid `unconnected_drive directive "
	                    "(extra garbage after pull direction).");
	    return;
      }

      uc_drive = ucd;
}

int yywrap(void)
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
      char *cpr;
	/* Skip any leading space. */
      char *cp = strchr(yytext, '#');
	/* Skip the #line directive. */
      assert(strncmp(cp, "#line", 5) == 0);
      cp += 5;
	/* Skip the space after the #line directive. */
      cp += strspn(cp, " \t");

	/* Find the starting " and skip it. */
      char *fn_start = strchr(cp, '"');
      if (cp != fn_start) {
	    VLerror(yylloc, "error: Invalid #line directive (file name start).");
	    return;
      }
      fn_start += 1;

	/* Find the last ". */
      char *fn_end = strrchr(fn_start, '"');
      if (!fn_end) {
	    VLerror(yylloc, "error: Invalid #line directive (file name end).");
	    return;
      }

	/* Copy the file name and assign it to yylloc. */
      char*buf = new char[fn_end-fn_start+1];
      strncpy(buf, fn_start, fn_end-fn_start);
      buf[fn_end-fn_start] = 0;

	/* Skip the space after the file name. */
      cp = fn_end;
      cp += 1;
      cpr = cp;
      cpr += strspn(cp, " \t");
      if (cp == cpr) {
	    VLerror(yylloc, "error: Invalid #line directive (missing space "
	                    "after file name).");
	    delete[] buf;
	    return;
      }
      cp = cpr;

	/* Get the line number and verify that it is correct. */
      unsigned long lineno = strtoul(cp, &cpr, 10);
      if (cp == cpr) {
	    VLerror(yylloc, "error: Invalid line number for #line directive.");
	    delete[] buf;
	    return;
      }
      cp = cpr;

	/* Verify that only space is left. */
      cpr += strspn(cp, " \t");
      if ((size_t)(cpr-yytext) != strlen(yytext)) {
	    VLerror(yylloc, "error: Invalid #line directive (extra garbage "
	                    "after line number).");
	    delete[] buf;
	    return;
      }

	/* Now we can assign the new values to yyloc. */
      yylloc.text = set_file_name(buf);
      yylloc.first_line = lineno;
}

/*
 * The line directive matches lines of the form `line N "foo" M and
 * calls this function. Here I parse out the file name and line
 * number, and change the yylloc to suite. M is ignored.
 */
static void line_directive2()
{
      char *cpr;
	/* Skip any leading space. */
      char *cp = strchr(yytext, '`');
	/* Skip the `line directive. */
      assert(strncmp(cp, "`line", 5) == 0);
      cp += 5;

	/* strtol skips leading space. */
      long lineno = strtol(cp, &cpr, 10);
      if (cp == cpr) {
	    VLerror(yylloc, "error: Invalid line number for `line directive.");
	    return;
      }
      if (lineno < 1) {
	    VLerror(yylloc, "error: Line number for `line directive most be greater than zero.");
	    return;
      }
      cp = cpr;

	/* Skip the space between the line number and the file name. */
      cpr += strspn(cp, " \t");
      if (cp == cpr) {
	    VLerror(yylloc, "error: Invalid `line directive (missing space "
	                    "after line number).");
	    return;
      }
      cp = cpr;

	/* Find the starting " and skip it. */
      char *fn_start = strchr(cp, '"');
      if (cp != fn_start) {
	    VLerror(yylloc, "error: Invalid `line directive (file name start).");
	    return;
      }
      fn_start += 1;

	/* Find the last ". */
      char*fn_end = strrchr(fn_start, '"');
      if (!fn_end) {
	    VLerror(yylloc, "error: Invalid `line directive (file name end).");
	    return;
      }

	/* Skip the space after the file name. */
      cp = fn_end + 1;
      cpr = cp;
      cpr += strspn(cp, " \t");
      if (cp == cpr) {
	    VLerror(yylloc, "error: Invalid `line directive (missing space "
	                    "after file name).");
	    return;
      }
      cp = cpr;

	/* Check that the level is correct, we do not need the level. */
      if (strspn(cp, "012") != 1) {
	    VLerror(yylloc, "error: Invalid level for `line directive.");
	    return;
      }
      cp += 1;

	/* Verify that only space and/or a single line comment is left. */
      cp += strspn(cp, " \t");
      if (strncmp(cp, "//", 2) != 0 &&
          (size_t)(cp-yytext) != strlen(yytext)) {
	    VLerror(yylloc, "error: Invalid `line directive (extra garbage "
	                    "after level).");
	    return;
      }

	/* Copy the file name and assign it and the line number to yylloc. */
      char*buf = new char[fn_end-fn_start+1];
      strncpy(buf, fn_start, fn_end-fn_start);
      buf[fn_end-fn_start] = 0;

      yylloc.text = set_file_name(buf);
      yylloc.first_line = lineno-1;
}

/*
 * Reset all compiler directives. This will be called when a `resetall
 * directive is encountered or when a new compilation unit is started.
 */
static void reset_all()
{
      pform_set_default_nettype(NetNet::WIRE, yylloc.text, yylloc.first_line);
      in_celldefine = false;
      uc_drive = UCD_NONE;
      pform_set_timescale(def_ts_units, def_ts_prec, 0, 0);
}

extern FILE*vl_input;
void reset_lexor()
{
      yyrestart(vl_input);
      yylloc.first_line = 1;

	/* Announce the first file name. */
      yylloc.text = set_file_name(strdupnew(vl_file.c_str()));

      if (separate_compilation) {
	    reset_all();
	    if (!keyword_mask_stack.empty()) {
		  lexor_keyword_mask = keyword_mask_stack.back();
		  keyword_mask_stack.clear();
	    }
      }
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
