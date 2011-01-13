/*
 * Copyright (C) 2011 Cary R. (cygcary@yahoo.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *
 * This is the vlog95 target module. It generates a 1364-1995 compliant
 * netlist from the input netlist. The generated netlist is expected to
 * be simulation equivalent to the original.
 */

# include <stdlib.h>
# include "config.h"
# include "vlog95_priv.h"

void emit_scaled_delay(ivl_scope_t scope, uint64_t delay)
{
      int scale = ivl_scope_time_units(scope) - sim_precision;
      int pre = ivl_scope_time_units(scope) - ivl_scope_time_precision(scope);
      char *frac;
      uint8_t real_dly = 0;
      assert(scale >= 0);
      assert(pre >= 0);
      assert(scale >= pre);
      frac = (char *)malloc(pre+1);
      frac[pre] = 0;
      for (/* none */; scale > 0; scale -= 1) {
	    if (scale > pre) {
		  assert((delay % 10) == 0);
	    } else {
		  frac[scale-1] = (delay % 10) + '0';
		  if (frac[scale-1] != '0') {
			real_dly = 1;
		  } else if (!real_dly) {
			frac[scale-1] = 0;
		  }
	    }
	    delay /= 10;
      }
      fprintf(vlog_out, "%"PRIu64, delay);
      if (real_dly) {
	    fprintf(vlog_out, ".%s", frac);
      }
      free(frac);
}
