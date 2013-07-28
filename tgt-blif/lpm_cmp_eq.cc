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
 * Implement IVL_LPM_CMP_EQ devices
 */
int print_lpm_cmp_eq(FILE*fd, ivl_lpm_t net)
{
      fprintf(fd, "# %s:%u: IVL_LPM_CMP_EQ: width=%u\n",
	      ivl_lpm_file(net), ivl_lpm_lineno(net), ivl_lpm_width(net));

      ivl_nexus_t q_nex = ivl_lpm_q(net);
      ivl_nexus_t a_nex = ivl_lpm_data(net,0);
      ivl_nexus_t b_nex = ivl_lpm_data(net,1);

      blif_nex_data_t*q_ned = blif_nex_data_t::get_nex_data(q_nex);
      blif_nex_data_t*a_ned = blif_nex_data_t::get_nex_data(a_nex);
      blif_nex_data_t*b_ned = blif_nex_data_t::get_nex_data(b_nex);

      assert(1                  == q_ned->get_width());
      assert(ivl_lpm_width(net) == a_ned->get_width());
      assert(ivl_lpm_width(net) == b_ned->get_width());

      if (ivl_lpm_width(net) == 1) {
	    fprintf(fd, ".names %s%s %s%s %s%s\n"
		    "11 1\n"
		    "00 1\n",
		    a_ned->get_name(), a_ned->get_name_index(0),
		    b_ned->get_name(), b_ned->get_name_index(0),
		    q_ned->get_name(), q_ned->get_name_index(0));
	    return 0;
      }

      for (unsigned idx = 0 ; idx < ivl_lpm_width(net) ; idx += 1) {
	    fprintf(fd, ".names %s%s %s%s %s/%u\n"
		    "11 1\n"
		    "00 1\n",
		    a_ned->get_name(), a_ned->get_name_index(idx),
		    b_ned->get_name(), b_ned->get_name_index(idx),
		    q_ned->get_name(), idx);
      }

      fprintf(fd, ".names");
      for (unsigned idx = 0 ; idx < ivl_lpm_width(net) ; idx += 1)
	    fprintf(fd, " %s/%u", q_ned->get_name(), idx);
      fprintf(fd, " %s\n", q_ned->get_name());

      for (unsigned idx = 0 ; idx < ivl_lpm_width(net) ; idx += 1)
	    fputc('1', fd);
      fprintf(fd, " 1\n");

      return 0;
}

/*
 * Implement IVL_LPM_CMP_NE devices
 */
int print_lpm_cmp_ne(FILE*fd, ivl_lpm_t net)
{
      fprintf(fd, "# %s:%u: IVL_LPM_CMP_NE: width=%u\n",
	      ivl_lpm_file(net), ivl_lpm_lineno(net), ivl_lpm_width(net));

      ivl_nexus_t q_nex = ivl_lpm_q(net);
      ivl_nexus_t a_nex = ivl_lpm_data(net,0);
      ivl_nexus_t b_nex = ivl_lpm_data(net,1);

      blif_nex_data_t*q_ned = blif_nex_data_t::get_nex_data(q_nex);
      blif_nex_data_t*a_ned = blif_nex_data_t::get_nex_data(a_nex);
      blif_nex_data_t*b_ned = blif_nex_data_t::get_nex_data(b_nex);

      assert(1                  == q_ned->get_width());
      assert(ivl_lpm_width(net) == a_ned->get_width());
      assert(ivl_lpm_width(net) == b_ned->get_width());

      if (ivl_lpm_width(net) == 1) {
	    fprintf(fd, ".names %s%s %s%s %s%s\n"
		    "10 1\n"
		    "01 1\n",
		    a_ned->get_name(), a_ned->get_name_index(0),
		    b_ned->get_name(), b_ned->get_name_index(0),
		    q_ned->get_name(), q_ned->get_name_index(0));
	    return 0;
      }

      for (unsigned idx = 0 ; idx < ivl_lpm_width(net) ; idx += 1) {
	    fprintf(fd, ".names %s%s %s%s %s/%u\n"
		    "10 1\n"
		    "01 1\n",
		    a_ned->get_name(), a_ned->get_name_index(idx),
		    b_ned->get_name(), b_ned->get_name_index(idx),
		    q_ned->get_name(), idx);
      }


      fprintf(fd, ".names");
      for (unsigned idx = 0 ; idx < ivl_lpm_width(net) ; idx += 1)
	    fprintf(fd, " %s/%u", q_ned->get_name(), idx);
      fprintf(fd, " %s\n", q_ned->get_name());

      for (unsigned idx = 0 ; idx < ivl_lpm_width(net) ; idx += 1) {
	    for (unsigned jdx = 0 ; jdx < ivl_lpm_width(net) ; jdx += 1) {
		  if (idx==jdx)
			fputc('1', fd);
		  else
			fputc('-', fd);
	    }
	    fprintf(fd, " 1\n");
      }

      return 0;
}
