/*
 * Copyright (c) 2001-2014 Stephen Williams (steve@icarus.com)
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

# include  <ivl_target.h>
# include  "fpga_priv.h"
# include  <assert.h>

static void show_cell_scope(ivl_scope_t scope)
{
      if (device->show_cell_scope == 0) {
	    fprintf(stderr, "fpga.tgt: ivl_synthesis_cell(scope)"
		    " not supported by this target.\n");
	    return;
      }

      device->show_cell_scope(scope);
}

static void show_gate_logic(ivl_net_logic_t net)
{
      if (device->show_logic == 0) {
	    fprintf(stderr, "fpga.tgt: IVL LOGIC not supported"
		    " by this target.\n");
	    return;
      }

      assert(device->show_logic);
      device->show_logic(net);
}

static void show_gate_lpm(ivl_lpm_t net)
{
      switch (ivl_lpm_type(net)) {

	  case IVL_LPM_ADD:
	    if (device->show_add == 0) {
		  fprintf(stderr, "fpga.tgt: IVL_LPM_ADD not supported"
			  " by this target.\n");
		  return;
	    }
	    device->show_add(net);
	    break;

	  case IVL_LPM_SUB:
	    if (device->show_sub == 0) {
		  fprintf(stderr, "fpga.tgt: IVL_LPM_SUB not supported"
			  " by this target.\n");
		  return;
	    }
	    device->show_sub(net);
	    break;

	  case IVL_LPM_CMP_EQ:
	    if (device->show_cmp_eq == 0) {
		  fprintf(stderr, "fpga.tgt: IVL_LPM_CMP_EQ not supported"
			  " by this target.\n");
		  return;
	    }
	    device->show_cmp_eq(net);
	    break;

	  case IVL_LPM_CMP_NE:
	    if (device->show_cmp_ne == 0) {
		  fprintf(stderr, "fpga.tgt: IVL_LPM_CMP_NE not supported"
			  " by this target.\n");
		  return;
	    }
	    device->show_cmp_ne(net);
	    break;

	  case IVL_LPM_CMP_GE:
	    if (device->show_cmp_ge == 0) {
		  fprintf(stderr, "fpga.tgt: IVL_LPM_CMP_GE not supported"
			  " by this target.\n");
		  return;
	    }
	    device->show_cmp_ge(net);
	    break;

	  case IVL_LPM_CMP_GT:
	    if (device->show_cmp_gt == 0) {
		  fprintf(stderr, "fpga.tgt: IVL_LPM_CMP_GT not supported"
			  " by this target.\n");
		  return;
	    }
	    device->show_cmp_gt(net);
	    break;

	  case IVL_LPM_FF:
	    if (device->show_dff == 0) {
		  fprintf(stderr, "fpga.tgt: IVL_LPM_FF not supported"
			  " by this target.\n");
		  return;
	    }
	    device->show_dff(net);
	    break;

	  case IVL_LPM_MUX:
	    if (device->show_mux == 0) {
		  fprintf(stderr, "fpga.tgt: IVL_LPM_MUX not supported"
			  " by this target.\n");
		  return;
	    }
	    device->show_mux(net);
	    break;

	  case IVL_LPM_MULT:
	    if (device->show_mult == 0) {
		  fprintf(stderr, "fpga.tgt: IVL_LPM_MULT not supported"
			  " by this target.\n");
		  return;
	    }
	    device->show_mult(net);
	    break;

	  case IVL_LPM_SHIFTL:
	    if (device->show_shiftl == 0) {
		  fprintf(stderr, "fpga.tgt: IVL_LPM_SHIFTL not supported"
			  " by this target.\n");
		  return;
	    }
	    device->show_shiftl(net);
	    break;

	  case IVL_LPM_SHIFTR:
	    if (device->show_shiftr == 0) {
		  fprintf(stderr, "fpga.tgt: IVL_LPM_SHIFTR not supported"
			  " by this target.\n");
		  return;
	    }
	    device->show_shiftr(net);
	    break;

	  default:
	    fprintf(stderr, "fpga.tgt: unknown LPM type %d\n",
		    ivl_lpm_type(net));
	    break;
      }
}

int show_scope_gates(ivl_scope_t net, void*x)
{
      unsigned idx;

      if (scope_has_attribute(net, "ivl_synthesis_cell")) {
	    show_cell_scope(net);
	    return 0;
      }

      for (idx = 0 ;  idx < ivl_scope_logs(net) ;  idx += 1)
	    show_gate_logic(ivl_scope_log(net, idx));

      for (idx = 0 ;  idx < ivl_scope_lpms(net) ;  idx += 1)
	    show_gate_lpm(ivl_scope_lpm(net, idx));

      return ivl_scope_children(net, show_scope_gates, 0);
}
