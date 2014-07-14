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

# include  "vvp_priv.h"
# include  <string.h>
# include  <stdlib.h>
# include  <assert.h>

void draw_lpm_substitute(ivl_lpm_t net)
{
      unsigned swidth = width_of_nexus(ivl_lpm_data(net,1));
      fprintf(vvp_out, "L_%p .substitute %u, %u %u",
	      net, ivl_lpm_width(net), ivl_lpm_base(net), swidth);
      fprintf(vvp_out, ", %s", draw_net_input(ivl_lpm_data(net,0)));
      fprintf(vvp_out, ", %s;\n", draw_net_input(ivl_lpm_data(net,1)));
}
