/*
 * Copyright (c) 2005-2010 Stephen Williams
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

# include  <ivl_target.h>
# include  "edif_priv.h"
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
	    fprintf(stderr, "fpga.tgt: unknown LPM type %u\n",
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
