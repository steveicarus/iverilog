
%{
# include "sys_readmem_lex.h"
# include  <string.h>
static void make_hex_value();
static void make_bin_value();
%}

%x BIN
%x HEX

%option noyywrap
%%

<*>"//".* { ; }
<*>[ \t\f\n\r] { ; }

<*>@[0-9a-fA-F]+ { return MEM_ADDRESS; }
<HEX>[0-9a-fA-FxXzZ_]+  { make_hex_value(); return MEM_WORD; }
<BIN>[01_]+  { make_bin_value(); return MEM_WORD; }

%%
static unsigned word_width = 0;
static struct t_vpi_vecval*vecval = 0;

static void make_hex_value()
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
	    int aval, bval;

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
		  bval = 0;
		  break;
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		  aval = *end - 'a' + 10;
		  bval = 0;
		  break;
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		  aval = *end - 'A' + 10;
		  bval = 0;
		  break;
		case 'x':
		case 'X':
		  aval = 15;
		  bval = 15;
		  break;
		case 'z':
		case 'Z':
		  aval = 0;
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
}

static void make_bin_value()
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
	    int aval, bval;

	    end -= 1;
	    if (*end == '_') continue;
	    switch (*end) {
		case '0':
		case '1':
		  aval = *end - '0';
		  bval = 0;
		  break;
		case 'x':
		case 'X':
		  aval = 1;
		  bval = 1;
		  break;
		case 'z':
		case 'Z':
		  aval = 0;
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
}

void sys_readmem_start_file(FILE*in, int bin_flag,
			    unsigned width, struct t_vpi_vecval *vv)
{
      yyin = in;
      YY_FLUSH_BUFFER;
      BEGIN(bin_flag? BIN : HEX);
      word_width = width;
      vecval = vv;
}
