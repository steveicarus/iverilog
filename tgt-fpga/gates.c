/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#ident "$Id: gates.c,v 1.6 2001/09/02 21:33:07 steve Exp $"

# include  <ivl_target.h>
# include  "fpga_priv.h"
# include  <assert.h>

static void show_gate_logic(ivl_net_logic_t net)
{
      device->show_logic(net);
}

static void show_gate_lpm(ivl_lpm_t net)
{
      switch (ivl_lpm_type(net)) {

	  case IVL_LPM_ADD:
	    device->show_add(net);
	    break;

	  case IVL_LPM_CMP_EQ:
	    device->show_cmp_eq(net);
	    break;

	  case IVL_LPM_CMP_NE:
	    device->show_cmp_ne(net);
	    break;

	  case IVL_LPM_FF:
	    device->show_dff(net);
	    break;

	  case IVL_LPM_MUX:
	    device->show_mux(net);
	    break;

	  default:
	    fprintf(stderr, "fpga.tgt: unknown LPM type %u\n",
		    ivl_lpm_type(net));
	    break;
      }
}

int show_scope_gates(ivl_scope_t net, void*x)
{
      unsigned idx;

      for (idx = 0 ;  idx < ivl_scope_logs(net) ;  idx += 1)
	    show_gate_logic(ivl_scope_log(net, idx));

      for (idx = 0 ;  idx < ivl_scope_lpms(net) ;  idx += 1)
	    show_gate_lpm(ivl_scope_lpm(net, idx));

      return ivl_scope_children(net, show_scope_gates, 0);
}

/*
 * $Log: gates.c,v $
 * Revision 1.6  2001/09/02 21:33:07  steve
 *  Rearrange the XNF code generator to be generic-xnf
 *  so that non-XNF code generation is also possible.
 *
 *  Start into the virtex EDIF output driver.
 *
 * Revision 1.5  2001/09/01 04:30:44  steve
 *  Generic ADD code.
 *
 * Revision 1.4  2001/09/01 02:28:42  steve
 *  Generate code for MUX devices.
 *
 * Revision 1.3  2001/09/01 02:01:30  steve
 *  identity compare, and PWR records for constants.
 *
 * Revision 1.2  2001/08/30 04:31:04  steve
 *  Mangle nexus names.
 *
 * Revision 1.1  2001/08/28 04:14:20  steve
 *  Add the fpga target.
 *
 */

