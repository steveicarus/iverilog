/*
 * Copyright (c) 2002-2006 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vpip_bin.cc,v 1.4 2006/08/03 05:05:06 steve Exp $"
#endif

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

void vpip_bin_str_to_vec4(vvp_vector4_t&vec4,
			  const char*buf, bool signed_flag)
{
      const char*ebuf = buf + strlen(buf);
      vvp_bit4_t last = BIT4_0;

      unsigned idx = 0;
      while (ebuf > buf) {
	    vvp_bit4_t val;

	    if (idx == vec4.size())
		  break;

	    ebuf -= 1;
	    switch (*ebuf) {
		case '0': val = BIT4_0; break;
		case '1': val = BIT4_1; break;
		case 'x':
		case 'X': val = BIT4_X; break;
		case 'z':
		case 'Z': val = BIT4_Z; break;
		default:  val = BIT4_0; break;
	    }

	    last = val;
	    vec4.set_bit(idx, val);
	    idx += 1;
      }

	/* Calculate the pad value based on the top bit and the signed
	   flag. We may sign extend or zero extend. */
      switch (last) {
	  case BIT4_0:
	    last = BIT4_0;
	    break;
	  case BIT4_1:
	    last = signed_flag? BIT4_1 : BIT4_0;
	    break;
	  case BIT4_X:
	    last = BIT4_X;
	    break;
	  case BIT4_Z:
	    last = BIT4_Z;
	    break;
      }

      while (idx < vec4.size())
	    vec4.set_bit(idx++, last);
}

/*
 * $Log: vpip_bin.cc,v $
 * Revision 1.4  2006/08/03 05:05:06  steve
 *  Fix infinite loop padding binary string to result.
 *
 * Revision 1.3  2006/02/21 05:31:54  steve
 *  Put strings for reg objects.
 *
 * Revision 1.2  2002/08/12 01:35:09  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.1  2002/05/11 04:39:35  steve
 *  Set and get memory words by string value.
 *
 */

