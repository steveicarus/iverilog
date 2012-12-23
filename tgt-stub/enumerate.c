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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "config.h"
# include  "priv.h"
# include  <string.h>

void show_enumerate(ivl_enumtype_t net)
{
      unsigned idx;
      fprintf(out, "  enumeration %p {\n", net);

      for (idx = 0 ; idx < ivl_enum_names(net) ; idx += 1) {
	    fprintf(out, "      %s = <", ivl_enum_name(net, idx));

	    const char*bits = ivl_enum_bits(net, idx);
	    size_t bits_len = strlen(bits);
	    size_t bit;
	    for (bit = bits_len ; bit > 0 ; bit -= 1)
		  fputc(bits[bit-1], out);

	    fprintf(out, ">\n");
      }

      fprintf(out, "  }\n");
}
