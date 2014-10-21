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

# include "config.h"

/*
 * This is the FPGA target module.
 */

# include  <ivl_target.h>
# include  <string.h>
# include  "fpga_priv.h"
# include  <assert.h>

/* This is the opened xnf file descriptor. It is the output that this
   code generator writes to. */
FILE*xnf = 0;

const char*part = 0;
const char*arch = 0;
device_t device = 0;

int scope_has_attribute(ivl_scope_t s, const char *name)
{
      int i;
      for (i=0; i<ivl_scope_attr_cnt(s); i++) {
	    const struct ivl_attribute_s *a;
	    a = ivl_scope_attr_val(s, i);
	    if (strcmp(a->key,name) == 0) return 1;
      }
      return 0;
}

static int show_process(ivl_process_t net, void*x)
{
      ivl_scope_t scope = ivl_process_scope(net);

	/* Ignore processes that are within scopes that are cells. The
	   cell_scope will generate a cell to represent the entire
	   scope. */
      if (scope_has_attribute(scope, "ivl_synthesis_cell"))
	    return 0;

      fprintf(stderr, "fpga target: unsynthesized behavioral code\n");
      return 0;
}

static void show_pads(ivl_scope_t scope)
{
      unsigned idx;

      if (device->show_pad == 0)
	    return;

      for (idx = 0 ;  idx < ivl_scope_sigs(scope) ;  idx += 1) {
	    ivl_signal_t sig = ivl_scope_sig(scope, idx);
	    const char*pad;

	    if (ivl_signal_port(sig) == IVL_SIP_NONE)
		  continue;

	    pad = ivl_signal_attr(sig, "PAD");
	    if (pad == 0)
		  continue;

	    assert(device->show_pad);
	    device->show_pad(sig, pad);
      }
}

static void show_constants(ivl_design_t des)
{
      unsigned idx;

      if (device->show_constant == 0)
	    return;

      for (idx = 0 ;  idx < ivl_design_consts(des) ;  idx += 1) {
	    ivl_net_const_t con = ivl_design_const(des, idx);
	    device->show_constant(con);
      }
}

/*
 * This is the main entry point that ivl uses to invoke me, the code
 * generator.
 */
int target_design(ivl_design_t des)
{
      ivl_scope_t root = ivl_design_root(des);
      const char*path = ivl_design_flag(des, "-o");

      xnf = fopen(path, "w");
      if (xnf == 0) {
	    perror(path);
	    return -1;
      }

      part = ivl_design_flag(des, "part");
      if (part && (part[0] == 0))
	    part = 0;

      arch = ivl_design_flag(des, "arch");
      if (arch && (arch[0] == 0))
	    arch = 0;

      if (arch == 0)
	    arch = "lpm";

      device = device_from_arch(arch);
      if (device == 0) {
	    fprintf(stderr, "Unknown architecture arch=%s\n", arch);
	    return -1;
      }

	/* Call the device driver to generate the netlist header. */
      device->show_header(des);

	/* Catch any behavioral code that is left, and write warnings
	   that it is not supported. */
      ivl_design_process(des, show_process, 0);

	/* Get the pads from the design, and draw them to connect to
	   the associated signals. */
      show_pads(root);

	/* Scan the scopes, looking for gates to draw into the output
	   netlist. */
      show_scope_gates(root, 0);

      show_constants(des);

	/* Call the device driver to close out the file. */
      device->show_footer(des);

      fclose(xnf);
      xnf = 0;
      return 0;
}
