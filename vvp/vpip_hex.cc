/*
 * Copyright (c) 2002 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: vpip_hex.cc,v 1.4 2006/02/21 02:39:27 steve Exp $"
#endif

# include  "config.h"
# include  "vpi_priv.h"
# include  <stdio.h>
# include  <string.h>
# include  <limits.h>     /* for CHAR_BIT */
# include  <stdlib.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <assert.h>

extern const char hex_digits[256];

void vpip_hex_str_to_vec4(vvp_vector4_t&val, const char*str)
{
      unsigned str_len = strlen(str);

      char pad = '0';
      switch (str[0]) {
	  case 'x':
	  case 'X':
	    pad = 'x';
	    break;
	  case 'z':
	  case 'Z':
	    pad = 'z';
	    break;
      }

      for (unsigned idx = 0 ;  idx < val.size() ;  idx += 1) {
	    unsigned tmp;
	    unsigned bit_off = idx%4;
	    unsigned str_off = idx/4;

	    char ch;
	    if (str_off >= str_len)
		  ch = pad;
	    else
		  ch = str[str_len-str_off-1];

	    switch (ch) {
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
		  tmp = ch - '0';
		  val.set_bit(idx, ((tmp>>bit_off)&1)? BIT4_1 : BIT4_0);
		  break;
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		  tmp = ch - 'a' + 10;
		  val.set_bit(idx, ((tmp>>bit_off)&1)? BIT4_1 : BIT4_0);
		  break;
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		  tmp = ch - 'A' + 10;
		  val.set_bit(idx, ((tmp>>bit_off)&1)? BIT4_1 : BIT4_0);
		  break;
		case 'x':
		case 'X':
		  val.set_bit(idx, BIT4_X);
		  break;
		case 'z':
		case 'Z':
		  val.set_bit(idx, BIT4_Z);
		  break;
		default:
		  fprintf(stderr, "Unsupported digit %c(%d).\n", ch, ch);
		  assert(0);
		  break;
	    }
      }
}

void vpip_vec4_to_hex_str(const vvp_vector4_t&bits, char*buf,
			  unsigned nbuf, bool signed_flag)
{
      unsigned slen = (bits.size() + 3) / 4;
      assert(slen < nbuf);

      buf[slen] = 0;

      unsigned val = 0;
      for (unsigned idx = 0 ;  idx < bits.size() ;  idx += 1) {
	    unsigned vs = (idx%4) * 2;

	    switch (bits.value(idx)) {
		case BIT4_0:
		  break;
		case BIT4_1:
		  val |= 1 << vs;
		  break;
		case BIT4_X:
		  val |= 2 << vs;
		  break;
		case BIT4_Z:
		  val |= 3 << vs;
	    }

	    if (vs == 6) {
		  slen -= 1;
		  buf[slen] = hex_digits[val];
		  val = 0;
	    }
      }

      if (slen > 0) {
	    unsigned padd = 0;

	    slen -= 1;
	    buf[slen] = hex_digits[val];
	    switch(buf[slen]) {
		case 'X': padd = 2; break;
		case 'Z': padd = 3; break;
	    }
	    if (padd) {
		  for (unsigned idx = bits.size() % 4; idx < 4; idx += 1) {
			val = val | padd << 2*idx;
		  }
		  buf[slen] = hex_digits[val];
	    }
      }
}

