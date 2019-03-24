%option prefix="yy"
%option never-interactive
%option nounput
%option reentrant
%option noyywrap

%{
/*
 * Copyright (c) 2011-2017 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

# define YY_NO_INPUT

# define YY_DECL int yylex(YYSTYPE*yylvalp, YYLTYPE*yyllocp, yyscan_t yyscanner)
//class vhdlnum;
//class vhdlreal;

extern int lexor_keyword_code (const char*str, unsigned len);

/*
 * Lexical location information is passed in the yylloc variable to the
 * parser. The file names, strings, are kept in a list so that I can
 * re-use them. The set_file_name function will return a pointer to
 * the name as it exists in the list (and delete the passed string).
 * If the name is new, it will be added to the list.
 */
#define yylloc (*yyllocp)
#define yylval (*yylvalp)

static bool are_underscores_correct(char* text);
static bool is_based_correct(char* text);
static char* escape_quot_and_dup(char* text);
static char* escape_apostrophe_and_dup(char* text);

static double make_double_from_based(char* text);
static int64_t make_long_from_based(char* text);
static char* make_bitstring_literal(const char*text);

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
time			{integer}{W}*([fFpPnNuUmM]?[sS])
%%

[ \t\b\f\r] { ; }
\n { yylloc.first_line += 1; }

  /* Single-line comments start with -- and run to the end of the
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

(\"([^\"]|(\"\"))*?\") {
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
    for(char*cp = yytext ; *cp ; ++cp)
        *cp = tolower(*cp);

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

{integer}?[sSuU]?[xXbBoOdD]\"[^\"]+\" {
      yylval.text = make_bitstring_literal(yytext);
      return BITSTRING_LITERAL;
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
"??" { return CC; }
"?=" { return M_EQ;}
"?/=" { return M_NE;}
"?<" { return M_LT; }
"?<=" { return M_LEQ;}
"?>" { return M_GT; }
"?>=" {return M_GEQ; }

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

static bool is_char_ok(char c, int base)
{
    if(base <= 10)
        return '0' <= c && c - '0' < base;
    else
        return isdigit(c) || (c >= 'a' && c < 'a' + base - 10);
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
    char clean_base[4] = {0,};
    char* clean_base_end = clean_base + sizeof(clean_base);
    char* clean_base_ptr = clean_base;
    for(ptr = text; ptr != strchr(text, '#'); ++ptr)
    {
        if(*ptr == '_')
            ++ptr;
        if(!(*ptr >= '0' && *ptr <= '9')) //the base uses chars other than digits
            return 0;
        if(clean_base_ptr == clean_base_end)
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
    bool point = false;

    //MANTISSA examination
    for(ptr = strchr(text, '#') + 1, length = 0; ptr != strrchr(text, '#'); ++ptr)
    {
        if(*ptr == '.')
        {
            //we found a dot and another one was already found
            if(point)
                return 0;
            else
            {
                //notice the fact of finding a point and continue, without increasing the length
                point = true;
                continue;
            }
        }
        //check if the number consists of other chars than allowed
        if(!is_char_ok(*ptr, base))
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

static char*make_bitstring_bin(int width_prefix, bool sflag, bool,
				  const char*src)
{
      int src_len = strlen(src);
      if (width_prefix < 0)
	    width_prefix = src_len;

      char*res = new char[width_prefix+1];
      char*rp = res;

      if (width_prefix > src_len) {
	    size_t pad = width_prefix - src_len;
	    for (size_t idx = 0 ; idx < pad ; idx += 1)
		  *rp++ = sflag? src[0] : '0';

      } else if (src_len > width_prefix) {
	    src += src_len - width_prefix;
      }

      while (*src) {
	    *rp++ = *src++;
      }
      *rp = 0;

      return res;
}

static char*make_bitstring_oct(int width_prefix, bool sflag, bool,
			       const char*src)
{
      int src_len = strlen(src);
      if (width_prefix < 0)
	    width_prefix = 3*src_len;

      char*res = new char[width_prefix+1];
      char*rp = res + width_prefix;
      *rp = 0;
      rp -= 1;

      for (const char*sp = src + src_len - 1; sp >= src ; sp -= 1) {
	    int val;
	    switch (*sp) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		  val = *sp - '0';
		  *rp-- = (val&1)? '1' : '0';
		  if (rp >= res) *rp-- = (val&2)? '1' : '0';
		  if (rp >= res) *rp-- = (val&4)? '1' : '0';
		  break;
		default:
		  *rp-- = *sp;
		  if (rp >= res) *rp-- = *sp;
		  if (rp >= res) *rp-- = *sp;
		  break;
	    }
	    if (rp < res)
		  break;
      }

      if (rp >= res) {
	    char pad = sflag? src[0] : '0';
	    while (rp >= res)
		  *rp-- = pad;
      }

      return res;
}

static char*make_bitstring_hex(int width_prefix, bool sflag, bool,
			       const char*src)
{
      int src_len = strlen(src);
      if (width_prefix <= 0)
	    width_prefix = 4*src_len;

      char*res = new char[width_prefix+1];
      char*rp = res + width_prefix;
      *rp = 0;
      rp -= 1;

      for (const char*sp = src + src_len - 1; sp >= src ; sp -= 1) {
	    int val;
	    switch (*sp) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		  val = *sp - '0';
		  *rp-- = (val&1)? '1' : '0';
		  if (rp >= res) *rp-- = (val&2)? '1' : '0';
		  if (rp >= res) *rp-- = (val&4)? '1' : '0';
		  if (rp >= res) *rp-- = (val&8)? '1' : '0';
		  break;
		case 'a': case 'A':
		case 'b': case 'B':
		case 'c': case 'C':
		case 'd': case 'D':
		case 'e': case 'E':
		case 'f': case 'F':
		  val = 10 + toupper(*sp) - 'A';
		  *rp-- = (val&1)? '1' : '0';
		  if (rp >= res) *rp-- = (val&2)? '1' : '0';
		  if (rp >= res) *rp-- = (val&4)? '1' : '0';
		  if (rp >= res) *rp-- = (val&8)? '1' : '0';
		  break;
		default:
		  *rp-- = *sp;
		  if (rp >= res) *rp-- = *sp;
		  if (rp >= res) *rp-- = *sp;
		  if (rp >= res) *rp-- = *sp;
		  break;
	    }
	    if (rp < res)
		  break;
      }

      if (rp >= res) {
	    char pad = sflag? src[0] : '0';
	    while (rp >= res)
		  *rp-- = pad;
      }

      return res;
}

static char*make_bitstring_dec(int, bool, bool, const char*)
{
      assert(0);
      return 0;
}

static char* make_bitstring_literal(const char*text)
{
      int width_prefix = -1;
      const char*cp = text;
      bool signed_flag = false;
      bool unsigned_flag = false;
      unsigned base = 0;

	// Parse out the explicit width, if present.
      if (size_t len = strspn(cp, "0123456789")) {
	    width_prefix = 0;
	    while (len > 0) {
		  width_prefix *= 10;
		  width_prefix += *cp - '0';
		  cp += 1;
          --len;
	    }
      } else {
	    width_prefix = -1;
      }

	// Detect and s/u flags.
      if (*cp == 's' || *cp == 'S') {
	    signed_flag = true;
	    cp += 1;
      } else if (*cp == 'u' || *cp == 'U') {
	    unsigned_flag = true;
	    cp += 1;
      }

	// Now get the base marker.
      switch (*cp) {
	  case 'b':
	  case 'B':
	    base = 2;
	    break;
	  case 'o':
	  case 'O':
	    base = 8;
	    break;
	  case 'x':
	  case 'X':
	    base = 16;
	    break;
	  case 'd':
	  case 'D':
	    base = 10;
	    break;
	  default:
	    assert(0);
      }
      cp += 1;

      char*simplified = new char [strlen(cp) + 1];
      char*dp = simplified;
      assert(*cp == '"');
      cp += 1;

      while (*cp && *cp != '"') {
	    if (*cp == '_') {
		  cp += 1;
		  continue;
	    }

	    *dp++ = *cp++;
      }
      *dp = 0;

      char*res;
      switch (base) {
	  case 2:
	    res = make_bitstring_bin(width_prefix, signed_flag, unsigned_flag, simplified);
	    break;
	  case 8:
	    res = make_bitstring_oct(width_prefix, signed_flag, unsigned_flag, simplified);
	    break;
	  case 10:
	    res = make_bitstring_dec(width_prefix, signed_flag, unsigned_flag, simplified);
	    break;
	  case 16:
	    res = make_bitstring_hex(width_prefix, signed_flag, unsigned_flag, simplified);
	    break;
	  default:
	    assert(0);
	    res = 0;
      }

      delete[]simplified;
      return res;
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

    //now let's deduce the base
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

yyscan_t prepare_lexor(FILE*fd)
{
      yyscan_t scanner;
      yylex_init(&scanner);
      yyrestart(fd, scanner);
      return scanner;
}

/*
 * Modern version of flex (>=2.5.9) can clean up the scanner data.
 */
void destroy_lexor(yyscan_t scanner)
{
# ifdef FLEX_SCANNER
#   if YY_FLEX_MAJOR_VERSION >= 2 && YY_FLEX_MINOR_VERSION >= 5
#     if YY_FLEX_MINOR_VERSION > 5 || defined(YY_FLEX_SUBMINOR_VERSION) && YY_FLEX_SUBMINOR_VERSION >= 9
      yylex_destroy(scanner);
#     endif
#   endif
# endif
}
