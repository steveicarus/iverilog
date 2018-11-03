/*
 * Copyright (c) 2013-2015 Stephen Williams (steve@icarus.com)
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

# include "config.h"
# include "priv.h"
# include <stdlib.h>
# include <inttypes.h>
# include <assert.h>

void show_constant(ivl_net_const_t net)
{
      assert(net);

      fprintf(out, "constant ");
      switch (ivl_const_type(net)) {
	  case IVL_VT_BOOL:
	  case IVL_VT_LOGIC:
	    {
	    const char*bits = ivl_const_bits(net);
	    unsigned idx;
	    assert(bits);
	    for (idx = 0 ; idx < ivl_const_width(net) ; idx += 1)
		  fprintf(out, "%c", bits[idx]);
	    }
	    break;
	  case IVL_VT_REAL:
	    fprintf(out, "%f", ivl_const_real(net));
	    break;
	  default:
	    fprintf(out, "<unsupported type>");
	    break;
      }
      fprintf(out, " at %s:%u\n", ivl_const_file(net), ivl_const_lineno(net));
}
