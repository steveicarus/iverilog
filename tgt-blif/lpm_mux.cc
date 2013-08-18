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
# include  <vector>
# include  <cassert>

using namespace std;

/*
 * This handles the special case that the select port to the MUX is a
 * single bit. In other words, a binary mux. This may have an
 * arbitrary data width, but it selects from input A or input B.
 */
static int print_lpm_mux_s1(FILE*fd, ivl_lpm_t net)
{
      ivl_nexus_t nex_out = ivl_lpm_q(net);
      blif_nex_data_t*ned_out = blif_nex_data_t::get_nex_data(nex_out);

      ivl_nexus_t nex_sel = ivl_lpm_select(net);
      blif_nex_data_t*ned_sel = blif_nex_data_t::get_nex_data(nex_sel);

      ivl_nexus_t nex_d0 = ivl_lpm_data(net,0);
      blif_nex_data_t*ned_d0 = blif_nex_data_t::get_nex_data(nex_d0);

      ivl_nexus_t nex_d1 = ivl_lpm_data(net,1);
      blif_nex_data_t*ned_d1 = blif_nex_data_t::get_nex_data(nex_d1);

      fprintf(fd, "# IVL_LPM_MUX ivl_lpm_width(net)=%u, Q=%s, D0=%s, D1=%s\n",
	      ivl_lpm_width(net), ned_out->get_name(), ned_d0->get_name(), ned_d1->get_name());
      assert(ivl_lpm_width(net) == ned_out->get_width());
      assert(ivl_lpm_width(net) == ned_d0->get_width());
      assert(ivl_lpm_width(net) == ned_d1->get_width());

	// Only support single-bit select
      assert(ned_sel->get_width() == 1);

      for (unsigned idx = 0 ; idx < ivl_lpm_width(net) ; idx += 1) {
	    fprintf(fd, ".names %s%s %s%s %s%s %s%s\n"
		        "01- 1\n"
		        "1-1 1\n",
		    ned_sel->get_name(), ned_sel->get_name_index(0),
		    ned_d0->get_name(),  ned_d0->get_name_index(idx),
		    ned_d1->get_name(),  ned_d1->get_name_index(idx),
		    ned_out->get_name(), ned_out->get_name_index(idx));
      }

      return 0;
}

static int print_lpm_mux_sN(FILE*fd, ivl_lpm_t net)
{
      ivl_nexus_t nex_out = ivl_lpm_q(net);
      blif_nex_data_t*ned_out = blif_nex_data_t::get_nex_data(nex_out);

      ivl_nexus_t nex_sel = ivl_lpm_select(net);
      blif_nex_data_t*ned_sel = blif_nex_data_t::get_nex_data(nex_sel);

      vector<blif_nex_data_t*> ned_d (ivl_lpm_size(net));
      for (size_t idx = 0 ; idx < ned_d.size() ; idx += 1) {
	    ivl_nexus_t tmp = ivl_lpm_data(net,idx);
	    ned_d[idx] = blif_nex_data_t::get_nex_data(tmp);
      }

      for (unsigned wid = 0 ; wid < ivl_lpm_width(net) ; wid += 1) {
	      // First, print the names record with all the ports...
	    fprintf(fd, ".names");
	    for (size_t idx = 0 ; idx < ned_sel->get_width() ; idx += 1) {
		  fprintf(fd, " %s%s", ned_sel->get_name(), ned_sel->get_name_index(idx));
	    }

	    for (size_t idx = 0 ; idx < ned_d.size() ; idx += 1) {
		  fprintf(fd, " %s%s", ned_d[idx]->get_name(),
			  ned_d[idx]->get_name_index(wid));
	    }

	    fprintf(fd, " %s%s\n", ned_out->get_name(), ned_out->get_name_index(wid));

	      // Print the logic table. We need one line for each
	      // select address. The select pins must exactly match
	      // the select address. The output depends only on the
	      // select D input.
	    for (size_t didx = 0 ; didx < ned_d.size() ; didx += 1) {

		  for (size_t idx = 0 ; idx < ned_sel->get_width() ; idx += 1) {
			if (didx & (1<<idx))
			      fputc('1', fd);
			else
			      fputc('0', fd);
		  }

		  for (size_t idx = 0 ; idx < ned_d.size() ; idx += 1) {
			if (didx == idx)
			      fputc('1', fd);
			else
			      fputc('-', fd);
		  }

		  fprintf(fd, " 1\n");
	    }
      }

      return 0;
}

int print_lpm_mux(FILE*fd, ivl_lpm_t net)
{
      if (ivl_lpm_selects(net) == 1)
	    return print_lpm_mux_s1(fd, net);
      else
	    return print_lpm_mux_sN(fd, net);
}
