
%option never-interactive
%option nounput
%option noyywrap

%{
/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
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

# include  "parse_api.h"
# include  "lexor_keyword.h"
# include  "vhdlnum.h"
# include  "vhdlreal.h"
# include  "parse_wrap.h"

# include  <iostream>

//class vhdlnum;
//class vhdlreal;

extern int lexor_keyword_code (const char*str, unsigned len);

/*
 * Lexical location information is passed in the yylloc variable to th
 * parser. The file names, strings, are kept in a list so that I can
 * re-use them. The set_file_name function will return a pointer to
 * the name as it exists in the list (and delete the passed string.)
 * If the name is new, it will be added to the list.
 */
extern YYLTYPE yylloc;

static int check_underscores(char* text);

vhdlnum* make_unisized_dec(char* text);
vhdlnum* make_unisized_based(char* text);

static char* strdupnew(char const *str)
{
       return str ? strcpy(new char [strlen(str)+1], str) : 0;
}

static int comment_enter;

%}

%x CCOMMENT
%x LCOMMENT

W [ \t\b\f\r]+
decimal_literal		{integer}(\.{integer})?({exponent})?
integer				[0-9](_?[0-9])*
exponent			[eE][-+]?{integer}

based_literal		{integer}#{based_integer}(\.{based_integer})?#{exponent}?
based_integer		[0-9a-fA-F](_?[0-9a-fA-F])*
%%

[ \t\b\f\r] { ; }
\n { yylloc.first_line += 1; }

  /* Single-line comments start with - - and run to the end of the
     current line. These are very easy to handle. */

"--".* { comment_enter = YY_START; BEGIN(LCOMMENT); }
<LCOMMENT>.    { yymore(); }
<LCOMMENT>\n   { yylloc.first_line += 1; BEGIN(comment_enter); }



  /* The contents of C-style comments are ignored, like white space. */

"/*" { comment_enter = YY_START; BEGIN(CCOMMENT); }
<CCOMMENT>.    { ; }
<CCOMMENT>\n   { yylloc.first_line += 1; }
<CCOMMENT>"*/" { BEGIN(comment_enter); }


[a-zA-Z][a-zA-Z0-9_]* {
      int rc = lexor_keyword_code(yytext, yyleng);
      switch (rc) {
	  case IDENTIFIER:
		if(check_underscores(yytext))
			std::cerr << "An invalid underscore in the identifier" << std::endl;
                //yywarn(yylloc, "An invalid underscore in the identifier"); 
	    yylval.text = strdupnew(yytext);
	    break;
	  default:
	    break;
      }
      return rc;
  }
  
\\(.|\\\\)*\\ { /* extended identifiers */
    yylval.text = strdupnew(yytext);
    printf("special %s\n", yytext);  
    return IDENTIFIER;    
}

   
{decimal_literal} {
    if(strchr(yytext, '.')) {
        yylval.real = new vhdlreal(yytext);
        return REAL_LITERAL;
    } else {
        yylval.integer = new vhdlnum(yytext); 
        return INT_LITERAL;
    }
}

{based_literal} {
    yylval.integer = new vhdlnum(yytext); 
    return INT_LITERAL;
}

    
  /* Compound symbols */
"<=" { return LEQ; }
">=" { return GEQ; }
":=" { return VASSIGN; }
"/=" { return NE; }
"<>" { return BOX; }
"**" { return EXP; }
"=>" { return ARROW; }
"<<" { return DLT; }
">>" { return DGT; }
    /* 
    Here comes a list of symbols that are more then strange,
    at least for the time being.
    
"??" { return K_CC; }
"?=" {}
"?/=" {}
"?<" {}
"?<=" {}
"?>" {}
"?>=" {}
*/

. { return yytext[0]; }

%%

extern void yyparse_set_filepath(const char*path);

static int check_underscores(char* text) 
{
	unsigned char underscore_allowed = 0;
	const char* cp;
	for( cp = text; *cp; ++cp)
	{
		if (*cp == '_')
		{
			if (!underscore_allowed || *(cp+1) == '\0')
				return 1;
			underscore_allowed = 0;
		}
		else
			underscore_allowed = 1;
	}
	return 0;
}

void reset_lexor(FILE*fd, const char*path)
{
      yylloc.text = path;
      yylloc.first_line = 1;
      yyrestart(fd);

      yyparse_set_filepath(path);
}
