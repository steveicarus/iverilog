%option prefix="readmem"
%option nounput
%option noinput

%{
/*
 * Copyright (c) 1999-2017,2019 Stephen Williams (steve@icarus.com)
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

# include "sys_readmem_lex.h"
# include  <string.h>
static void make_addr(void);
static void make_hex_value(void);
static void make_bin_value(void);

static int save_state;

char *readmem_error_token = 0;

%}

%x BIN
%x HEX
%x CCOMMENT

%option noyywrap
%%

<HEX,BIN>"//".* { ; }
<HEX,BIN>[ \t\f\n\r] { ; }

<HEX,BIN>@[0-9a-fA-F]+ { make_addr(); return MEM_ADDRESS; }
<HEX>[0-9a-fA-FxXzZ_]+  { make_hex_value(); return MEM_WORD; }
<BIN>[01xXzZ_]+  { make_bin_value(); return MEM_WORD; }

<HEX,BIN>"/*"   { save_state = YY_START; BEGIN(CCOMMENT); }
<CCOMMENT>[^*]* { ; }
<CCOMMENT>"*"   { ; }
<CCOMMENT>"*"+"/" { BEGIN(save_state); }

 /* Catch any invalid tokens and flagged them as an error. */
<HEX,BIN>. { readmem_error_token = yytext; return MEM_ERROR; }

%%
 /* The call_handle is the handle for the call to the $readmem()
    system task. This is used for adding line numbers to warnings and
    error messages. */
static vpiHandle call_handle = 0;
static int too_many_digits_warning = 0;

static unsigned word_width = 0;
static struct t_vpi_vecval*vecval = 0;

static void make_addr(void)
{
      sscanf(yytext+1, "%x", (unsigned int*)&vecval->aval);
}

static void make_hex_value(void)
{
      char*beg = yytext;
      char*end = beg + strlen(beg);
      struct t_vpi_vecval*cur;
      int idx;
      int width = 0, word_max = word_width;

      for (idx = 0, cur = vecval ;  idx < word_max ;  idx += 32, cur += 1) {
	    cur->aval = 0;
	    cur->bval = 0;
      }

      cur = vecval;
      while ((width < word_max) && (end > beg)) {
	    int aval = 0;
	    int bval = 0;

	    end -= 1;
	    if (*end == '_') continue;
	    switch (*end) {
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
		  aval = *end - '0';
		  break;
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		  aval = *end - 'a' + 10;
		  break;
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		  aval = *end - 'A' + 10;
		  break;
		case 'x':
		case 'X':
		  aval = 15;
		  bval = 15;
		  break;
		case 'z':
		case 'Z':
		  bval = 15;
		  break;
	    }

	    cur->aval |= aval << width;
	    cur->bval |= bval << width;
	    width += 4;
	    if (width == 32) {
		  cur += 1;
		  width = 0;
		  word_max -= 32;
	    }
      }

	/* If there are more text digits then needed to fill the
	   memory word, count those digits and print a warning
	   message. Print that warning only once per call to
	   $readmemh() so that the user isn't flooded. */
      int count_extra_digits = 0;
      while (end > beg) {
	    end -= 1;
	    if (*end == '_') continue;
	    count_extra_digits += 1;
      }

      if (count_extra_digits && too_many_digits_warning==0) {
	    vpi_printf("WARNING: %s:%d: Excess hex digits (%d of '%s') while reading %d-bit words.\n",
		       vpi_get_str(vpiFile, call_handle),
		       vpi_get(vpiLineNo, call_handle),
		       count_extra_digits, beg,
		       word_width);
	    too_many_digits_warning += 1;
      }
}

static void make_bin_value(void)
{
      char*beg = yytext;
      char*end = beg + strlen(beg);
      struct t_vpi_vecval*cur;
      int idx;
      int width = 0, word_max = word_width;

      for (idx = 0, cur = vecval ;  idx < word_max ;  idx += 32, cur += 1) {
	    cur->aval = 0;
	    cur->bval = 0;
      }

      cur = vecval;
      while ((width < word_max) && (end > beg)) {
	    int aval = 0;
	    int bval = 0;

	    end -= 1;
	    if (*end == '_') continue;
	    switch (*end) {
		case '0':
		case '1':
		  aval = *end - '0';
		  break;
		case 'x':
		case 'X':
		  aval = 1;
		  bval = 1;
		  break;
		case 'z':
		case 'Z':
		  bval = 1;
		  break;
	    }

	    cur->aval |= aval << width;
	    cur->bval |= bval << width;
	    width += 1;
	    if (width == 32) {
		  cur += 1;
		  width = 0;
		  word_max -= 32;
	    }
      }

	/* If there are more text digits then needed to fill the
	   memory word, count those digits and print a warning
	   message. Print that warning only once per call to
	   $readmem() so that the user isn't flooded. */
      int count_extra_digits = 0;
      while (end > beg) {
	    end -= 1;
	    if (*end == '_') continue;
	    count_extra_digits += 1;
      }

      if (count_extra_digits && too_many_digits_warning==0) {
	    vpi_printf("WARNING: %s:%d: Excess binary digits (%d of '%s') while reading %d-bit words.\n",
		       vpi_get_str(vpiFile, call_handle),
		       vpi_get(vpiLineNo, call_handle),
		       count_extra_digits, beg,
		       word_width);
	    too_many_digits_warning += 1;
      }
}

void sys_readmem_start_file(vpiHandle callh, FILE*in, int bin_flag,
			    unsigned width, struct t_vpi_vecval *vv)
{
      call_handle = callh;
      too_many_digits_warning = 0;
      yyrestart(in);
      BEGIN(bin_flag? BIN : HEX);
      word_width = width;
      vecval = vv;
}

/*
 * Modern version of flex (>=2.5.9) can clean up the scanner data.
 */
void destroy_readmem_lexor(void)
{
# ifdef FLEX_SCANNER
#   if YY_FLEX_MAJOR_VERSION >= 2 && YY_FLEX_MINOR_VERSION >= 5
#     if YY_FLEX_MINOR_VERSION > 5 || defined(YY_FLEX_SUBMINOR_VERSION) && YY_FLEX_SUBMINOR_VERSION >= 9
    yylex_destroy();
#     endif
#   endif
# endif
}
