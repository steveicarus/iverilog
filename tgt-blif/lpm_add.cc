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
 *  assign Q = A ^ B ^ Cin;
 *  assign Cout = A&B | A&Cin | B&Cin;
 */
int print_lpm_add(FILE*fd, ivl_lpm_t net)
{
      fprintf(fd, "# %s:%u: IVL_LPM_ADD: width=%u\n",
	      ivl_lpm_file(net), ivl_lpm_lineno(net), ivl_lpm_width(net));

      ivl_nexus_t q_nex = ivl_lpm_q(net);
      ivl_nexus_t a_nex = ivl_lpm_data(net,0);
      ivl_nexus_t b_nex = ivl_lpm_data(net,1);

      blif_nex_data_t*q_ned = blif_nex_data_t::get_nex_data(q_nex);
      blif_nex_data_t*a_ned = blif_nex_data_t::get_nex_data(a_nex);
      blif_nex_data_t*b_ned = blif_nex_data_t::get_nex_data(b_nex);

      assert(ivl_lpm_width(net) == q_ned->get_width());
      assert(ivl_lpm_width(net) == a_ned->get_width());
      assert(ivl_lpm_width(net) == b_ned->get_width());

	// Q[0] = A[0] ^ B[0]
      fprintf(fd, ".names %s%s %s%s %s%s\n"
	      "10 1\n"
	      "01 1\n",
	      a_ned->get_name(), a_ned->get_name_index(0),
	      b_ned->get_name(), b_ned->get_name_index(0),
	      q_ned->get_name(), q_ned->get_name_index(0));

	// Special case: If this is a 1-bit adder, then all we need is
	// the XOR above. We're done.
      if (ivl_lpm_width(net) == 1)
	    return 0;

	// Cout[0] = A[0] & B[0]
      fprintf(fd, ".names %s%s %s%s %s%s/cout\n"
	      "11 1\n",
	      a_ned->get_name(), a_ned->get_name_index(0),
	      b_ned->get_name(), b_ned->get_name_index(0),
	      q_ned->get_name(), q_ned->get_name_index(0));

      for (unsigned idx = 1 ; idx < ivl_lpm_width(net) ; idx += 1) {
	    char cin[128];
	    snprintf(cin, sizeof cin, "%s%s/cout",
		     q_ned->get_name(), q_ned->get_name_index(idx-1));

	      // Q[idx] = A[idx] ^ B[idx] ^ cin
	    fprintf(fd, ".names %s%s %s%s %s %s%s\n"
		    "001 1\n"
		    "010 1\n"
		    "100 1\n"
		    "111 1\n",
		    a_ned->get_name(), a_ned->get_name_index(idx),
		    b_ned->get_name(), b_ned->get_name_index(idx),
		    cin,
		    q_ned->get_name(), q_ned->get_name_index(idx));

	    if ((idx+1) == ivl_lpm_width(net))
		  continue;

	      // Cout[idx] = A[idx]&B[idx] | A[idx]&Cin | B[idx]&Cin
	    fprintf(fd, ".names %s%s %s%s %s %s%s/cout\n"
		    "011 1\n"
		    "101 1\n"
		    "11- 1\n",
		    a_ned->get_name(), a_ned->get_name_index(idx),
		    b_ned->get_name(), b_ned->get_name_index(idx),
		    cin,
		    q_ned->get_name(), q_ned->get_name_index(idx));
      }

      return 0;
}

/*
 *  Cin[0] = 1;
 *  Cin[n: n!=0] = Cout[n-1]
 *  assign Q[n] = A[n] ^ (~B[n]) ^ Cin[n];
 *  assign Cout[n] = A[n]&~B[n] | A[n]&Cin[n] | (~B[n])&Cin[n];
 */
int print_lpm_sub(FILE*fd, ivl_lpm_t net)
{
      fprintf(fd, "# %s:%u: IVL_LPM_SUB: width=%u\n",
	      ivl_lpm_file(net), ivl_lpm_lineno(net), ivl_lpm_width(net));

      ivl_nexus_t q_nex = ivl_lpm_q(net);
      ivl_nexus_t a_nex = ivl_lpm_data(net,0);
      ivl_nexus_t b_nex = ivl_lpm_data(net,1);

      blif_nex_data_t*q_ned = blif_nex_data_t::get_nex_data(q_nex);
      blif_nex_data_t*a_ned = blif_nex_data_t::get_nex_data(a_nex);
      blif_nex_data_t*b_ned = blif_nex_data_t::get_nex_data(b_nex);

      assert(ivl_lpm_width(net) == q_ned->get_width());
      assert(ivl_lpm_width(net) == a_ned->get_width());
      assert(ivl_lpm_width(net) == b_ned->get_width());

	// Q[0] = A[0] ^ ~B[0] ^ 1
      fprintf(fd, ".names %s%s %s%s %s%s\n"
	      "01 1\n"
	      "10 1\n",
	      a_ned->get_name(), a_ned->get_name_index(0),
	      b_ned->get_name(), b_ned->get_name_index(0),
	      q_ned->get_name(), q_ned->get_name_index(0));

      if (ivl_lpm_width(net) == 1)
	    return 0;

	// Cout[0] = A[0] & ~B[0] | A[0]&1 | ~B[0]&1
	//         = A[0] & ~B[0] | A[0] | ~B[0]
	//         = A[0] | ~B[0]
      fprintf(fd, ".names %s%s %s%s %s%s/cout\n"
	      "1- 1\n"
	      "-0 1\n",
	      a_ned->get_name(), a_ned->get_name_index(0),
	      b_ned->get_name(), b_ned->get_name_index(0),
	      q_ned->get_name(), q_ned->get_name_index(0));

      for (unsigned idx = 1 ; idx < ivl_lpm_width(net) ; idx += 1) {
	    char cin[128];
	    snprintf(cin, sizeof cin, "%s%s/cout",
		     q_ned->get_name(), q_ned->get_name_index(idx-1));

	      // Q[idx] = A[idx] ^ ~B[idx] ^ cin
	    fprintf(fd, ".names %s%s %s%s %s %s%s\n"
		    "011 1\n"
		    "000 1\n"
		    "110 1\n"
		    "101 1\n",
		    a_ned->get_name(), a_ned->get_name_index(idx),
		    b_ned->get_name(), b_ned->get_name_index(idx),
		    cin,
		    q_ned->get_name(), q_ned->get_name_index(idx));

	    if ((idx+1) == ivl_lpm_width(net))
		  continue;

	      // Cout[idx] = A[idx]&~B[idx] | A[idx]&Cin | ~B[idx]&Cin
	    fprintf(fd, ".names %s%s %s%s %s %s%s/cout\n"
		    "001 1\n"
		    "111 1\n"
		    "10- 1\n",
		    a_ned->get_name(), a_ned->get_name_index(idx),
		    b_ned->get_name(), b_ned->get_name_index(idx),
		    cin,
		    q_ned->get_name(), q_ned->get_name_index(idx));
      }

      return 0;
}
