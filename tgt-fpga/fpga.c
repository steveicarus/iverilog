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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: fpga.c,v 1.3 2001/09/01 02:01:30 steve Exp $"
#endif

# include "config.h"

/*
 * This is a null target module. It does nothing.
 */

# include  <ivl_target.h>
# include  "fpga_priv.h"

/* This is the opened xnf file descriptor. It is the output that this
   code generator writes to. */
FILE*xnf = 0;

const char*part = 0;
device_t device = 0;

extern const struct device_s d_generic;

static int show_process(ivl_process_t net, void*x)
{
      fprintf(stderr, "fpga target: unsynthesized behavioral code\n");
      return 0;
}

static void show_root_ports(ivl_scope_t root)
{
      unsigned cnt = ivl_scope_sigs(root);
      unsigned idx;

      for (idx = 0 ;  idx < cnt ;  idx += 1) {
	    ivl_signal_t sig = ivl_scope_sig(root, idx);
	    const char*use_name;

	    if (ivl_signal_port(sig) == IVL_SIP_NONE)
		  continue;

	    use_name = ivl_signal_basename(sig);
	    if (ivl_signal_pins(sig) == 1) {
		  ivl_nexus_t nex = ivl_signal_pin(sig, 0);
		  fprintf(xnf, "SIG, %s, PIN=%s\n",
			  mangle_nexus_name(nex), use_name);

	    } else {
		  unsigned pin;

		  for (pin = 0 ; pin < ivl_signal_pins(sig); pin += 1) {
			ivl_nexus_t nex = ivl_signal_pin(sig, pin);
			fprintf(xnf, "SIG, %s, PIN=%s%u\n",
				mangle_nexus_name(nex), use_name,
				pin);
		  }
	    }
      }
}

static void show_design_consts(ivl_design_t des)
{
      unsigned idx;

      for (idx = 0 ;  idx < ivl_design_consts(des) ;  idx += 1) {
	    unsigned pin;
	    ivl_net_const_t net = ivl_design_const(des, idx);
	    const char*val = ivl_const_bits(net);

	    for (pin = 0 ;  pin < ivl_const_pins(net) ;  pin += 1) {
		  ivl_nexus_t nex = ivl_const_pin(net, pin);
		  fprintf(xnf, "PWR,%c,%s\n", val[pin],
			  mangle_nexus_name(nex));
	    }
      }
}

/*
 * This is the main entry point that ivl uses to invoke me, the code
 * generator.
 */
int target_design(ivl_design_t des)
{
      const char*path = ivl_design_flag(des, "-o");
      ivl_scope_t root = ivl_design_root(des);

      xnf = fopen(path, "w");
      if (xnf == 0) {
	    perror(path);
	    return -1;
      }

      fprintf(xnf, "LCANET,6\n");
      fprintf(xnf, "PROG,iverilog,$Name:  $,\"Icarus Verilog/fpga.tgt\"\n");

      part = ivl_design_flag(des, "part");
      if (part && (part[0]!=0)) {
	    fprintf(xnf, "PART,%s\n", part);
      }

      device = &d_generic;

	/* Catch any behavioral code that is left, and write warnings
	   that it is not supported. */
      ivl_design_process(des, show_process, 0);

      show_root_ports(root);

	/* Scan the scopes, looking for gates to draw into the output
	   netlist. */
      show_scope_gates(root, 0);

      show_design_consts(des);

      fprintf(xnf, "EOF\n");
      return 0;
}

/*
 * $Log: fpga.c,v $
 * Revision 1.3  2001/09/01 02:01:30  steve
 *  identity compare, and PWR records for constants.
 *
 * Revision 1.2  2001/08/31 02:59:06  steve
 *  Add root port SIG records.
 *
 * Revision 1.1  2001/08/28 04:14:20  steve
 *  Add the fpga target.
 *
 */

