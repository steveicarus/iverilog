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

/*
 * Implement IVL_LPM_CONCAT devices by creating a .names buffer for
 * each bit of the output. Connect the output bit to the input bit. So
 * for example:
 *    Q = {A, B, C}
 * becomes:
 *    .names C Q[0]
 *    1 1
 *    .names B Q[1]
 *    1 1
 *    .names A Q[2]
 *    1 1
 * (In this example, A, B, and C are 1 bit.)
 */
static int print_concat(FILE*fd, ivl_lpm_t net)
{
      fprintf(fd, "# %s:%u: IVL_LPM_CONCAT: width=%u\n",
	      ivl_lpm_file(net), ivl_lpm_lineno(net), ivl_lpm_width(net));

      ivl_nexus_t nex_q = ivl_lpm_q(net);
      blif_nex_data_t*ned_q = blif_nex_data_t::get_nex_data(nex_q);

      ivl_nexus_t nex_d = ivl_lpm_data(net,0);
      blif_nex_data_t*ned_d = blif_nex_data_t::get_nex_data(nex_d);
      unsigned didx = 0;
      unsigned dpos = 0;
      for (unsigned wid = 0 ; wid < ivl_lpm_width(net) ; wid += 1) {
	    if (dpos >= ned_d->get_width()) {
		  didx += 1;
		  dpos = 0;
		  nex_d = ivl_lpm_data(net,didx);
		  ned_d = blif_nex_data_t::get_nex_data(nex_d);
	    }

	    fprintf(fd, ".names %s%s %s%s\n1 1\n",
		    ned_d->get_name(), ned_d->get_name_index(dpos),
		    ned_q->get_name(), ned_q->get_name_index(wid));
	    dpos += 1;
      }

      return 0;
}

int print_lpm(FILE*fd, ivl_lpm_t net)
{
      int rc = 0;
      ivl_lpm_type_t type = ivl_lpm_type(net);

      switch (type) {
	  case IVL_LPM_ADD:
	    rc += print_lpm_add(fd, net);
	    break;
	  case IVL_LPM_CMP_EQ:
	  case IVL_LPM_CMP_EEQ:
	    rc += print_lpm_cmp_eq(fd, net);
	    break;
	  case IVL_LPM_CMP_GE:
	  case IVL_LPM_CMP_GT:
	    rc += print_lpm_cmp_gt(fd, net);
	    break;
	  case IVL_LPM_CONCAT:
	  case IVL_LPM_CONCATZ:
	    rc += print_concat(fd, net);
	    break;
	  case IVL_LPM_CMP_NE:
	  case IVL_LPM_CMP_NEE:
	    rc += print_lpm_cmp_ne(fd, net);
	    break;
	  case IVL_LPM_FF:
	    rc += print_lpm_ff(fd, net);
	    break;
	  case IVL_LPM_MUX:
	    rc += print_lpm_mux(fd, net);
	    break;
	  case IVL_LPM_PART_VP:
	    rc += print_lpm_part_vp(fd, net);
	    break;
	  case IVL_LPM_RE_AND:
	  case IVL_LPM_RE_OR:
	  case IVL_LPM_RE_XOR:
	  case IVL_LPM_RE_NAND:
	  case IVL_LPM_RE_NOR:
	  case IVL_LPM_RE_XNOR:
	    rc += print_lpm_re_logic(fd, net);
	    break;
	  case IVL_LPM_SUB:
	    rc += print_lpm_sub(fd, net);
	    break;
	  case IVL_LPM_SHIFTL:
	  case IVL_LPM_SHIFTR:
	    rc += print_lpm_shift(fd, net, type == IVL_LPM_SHIFTL);
	    break;
	  case IVL_LPM_SIGN_EXT:
	    rc += print_lpm_sign_ext(fd, net);
	    break;
	  default:
	    fprintf(fd, "# XXXX ivl_lpm_type(net) --> %d\n", ivl_lpm_type(net));
	    fprintf(stderr, "%s:%u: sorry: ivl_lpm_type(net)==%d not implemented.\n",
		    ivl_lpm_file(net), ivl_lpm_lineno(net), ivl_lpm_type(net));
	    rc += 1;
	    break;
      }

      return rc;
}
