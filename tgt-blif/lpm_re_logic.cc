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

static bool re_xor(unsigned val)
{
      bool flag = false;
      for (size_t idx = 0 ; idx < 8*sizeof(val) ; idx += 1) {
	    if (val&1) flag ^= true;
	    val >>= 1;
      }
      return flag;
}

int print_lpm_re_logic(FILE*fd, ivl_lpm_t net)
{
      ivl_nexus_t nex_q = ivl_lpm_q(net);
      blif_nex_data_t*ned_q = blif_nex_data_t::get_nex_data(nex_q);

      ivl_nexus_t nex_d = ivl_lpm_data(net,0);
      blif_nex_data_t*ned_d = blif_nex_data_t::get_nex_data(nex_d);

      assert(ned_q->get_width() == 1);

      fprintf(fd, ".names");
      for (size_t idx = 0 ; idx < ned_d->get_width() ; idx += 1) {
	    fprintf(fd, " %s%s", ned_d->get_name(), ned_d->get_name_index(idx));
      }
      fprintf(fd, " %s%s\n", ned_q->get_name(), ned_q->get_name_index(0));

      switch (ivl_lpm_type(net)) {
	  case IVL_LPM_RE_AND:
	    for (size_t idx = 0 ; idx < ned_d->get_width() ; idx += 1)
		  fputc('1', fd);
	    fprintf(fd, " 1\n");
	    break;

	  case IVL_LPM_RE_OR:
	    for (size_t idx = 0 ; idx < ned_d->get_width() ; idx += 1) {
		  for (size_t wid = 0 ; wid < ned_d->get_width() ; wid += 1) {
			if (wid==idx)
			      fputc('1', fd);
			else
			      fputc('-', fd);
		  }
		  fprintf(fd, " 1\n");
	    }
	    break;

	  case IVL_LPM_RE_XOR:
	    assert(ned_d->get_width() < 8*sizeof(unsigned));
	    for (unsigned val = 0; val < (1U<<ned_d->get_width()); val += 1) {
		  if (! re_xor(val))
			continue;

		  for (size_t idx = 0 ; idx < ned_d->get_width() ; idx += 1) {
			if (val & (1<<idx))
			      fputc('1', fd);
			else
			      fputc('0', fd);
		  }
		  fprintf(fd, " 1\n");
	    }
	    break;

	  case IVL_LPM_RE_NAND:
	    for (size_t idx = 0 ; idx < ned_d->get_width() ; idx += 1) {
		  for (size_t wid = 0 ; wid < ned_d->get_width() ; wid += 1) {
			if (wid==idx)
			      fputc('0', fd);
			else
			      fputc('-', fd);
		  }
		  fprintf(fd, " 1\n");
	    }
	    break;

	  case IVL_LPM_RE_NOR:
	    for (size_t idx = 0 ; idx < ned_d->get_width() ; idx += 1)
		  fputc('0', fd);
	    fprintf(fd, " 1\n");
	    break;

	  case IVL_LPM_RE_XNOR:
	    assert(ned_d->get_width() < 8*sizeof(unsigned));
	    for (unsigned val = 0; val < (1U<<ned_d->get_width()); val += 1) {
		  if (re_xor(val))
			continue;

		  for (size_t idx = 0 ; idx < ned_d->get_width() ; idx += 1) {
			if (val & (1<<idx))
			      fputc('1', fd);
			else
			      fputc('0', fd);
		  }
		  fprintf(fd, " 1\n");
	    }
	    break;

	  default:
	    assert(0);
      }

      return 0;
}
