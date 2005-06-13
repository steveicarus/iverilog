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
#ident "$Id: vpip_hex.cc,v 1.3 2005/06/13 00:54:04 steve Exp $"
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

void vpip_hex_str_to_bits(unsigned char*bits, unsigned nbits,
			  const char*buf, bool signed_flag)
{
      const char*ebuf = buf + strlen(buf);
      unsigned char last = 0x00;

      while (ebuf > buf) {

	    if (nbits == 0)
		  break;

	    ebuf -= 1;
	    switch (*ebuf) {
		case '0': *bits = 0x00; break;
		case '1': *bits = 0x01; break;
		case '2': *bits = 0x04; break;
		case '3': *bits = 0x05; break;
		case '4': *bits = 0x10; break;
		case '5': *bits = 0x11; break;
		case '6': *bits = 0x14; break;
		case '7': *bits = 0x15; break;
		case '8': *bits = 0x40; break;
		case '9': *bits = 0x41; break;
		case 'a':
		case 'A': *bits = 0x44; break;
		case 'b':
		case 'B': *bits = 0x45; break;
		case 'c':
		case 'C': *bits = 0x50; break;
		case 'd':
		case 'D': *bits = 0x51; break;
		case 'e':
		case 'E': *bits = 0x54; break;
		case 'f':
		case 'F': *bits = 0x55; break;
		case 'x':
		case 'X': *bits = 0xaa; break;
		case 'z':
		case 'Z': *bits = 0xff; break;
		default:  *bits = 0x00; break;
	    }

	    last = *bits;
	    bits += 1;
	    if (nbits < 4)
		  nbits = 0;
	    else
		  nbits -= 4;
      }

	/* Calculate the pad value based on the top bit and the signed
	   flag. We may sign extend or zero extend. */
      switch (last >> 6) {
	  case 0:
	    last = 0x00;
	    break;
	  case 1:
	    last = signed_flag? 0x55 : 0x00;
	    break;
	  case 2:
	    last = 0xaa;
	    break;
	  case 3:
	    last = 0xff;
	    break;
      }

      while (nbits > 0) {
	    *bits = last;
	    bits += 1;
	    if (nbits < 4)
		  nbits = 0;
	    else
		  nbits -= 4;
      }

}

/*
 * $Log: vpip_hex.cc,v $
 * Revision 1.3  2005/06/13 00:54:04  steve
 *  More unified vec4 to hex string functions.
 *
 * Revision 1.2  2002/08/12 01:35:09  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.1  2002/05/11 04:39:35  steve
 *  Set and get memory words by string value.
 *
 */

