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
# include  <limits.h>     /* for CHAR_BIT */
# include  <stdlib.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <assert.h>

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
