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

extern const char oct_digits[64];

void vpip_oct_str_to_vec4(vvp_vector4_t&val, const char*str)
{
      unsigned str_len = strlen(str);
      unsigned skip_chars = 0;
      const char*tstr = str;
	/* Find the number of non-numeric characters. */
      while ((tstr = strpbrk(tstr, "-_"))) {
	    skip_chars += 1;
	    tstr += 1;
      }
      vvp_vector4_t tval(3*(str_len-skip_chars));
      skip_chars = 0;
      for (unsigned idx = 0 ;  idx < tval.size() ;  idx += 1) {
	    unsigned tmp;
	    unsigned bit_off = idx%3;
	    unsigned str_off = idx/3;
	    char ch;

	    assert(str_off+skip_chars < str_len);
	      /* Skip any "_" characters in the string. */
	    while ((ch = str[str_len-str_off-1-skip_chars]) == '_') {
		  skip_chars += 1;
		  assert(str_off+skip_chars < str_len);
	    }

	      /* If we find a "-" it must be at the head of the string. */
	    if (ch == '-') assert(0);

	    switch (ch) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		  tmp = ch - '0';
		  tval.set_bit(idx, ((tmp>>bit_off)&1)? BIT4_1 : BIT4_0);
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
		  fprintf(stderr, "Warning: Invalid octal digit %c(%d) in "
		          "\"%s\".\n", ch, ch, str);
		  for (unsigned jdx = 0 ;  jdx < val.size() ;  jdx += 1) {
			val.set_bit(jdx, BIT4_X);
		  }
		  return;
	    }
      }

	/* Make a negative value when needed. */
      if (str[0] == '-') {
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
	    if (str[0] == '-') {
		  pad = BIT4_1;
		  break;
	    }
	    // fallthrough
	  default:  // Everything else gets '0' padded.
	    pad = BIT4_0;
	    break;
      }

	/* Copy the temporary value to the real value, padding if needed. */
      for (unsigned idx = 0 ;  idx < val.size() ;  idx += 1) {
	    if (idx < tval.size()) val.set_bit(idx, tval.value(idx));
	    else val.set_bit(idx, pad);
      }
}

void vpip_vec4_to_oct_str(const vvp_vector4_t&bits, char*buf, unsigned nbuf)
{
      unsigned slen = (bits.size() + 2) / 3;
      assert(slen < nbuf);

      buf[slen] = 0;

      unsigned val = 0;
      for (unsigned idx = 0 ;  idx < bits.size() ;  idx += 1) {
	    unsigned vs = (idx%3) * 2;

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

	    if (vs == 4) {
		  slen -= 1;
		  buf[slen] = oct_digits[val];
		  val = 0;
	    }
      }

	/* Fill in X or Z if they are the only thing in the value. */
      switch (bits.size() % 3) {
	  case 1:
	    if (val == 2) val = 42;
	    else if (val == 3) val = 63;
	    break;
	  case 2:
	    if (val == 10) val = 42;
	    else if (val == 15) val = 63;
	    break;
      }

      if (slen > 0) {
	    slen -= 1;
	    buf[slen] = oct_digits[val];
      }
}
