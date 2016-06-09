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
 * Implement IVL_LPM_SHIFT devices (via standard layered barrel shifter design)
 */
int print_lpm_shift(FILE*fd, ivl_lpm_t net, bool left)
{
      fprintf(fd, "# %s:%u: IVL_LPM_SHIFT%c: width=%u\n",
	      ivl_lpm_file(net), ivl_lpm_lineno(net), left ? 'L' : 'R',
	      ivl_lpm_width(net));

      ivl_nexus_t q_nex = ivl_lpm_q(net);
      ivl_nexus_t d_nex = ivl_lpm_data(net,0);
      ivl_nexus_t s_nex = ivl_lpm_data(net,1);

      blif_nex_data_t*q_ned = blif_nex_data_t::get_nex_data(q_nex);
      blif_nex_data_t*d_ned = blif_nex_data_t::get_nex_data(d_nex);
      blif_nex_data_t*s_ned = blif_nex_data_t::get_nex_data(s_nex);

      unsigned dataw = ivl_lpm_width(net);
      size_t shiftw = s_ned->get_width();
      bool signed_ = ivl_lpm_signed(net);

      assert(dataw == q_ned->get_width());
      assert(dataw == d_ned->get_width());

      // TODO: output width can be larger than data

      // TODO: optimizations:
      // * too large shift widths (more than data size)
      // * shift width of 1
      // * data width of 1

      for (size_t lvl = 0 ; lvl < shiftw ; lvl += 1) {
	    for (unsigned idx = 0 ; idx < dataw ; idx += 1) {
		  unsigned idx_2 = left ? idx - (1 << lvl) : idx + (1 << lvl);
		  bool borrow = idx_2 >= dataw;
		  bool signed_borrow = borrow && !left && signed_;

		  // First arg (shift)
		  fprintf(fd, ".names %s%s",
			  s_ned->get_name(), s_ned->get_name_index(lvl));

		  // Multiplexed bits
		  if (!borrow) {
			if (lvl == 0) {
			      fprintf(fd, " %s%s %s%s",
				      d_ned->get_name(), d_ned->get_name_index(idx),
				      d_ned->get_name(), d_ned->get_name_index(idx_2));
			} else {
			      fprintf(fd, " %s/%zu/%u %s/%zu/%u",
				      q_ned->get_name(), lvl - 1, idx,
				      q_ned->get_name(), lvl - 1, idx_2);
			}
		  } else if (signed_borrow) {
			if (lvl == 0) {
			      fprintf(fd, " %s%s %s%s",
				      d_ned->get_name(), d_ned->get_name_index(idx),
				      d_ned->get_name(), d_ned->get_name_index(dataw - 1));
			} else {
			      fprintf(fd, " %s/%zu/%u %s%s",
				      q_ned->get_name(), lvl - 1, idx,
				      d_ned->get_name(), d_ned->get_name_index(dataw - 1));
			}
		  } else {
			if (lvl == 0) {
			      fprintf(fd, " %s%s",
				      d_ned->get_name(), d_ned->get_name_index(idx));
			} else {
			      fprintf(fd, " %s/%zu/%u",
				      q_ned->get_name(), lvl - 1, idx);
			}
		  }

		  // Output
		  if (lvl == shiftw - 1) {
			fprintf(fd, " %s%s\n",
				q_ned->get_name(), q_ned->get_name_index(idx));
		  } else {
			fprintf(fd, " %s/%zu/%u\n",
				q_ned->get_name(), lvl, idx);
		  }

		  if (!borrow || signed_borrow)
			fputs("1-1 1\n"
			      "01- 1\n",
			      fd);
		  else
			fputs("01 1\n", fd);
	    }
      }

      return 0;
}

