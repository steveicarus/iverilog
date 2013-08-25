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

using namespace std;

int print_lpm_ff(FILE*fd, ivl_lpm_t net)
{
      int errors = 0;

      ivl_nexus_t nex_q = ivl_lpm_q(net);
      blif_nex_data_t*ned_q = blif_nex_data_t::get_nex_data(nex_q);

      ivl_nexus_t nex_d = ivl_lpm_data(net,0);
      blif_nex_data_t*ned_d = blif_nex_data_t::get_nex_data(nex_d);

      ivl_nexus_t nex_c = ivl_lpm_clk(net);
      blif_nex_data_t*ned_c = blif_nex_data_t::get_nex_data(nex_c);

      if (ivl_lpm_enable(net)) {
	    errors += 1;
	    fprintf(stderr, "%s:%u: sorry: blif: Clock-Enable not implemented yet.\n",
		    ivl_lpm_file(net), ivl_lpm_lineno(net));
      }
      if (ivl_lpm_async_clr(net)) {
	    errors += 1;
	    fprintf(stderr, "%s:%u: sorry: blif: Asynchronous clear not implemented yet\n",
		    ivl_lpm_file(net), ivl_lpm_lineno(net));
      }

      if (ivl_lpm_async_set(net)) {
	    errors += 1;
	    fprintf(stderr, "%s:%u: sorry: blif: Asynchronous set not implemented yet\n",
		    ivl_lpm_file(net), ivl_lpm_lineno(net));
      }

      fprintf(fd, "# IVL_LPM_FF: width=%u, Q=%s, D=%s, C=%s\n",
	      ivl_lpm_width(net), ned_q->get_name(), ned_d->get_name(), ned_c->get_name());

      for (unsigned wid = 0 ; wid < ivl_lpm_width(net) ; wid += 1) {
	    fprintf(fd, ".latch %s%s %s%s re %s%s 3\n",
		    ned_d->get_name(), ned_d->get_name_index(wid),
		    ned_q->get_name(), ned_q->get_name_index(wid),
		    ned_c->get_name(), ned_c->get_name_index(0));
      }

      return errors;
}
