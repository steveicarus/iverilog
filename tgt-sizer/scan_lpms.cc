/*
 * Copyright (c) 2014 Stephen Williams (steve@icarus.com)
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

# include  "sizer_priv.h"

static void scan_lpms_ff(ivl_scope_t scope, ivl_lpm_t lpm, struct sizer_statistics&stats)
{
      ivl_nexus_t out = ivl_lpm_q(lpm);
      unsigned wid = get_nexus_width(out);

      stats.flop_count += wid;
}

void scan_lpms(ivl_scope_t scope, struct sizer_statistics&stats)
{
      for (unsigned idx = 0 ; idx < ivl_scope_lpms(scope) ; idx += 1) {
	    ivl_lpm_t lpm = ivl_scope_lpm(scope,idx);
	    switch (ivl_lpm_type(lpm)) {
		case IVL_LPM_FF:
		  scan_lpms_ff(scope, lpm, stats);
		  break;
		default:
		  stats.lpm_unknown += 1;
		  break;
	    }
      }
}
