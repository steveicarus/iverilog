/*
 * Copyright (c) 2010 Stephen Williams (steve@icarus.com)
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

# include  "vvp_priv.h"
# include  "ivl_alloc.h"
# include  <stdlib.h>
# include  <assert.h>

void draw_enumeration_in_scope(ivl_enumtype_t enumtype)
{
      unsigned idx;

      fprintf(vvp_out, "enum%p .enum\n", enumtype);

      for (idx = 0 ; idx < ivl_enum_names(enumtype) ; idx += 1) {
	    const char*comma = idx+1 < ivl_enum_names(enumtype)? "," : "";

	    fprintf(vvp_out, "   \"%s\"", ivl_enum_name(enumtype, idx));

	    long val = 0;
	    long mask = 1;
	    const char*bits = ivl_enum_bits(enumtype, idx);
	    const char*bit;
	    for (bit = bits, mask = 1 ; bit[0] != 0 ; bit += 1, mask <<= 1) {
		  if (*bit == '1')
			val |= mask;
	    }

	    fprintf(vvp_out, " %ld%s\n", val, comma);
      }

      fprintf(vvp_out, " ;\n");
}
