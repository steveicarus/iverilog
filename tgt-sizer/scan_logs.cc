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

void scan_logs_gates(ivl_scope_t, ivl_net_logic_t log, struct sizer_statistics&stats)
{
      unsigned wid = ivl_logic_width(log);

      stats.gate_count += wid;
}

void scan_logs(ivl_scope_t scope, struct sizer_statistics&stats)
{
      for (unsigned idx = 0 ; idx < ivl_scope_logs(scope) ; idx += 1) {
	    ivl_net_logic_t log = ivl_scope_log(scope, idx);
	    switch (ivl_logic_type(log)) {
		    // These logic gate types don't really exist in a
		    // mapped design.
		case IVL_LO_BUFZ:
		  break;

		case IVL_LO_AND:
		case IVL_LO_OR:
		case IVL_LO_XOR:
		case IVL_LO_NAND:
		case IVL_LO_NOR:
		case IVL_LO_XNOR:
		case IVL_LO_BUF:
		case IVL_LO_NOT:
		  scan_logs_gates(scope, log, stats);
		  break;
		default:
		  stats.log_bytype[ivl_logic_type(log)] += 1;
		  break;
	    }
      }
}
