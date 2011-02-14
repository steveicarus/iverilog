
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
 *    aint64_t with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include  "parse_api.h"
# include  "lexor_keyword.h"
# include  "vhdlint.h"
# include  "vhdlreal.h"
# include  "parse_wrap.h"

# include  <cmath>
# include  <cassert>
# include  <iostream>
# include  <set>

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

static bool are_underscores_correct(char* text);
static bool is_based_correct(char* text);
static char* escape_quot_and_dup(char* text);
static char* escape_apostrophe_and_dup(char* text);

static double make_double_from_based(char* text);
static int64_t make_long_from_based(char* text);

static int64_t lpow(int64_t left, int64_t right);
static unsigned short short_from_hex_char(char ch);

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

\'.\' { 
    yylval.text = escape_apostrophe_and_dup(yytext);
    return CHARACTER_LITERAL;
}

(\"([^\"]|(\"\"))*?\")|(\"[^\"]*\")  {
/* first pattern: string literals with doubled quotation mark */
/* second pattern: string literals without doubled quotation */
    yylval.text = escape_quot_and_dup(yytext);
    assert(yylval.text);
    return STRING_LITERAL;
}

[a-zA-Z_][a-zA-Z0-9_]* {
      for (char*cp = yytext ; *cp ; cp += 1)
	    *cp = tolower(*cp);
      int rc = lexor_keyword_code(yytext, yyleng);
      switch (rc) {
	  case IDENTIFIER:
		if(!are_underscores_correct(yytext))
			std::cerr << "An invalid underscore in the identifier:"
                    << yytext << std::endl;
                //yywarn(yylloc, "An invalid underscore in the identifier"); 
	    yylval.text = strdupnew(yytext);
	    break;
	  default:
	    break;
      }
      return rc;
  }
  
\\([^\\]|\\\\)*\\ { /* extended identifiers */
    yylval.text = strdupnew(yytext);
    return IDENTIFIER;    
}

{decimal_literal} {
      char*tmp = new char[strlen(yytext)+1];
      char*dst, *src;
      int rc = INT_LITERAL;
      for (dst = tmp, src = yytext ; *src ; ++src) {
	    if (*src == '_')
		  continue;
	    if (*src == '.')
		  rc = REAL_LITERAL;
	    *dst++ = *src;
      }
      *dst = 0;

      if (rc == REAL_LITERAL) {
	    yylval.uni_real = strtod(tmp, 0);
      } else {
	    yylval.uni_integer = strtoimax(tmp, 0, 10);
      }
      delete[]tmp;
      return rc;
}

{based_literal} {
    if(!are_underscores_correct(yytext) || !is_based_correct(yytext))
        std::cerr << "An invalid form of based literal:" 
            << yytext << std::endl;
            
    if(strchr(yytext, '.'))
    {
        double val = make_double_from_based(yytext);
        yylval.uni_real = val;
        return REAL_LITERAL;
    }
    else
    {
        int64_t val = make_long_from_based(yytext);
        yylval.uni_integer = val;
        return INT_LITERAL;
    }
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

/**
* This function checks if underscores in an identifier
* or in a number are correct.
*
* \return true is returned if underscores are placed
* correctly according to specification
*/
static bool are_underscores_correct(char* text) 
{
	unsigned char underscore_allowed = 0;
	const char* cp;
	for( cp = text; *cp; ++cp)
	{
		if (*cp == '_')
		{
			if (!underscore_allowed || *(cp+1) == '\0')
				return 0;
			underscore_allowed = 0;
		}
		else
			underscore_allowed = 1;
	}
	return 1;
}

/**
* This function checks if the format of a based number
* is correct according to the VHDL standard
*
* \return true is returned if a based number
* is formed well according to specification
*/
static bool is_based_correct(char* text)
{
    char* ptr;
    //BASE examination
    char clean_base[4];
    clean_base[3] = '\0';
    char* clean_base_ptr = clean_base;
    for(ptr = text; ptr != strchr(text, '#'); ++ptr)
    {
        if(*ptr == '_')
            ++ptr;
        if(!(*ptr >= '0' && *ptr <= '9')) //the base uses chars other than digits
            return 0;
        if(*clean_base_ptr == '\0')
            break;
        *clean_base_ptr = *ptr;
        ++clean_base_ptr;
    }
    unsigned length = clean_base_ptr - clean_base;
    unsigned base; 
    if(length > 2 || length == 0)
        return 0; //the base is too big or too small
    if(length == 2)
    {
        base = 10*(clean_base[0] - '0') + (clean_base[1] - '0'); 
        //the base exceeds 16 or equals 0
        if(base > 16 || base == 0)
            return 0;
    }
    else
    { //the base consists of one char and is equal to zero
        base = clean_base[0] - '0';
        if(base == 0)
            return 0;
    }
    bool point = 0;
    set<char> allowed_chars;
    
    unsigned c;
    if(base <= 10) {
        for(c = 0; c < base; ++c)
            allowed_chars.insert(c + '0');
    }
    else
    {
        for(c = 0; c < 10; ++c)
            allowed_chars.insert(c + '0');
        for(c = 0; c < base - 10; ++c)
            allowed_chars.insert(c + 'a');
    }
    //MANTISSA examination
    for(ptr = strchr(text, '#') + 1, length = 0; ptr != strrchr(text, '#'); ++ptr)
    {
        if(*ptr == '.') 
        {
            //we found a dot and another one was already found
            if(point == 1)
                return 0;
            else
            {
                //notice the fact of finding a point and continue, without increasing the length
                point = 1;
                continue;
            }
        }
        //the number consists of other chars than allowed
        if(allowed_chars.find(*ptr) == allowed_chars.end())
            return 0;
        ++length;
    }
    if(length == 0)
        return 0;
    
    //EXPONENT examination
    if(strchr(text, '\0') - strrchr(text, '#') > 1) { //the number contains an exponent
        if(*(strrchr(text, '#') + 2) == '-')
            return 0;
        length = 0;
        for(ptr = strrchr(text, '#')+2; *ptr != '\0'; ++ptr)
        {
            //the exponent consists of other chars than {'0'.,'9','a'..'f'}
            if(!((*ptr >= '0' && *ptr <= '9') || (*ptr >= 'a' && *ptr <= 'f')))
                return 0;
        }
    } 
    return 1;
}

/**
* This function takes a string literal, gets rid of
* quotation marks and copies the remaining characters
* to a new persistent C-string
*
* \return pointer to the new string is returned
*/
static char* escape_quot_and_dup(char* text)
{
    char* newstr = new char[strlen(text)+1];
    
    unsigned old_idx, new_idx;
    for(new_idx = 0, old_idx = 0; old_idx < strlen(text); ) 
    {
        if(text[old_idx] == '"' && old_idx == 0) 
        { //the beginning of the literal
            ++old_idx; 
            continue;
        }
        else
        if(text[old_idx] == '"' && text[old_idx+1] == '\0')
        { //the end
            newstr[new_idx] = '\0';
            return newstr;
        }
        else
        if(text[old_idx] == '"' && text[old_idx+1] == '"') 
        {
            newstr[new_idx++] = '"';
            old_idx += 2; //jump across two chars
        }
        else 
        {
            newstr[new_idx] = text[old_idx];
            ++old_idx;
            ++new_idx;
        }
    }
    //the function should never reach this point
    return 0;
}

/**
* This function takes a character literal, gets rid
* of the apostrophes and returns new C-string
*
* \return pointer to the new string is returned
*/
static char* escape_apostrophe_and_dup(char* text)
{
    char* newstr = new char[2];
    newstr[0] = text[1];
    newstr[1] = '\0';
    return newstr;
}

/**
* This function takes a floating point based number 
* in form of a C-strings and converts it to a double.
*
* \return new double is returned
*/
static double make_double_from_based(char* text)
{
    char* first_hash_ptr = strchr(text, '#');
    char* second_hash_ptr = strrchr(text, '#');
    char* last_char_ptr = strchr(text, '\0') - 1;
    //put null byte in lieu of hashes
    *first_hash_ptr = '\0';
    *second_hash_ptr = '\0';
    
    //now lets deduce the base
    unsigned base = (unsigned)strtol(text, 0, 10) ;
    
    double mantissa = 0.0;
    char*ptr = first_hash_ptr + 1;
    for( ; ptr != second_hash_ptr ; ++ptr)
    {
        if(*ptr == '.')
            break;
        if(*ptr != '_')
        {
            mantissa = mantissa*base + short_from_hex_char(*ptr);
        }
    }
    double fraction = 0.0;
    double factor = 1.0/base;
    for(++ptr ; ptr != second_hash_ptr; ++ptr)
    {
        if(*ptr != '_')
        {
            fraction = fraction + short_from_hex_char(*ptr)*factor;
            factor = factor / base;
        }
    }
    if(last_char_ptr == second_hash_ptr) //there is no exponent
    {
        return mantissa + fraction;
    }
    
    //now calculate the value of the exponent
    double exponent = 0.0;
    //leave 'e'/'E' and '+'
    ptr = *(second_hash_ptr + 2) == '+' ? second_hash_ptr + 3 : second_hash_ptr + 2;  
    
    for( ; *ptr != '\0'; ++ptr)
    {
        if(*ptr != '_')
        {
            exponent = exponent*base + short_from_hex_char(*ptr);
        }
    }
    return pow(mantissa + fraction, exponent);
}   

/** 
* This function takes a hexadecimal digit in form of
* a char and returns its litteral value as short
*/
static unsigned short short_from_hex_char(char ch) 
{
    if(ch >= '0' && ch <= '9')
        return ch - '0';
    else
        return ch - 'a' + 10;
}

/**
* This function takes a based number in form of
* a C-strings and converts it to a int64_t.
*
* \return new double is returned
*/
static int64_t make_long_from_based(char* text) {
    char* first_hash_ptr = strchr(text, '#');
    char* second_hash_ptr = strrchr(text, '#');
    char* end_ptr = strrchr(text, '\0');
    //now lets deduce the base
    *first_hash_ptr = '\0'; 
    unsigned base = (unsigned)strtol(text, 0, 10) ;
    
    char *ptr = first_hash_ptr + 1;
    int64_t mantissa = 0;
    for( ; ptr != second_hash_ptr ; ++ptr)
    {
        if(*ptr != '_')
        {
            mantissa = mantissa * base + short_from_hex_char(*ptr);
        }
    }
    //if there is an exponent
    if(end_ptr - second_hash_ptr > 1)
    {
       int64_t exponent = 0L; 
       
       ptr = *(second_hash_ptr + 2) == '+' ? second_hash_ptr + 3 : second_hash_ptr + 2; 
       for( ; *ptr != '\0'; ++ptr)
       {
           if(*ptr != '_')
               exponent = base*exponent + short_from_hex_char(*ptr);
       }
       return lpow(mantissa, exponent);
    }
    else
        return mantissa;
}

/**
* Recursive power function for int64_t
*/
static int64_t lpow(int64_t left, int64_t right) {
    if(right == 0)
        return 1;
    else
        return left*lpow(left, right - 1);
}

void reset_lexor(FILE*fd, const char*path)
{
      yylloc.text = path;
      yylloc.first_line = 1;
      yyrestart(fd);

      yyparse_set_filepath(path);
}
