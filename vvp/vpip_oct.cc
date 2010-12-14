/*
 * Copyright (c) 2002-2010 Stephen Williams (steve@icarus.com)
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

# include  "config.h"
# include  "vpi_priv.h"
# include  <stdio.h>
# include  <string.h>
# include  <limits.h>
# include  <stdlib.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <assert.h>

extern const char oct_digits[64];

void vpip_oct_str_to_bits(unsigned char*bits, unsigned nbits,
			  const char*buf, bool signed_flag)
{
      const char*ebuf = buf + strlen(buf);
      unsigned char last = 0x00;
      unsigned pos = 0;

      for (unsigned idx = 0 ;  idx < (nbits+3)/4 ;  idx += 1)
	    bits[idx] = 0;

      while (ebuf > buf) {
	    unsigned val;

	    if (nbits == 0)
		  break;

	    ebuf -= 1;
	    switch (*ebuf) {
		case '0': val = 0x00; break;
		case '1': val = 0x01; break;
		case '2': val = 0x04; break;
		case '3': val = 0x05; break;
		case '4': val = 0x10; break;
		case '5': val = 0x11; break;
		case '6': val = 0x14; break;
		case '7': val = 0x15; break;
		case 'x':
		case 'X': val = 0x2a; break;
		case 'z':
		case 'Z': val = 0x3f; break;
		default:  val = 0x00; break;
	    }

	    last = val;
	    switch (pos) {
		case 0:
		  bits[0] = val;
		  pos = 3;
		  break;
		case 1:
		  bits[0] |= val << 2;
		  bits += 1;
		  pos = 0;
		  break;
		case 2:
		  bits[0] |= val << 4;
		  if (nbits > 2)
			bits[1] =  val >> 4;
		  bits += 1;
		  pos = 1;
		  break;
		case 3:
		  bits[0] |= val << 6;
		  if (nbits > 1)
			bits[1] =  val >> 2;
		  bits += 1;
		  pos = 2;
	    }

	    if (nbits > 3)
		  nbits -= 3;
	    else
		  nbits = 0;
      }

	/* Calculate the pad value based on the top bit and the signed
	   flag. We may sign extend or zero extend. */
      switch (last >> 4) {
	  case 0:
	    last = 0x00;
	    break;
	  case 1:
	    last = signed_flag? 0x01 : 0x00;
	    break;
	  case 2:
	    last = 0x02;
	    break;
	  case 3:
	    last = 0x03;
	    break;
      }

      while (nbits > 0) switch (pos) {
	  case 0:
	    bits[0] = last;
	    nbits -= 1;
	    pos = 1;
	    break;
	  case 1:
	    bits[0] |= last << 2;
	    nbits -= 1;
	    pos = 2;
	    break;
	  case 2:
	    bits[0] |= last << 4;
	    bits -= 1;
	    pos = 3;
	  case 3:
	    bits[0] |= last << 6;
	    nbits -= 1;
	    bits += 1;
	    pos = 0;
      }

}

void vpip_bits_to_oct_str(const unsigned char*bits, unsigned nbits,
			  char*buf, unsigned nbuf, bool signed_flag)
{
      unsigned slen = (nbits + 2) / 3;
      unsigned val = 0;
      assert(slen < nbuf);

      buf[slen] = 0;

      for (unsigned idx = 0 ;  idx < nbits ;  idx += 1) {
	    unsigned bi = idx/4;
	    unsigned bs = (idx%4) * 2;
	    unsigned bit = (bits[bi] >> bs) & 3;

	    unsigned vs = (idx%3) * 2;
	    val |= bit << vs;

	    if (vs == 4) {
		  slen -= 1;
		  buf[slen] = oct_digits[val];
		  val = 0;
	    }
      }

      if (slen > 0) {
	    slen -= 1;
	    buf[slen] = oct_digits[val];
      }
}
