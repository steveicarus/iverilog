/*
 * Copyright (c) 2002-2013 Stephen Williams (steve@icarus.com)
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

# include  "config.h"
# include  "vpi_priv.h"
# include  <cstdio>
# include  <cstring>
# include  <climits>
# include  <cstdlib>
# include  <cassert>

void vpip_bin_str_to_vec4(vvp_vector4_t&vec4, const char*buf)
{
      const char*ebuf = buf + strlen(buf);
      unsigned idx = 0;
      unsigned skip_chars = 0;
      const char*tbuf = buf;
	/* Find the number of non-numeric characters. */
      while ((tbuf = strpbrk(tbuf, "-_"))) {
	    skip_chars += 1;
	    tbuf += 1;
      }
      vvp_vector4_t tval(strlen(buf)-skip_chars);
      while (ebuf > buf) {
	    ebuf -= 1;
	      /* Skip any "_" characters in the string. */
	    while (*ebuf == '_') {
		  ebuf -= 1;
		  assert(ebuf > buf);
	    }

	      /*  If we find a "-" it must be at the head of the string. */
	    if (*ebuf == '-') {
		  if (ebuf != buf) assert(0);
		  break;
	    }

	    assert(idx < tval.size());
	    switch (*ebuf) {
		case '0':
		  tval.set_bit(idx, BIT4_0);
		  break;
		case '1':
		  tval.set_bit(idx, BIT4_1);
		  break;
		case 'x':
		case 'X':
		  tval.set_bit(idx, BIT4_X);
		  break;
		case 'z':
		case 'Z':
		  tval.set_bit(idx, BIT4_Z);
		  break;
		default:
		    /* Return "x" if there are invalid digits in the string. */
		  fprintf(stderr, "Warning: Invalid binary digit %c(%d) in "
		          "\"%s\".\n", *ebuf, *ebuf, buf);
		  for (unsigned jdx = 0 ;  jdx < vec4.size() ;  jdx += 1) {
			vec4.set_bit(jdx, BIT4_X);
		  }
		  return;
	    }

	    idx += 1;
      }

	/* Make a negative value when needed. */
      if (buf[0] == '-') {
	    tval.invert();
	    tval += (int64_t) 1;
      }

	/* Find the correct padding value. */
      vvp_bit4_t pad;
      switch (tval.value(tval.size()-1)) {
	  case BIT4_X:  // Pad MSB 'x' with 'x'
	    pad = BIT4_X;
	    break;
	  case BIT4_Z:  // Pad MSB 'z' with 'z'
	    pad = BIT4_Z;
	    break;
	  case BIT4_1:  // If negative pad MSB '1' with '1'
	    if (buf[0] == '-') {
		  pad = BIT4_1;
		  break;
	    }
	    // fallthrough
	  default:  // Everything else gets '0' padded/
	    pad = BIT4_0;
	    break;
      }

      for (unsigned jdx = 0 ;  jdx < vec4.size() ; jdx += 1) {
	    if (jdx < tval.size()) vec4.set_bit(jdx, tval.value(jdx));
	    else vec4.set_bit(jdx, pad);
      }
}
