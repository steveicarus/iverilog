/*
 * Copyright (c) 2016 Yury Gribov (tetra2005@gmail.com)
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
# include  <cassert>

/*
 * Implement IVL_LPM_SIGN_EXT devices
 */
int print_lpm_sign_ext(FILE*fd, ivl_lpm_t net)
{
      fprintf(fd, "# %s:%u: IVL_LPM_SIGN_EXT: width=%u\n",
	      ivl_lpm_file(net), ivl_lpm_lineno(net), ivl_lpm_width(net));

      ivl_nexus_t q_nex = ivl_lpm_q(net);
      ivl_nexus_t d_nex = ivl_lpm_data(net,0);

      blif_nex_data_t*q_ned = blif_nex_data_t::get_nex_data(q_nex);
      blif_nex_data_t*d_ned = blif_nex_data_t::get_nex_data(d_nex);

      unsigned inw = d_ned->get_width();
      unsigned outw = ivl_lpm_width(net);

//printf("Shift: LPM width = %u, output width = %zd, input width = %u\n", outw, q_ned->get_width(), inw);

      assert(outw == q_ned->get_width());
      assert(inw < outw);

      for (unsigned idx = 0 ; idx < outw ; idx += 1) {
        unsigned idx_in = idx < inw ? idx : inw - 1;

        fprintf(fd, ".names %s%s %s%s\n",
                d_ned->get_name(), d_ned->get_name_index(idx_in),
                q_ned->get_name(), q_ned->get_name_index(idx));
        fprintf(fd, "1 1\n");
      }

      return 0;
}

