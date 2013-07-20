/*
 * Copyright (c) 2013 Stephen Williams (steve@icarus.com)
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

# include  "priv.h"
# include  "nex_data.h"
# include  <cstdio>

static void print_constant(FILE*fd, ivl_net_const_t net)
{
      unsigned wid = ivl_const_width(net);
      const char*val = ivl_const_bits(net);
      ivl_nexus_t nex = ivl_const_nex(net);
      blif_nex_data_t*ned = blif_nex_data_t::get_nex_data(nex);

      for (unsigned idx = 0 ; idx < wid ; idx += 1) {
	    switch (val[idx]) {
		case '1':
		  fprintf(fd, ".names %s%s # const 1\n1\n",
			  ned->get_name(), ned->get_name_index(idx));
		  break;
		case '0':
		  fprintf(fd, ".names %s%s # const 0\n",
			  ned->get_name(), ned->get_name_index(idx));
		  break;
		default:
		  fprintf(fd, ".names %s%s # const %c\n",
			  ned->get_name(), ned->get_name_index(idx), val[idx]);
		  break;
	    }
      }
}

void emit_constants(FILE*fd, ivl_design_t des, ivl_scope_t model)
{
      for (unsigned idx = 0 ; idx < ivl_design_consts(des) ; idx += 1) {
	    ivl_net_const_t net = ivl_design_const(des, idx);
	    if (! scope_is_in_model(model, ivl_const_scope(net)))
		  continue;

	    print_constant(fd, net);
      }
}
