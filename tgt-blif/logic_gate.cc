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

static int do_print_logic_gate(FILE*fd, ivl_net_logic_t net, unsigned bit);

int print_logic_gate(FILE*fd, ivl_net_logic_t net)
{
      int rc = 0;
      for (unsigned idx = 0 ; idx < ivl_logic_width(net) ; idx += 1) {
	    rc += do_print_logic_gate(fd, net, idx);
	    if (rc != 0)
		  break;
      }

      return rc;
}

static int do_print_logic_gate(FILE*fd, ivl_net_logic_t net, unsigned bit)
{
      int rc = 0;

      ivl_nexus_t nex_out = ivl_logic_pin(net,0);
      blif_nex_data_t*ned_out = blif_nex_data_t::get_nex_data(nex_out);

      assert(ned_out->get_width() > bit);

      fprintf(fd, ".names");
      for (unsigned idx = 1 ; idx < ivl_logic_pins(net) ; idx += 1) {
	    ivl_nexus_t nex = ivl_logic_pin(net,idx);
	    blif_nex_data_t*ned = blif_nex_data_t::get_nex_data(nex);
	    fprintf(fd, " %s%s", ned->get_name(), ned->get_name_index(bit));
      }

      fprintf(fd, " %s%s", ned_out->get_name(), ned_out->get_name_index(bit));
      fprintf(fd, "\n");

      switch (ivl_logic_type(net)) {
	  case IVL_LO_AND:
	    for (unsigned idx = 1 ; idx < ivl_logic_pins(net) ; idx += 1)
		  fprintf(fd, "1");
	    fprintf(fd, " 1\n");
	    break;
	  case IVL_LO_OR:
	    assert(ivl_logic_pins(net)==3);
	    fprintf(fd, "1- 1\n");
	    fprintf(fd, "-1 1\n");
	    break;
	  case IVL_LO_XOR:
	    assert(ivl_logic_pins(net)==3);
	    fprintf(fd, "10 1\n");
	    fprintf(fd, "01 1\n");
	    break;

	  case IVL_LO_NAND:
	    assert(ivl_logic_pins(net)==3);
	    fprintf(fd, "0- 1\n");
	    fprintf(fd, "-0 1\n");
	    break;
	  case IVL_LO_NOR:
	    for (unsigned idx = 1 ; idx < ivl_logic_pins(net) ; idx += 1)
		  fprintf(fd, "0");
	    fprintf(fd, " 1\n");
	    break;
	  case IVL_LO_XNOR:
	    assert(ivl_logic_pins(net)==3);
	    fprintf(fd, "00 1\n");
	    fprintf(fd, "11 1\n");
	    break;

	  case IVL_LO_BUF:
	    assert(ivl_logic_pins(net)==2);
	    fprintf(fd, "1 1\n");
	    break;
	  case IVL_LO_NOT:
	    assert(ivl_logic_pins(net)==2);
	    fprintf(fd, "0 1\n");
	    break;

	  case IVL_LO_PULLDOWN:
	    assert(ivl_logic_pins(net)==1);
	    fprintf(fd, "0\n");
	    break;
	  case IVL_LO_PULLUP:
	    assert(ivl_logic_pins(net)==1);
	    fprintf(fd, "1\n");
	    break;

	  case IVL_LO_BUFZ:
	    assert(ivl_logic_pins(net)==2);
	    fprintf(fd, "1 1\n");
	    break;

	  default:
	    fprintf(fd, "# ERROR: Logic type %d not handled\n", ivl_logic_type(net));
	    rc += 1;
	    break;
      }

      return rc;
}
