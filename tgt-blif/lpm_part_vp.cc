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
# include  <cassert>

static int print_part_vp_mux(FILE*fd, ivl_lpm_t net);

/*
 * This implements the IVL_LPM_PART_VP, which is the vector-to-part
 * part select. Implement this as a .names buffer.
 */
int print_lpm_part_vp(FILE*fd, ivl_lpm_t net)
{
      int rc = 0;

	// If this is a non-constant bit select, then handle it
	// elsewhere.
      if (ivl_lpm_data(net,1) != 0)
	    return print_part_vp_mux(fd, net);

      ivl_nexus_t nex_out = ivl_lpm_q(net);
      blif_nex_data_t*ned_out = blif_nex_data_t::get_nex_data(nex_out);

      assert(ivl_lpm_width(net) == ned_out->get_width());
	// Only handle constant part select base.
      assert(ivl_lpm_data(net,1) == 0);

      unsigned bit_sel = ivl_lpm_base(net);

      ivl_nexus_t nex_in = ivl_lpm_data(net,0);
      blif_nex_data_t*ned_in = blif_nex_data_t::get_nex_data(nex_in);

      assert(bit_sel < ned_in->get_width());

      for (unsigned idx = 0 ; idx < ivl_lpm_width(net) ; idx += 1) {
	    fprintf(fd, ".names %s%s %s%s # %s:%u\n1 1\n",
		    ned_in->get_name(), ned_in->get_name_index(bit_sel+idx),
		    ned_out->get_name(), ned_out->get_name_index(idx),
		    ivl_lpm_file(net), ivl_lpm_lineno(net));
      }

      return rc;
}

static int print_part_vp_mux(FILE*fd, ivl_lpm_t net)
{
	// Only handle constant part select base.
      assert(ivl_lpm_data(net,1) != 0);

      ivl_nexus_t nex_out = ivl_lpm_q(net);
      blif_nex_data_t*ned_out = blif_nex_data_t::get_nex_data(nex_out);

	// Only handle bit selects.
      assert(ned_out->get_width() == 1);

      ivl_nexus_t nex_in = ivl_lpm_data(net,0);
      blif_nex_data_t*ned_in = blif_nex_data_t::get_nex_data(nex_in);

      ivl_nexus_t nex_sel = ivl_lpm_data(net,1);
      blif_nex_data_t*ned_sel = blif_nex_data_t::get_nex_data(nex_sel);

      unsigned sel_wid = ned_sel->get_width();
      unsigned in_wid = ned_in->get_width();

      assert((1U<<sel_wid) <= in_wid);

      fprintf(fd, ".names ");
      for (unsigned idx = 0 ; idx < sel_wid ; idx += 1) {
	    fprintf(fd, " %s%s",
		    ned_sel->get_name(), ned_sel->get_name_index(idx));
      }
      for (unsigned idx = 0 ; idx < in_wid ; idx += 1) {
	    fprintf(fd, " %s%s",
		    ned_in->get_name(), ned_in->get_name_index(idx));
      }
      fprintf(fd, " %s%s\n",
	      ned_out->get_name(), ned_out->get_name_index(0));

      for (unsigned idx = 0 ; idx < (1U<<sel_wid) ; idx += 1) {
	    for (unsigned jdx = 0 ; jdx < sel_wid ; jdx += 1)
		  fputc(idx&(1<<jdx)? '1' : '0', fd);

	    for (unsigned jdx = 0 ; jdx < (1U<<sel_wid) ; jdx += 1) {
		  if (jdx==idx)
			fputc('1', fd);
		  else
			fputc('-', fd);
	    }

	    fprintf(fd, " 1\n");
      }

      return 0;
}
