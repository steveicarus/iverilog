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

/*
 * This is the EDIF target module.
 */

# include  <ivl_target.h>
# include  <string.h>
# include  "edif_priv.h"
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
      const struct ivl_attribute_s *a;
      for (i=0; i<ivl_scope_attr_cnt(s); i++) {
	      a = ivl_scope_attr_val(s, i);
	      if (strcmp(a->key,name) == 0)
		    return 1;
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

/*
 * This function tests whether the nexus has a constant value. It
 * returns true if the value is constant, or false if there are
 * non-constant or conflicting drivers.
 */
int test_nexus_constant(ivl_nexus_t nex, char*val)
{
      int count_drivers = 0;
      unsigned idx;

      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ivl_net_const_t con;
	    unsigned pin;
	    const char*cbits;
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex, idx);

	      /* If this nexus link is an input pin to the device (or
		 otherwise does not drive the nexus) then skip it. */
	    if (ivl_nexus_ptr_drive0(ptr) == IVL_DR_HiZ
		&& ivl_nexus_ptr_drive1(ptr) == IVL_DR_HiZ)
		  continue;

	    count_drivers += 1;

	      /* If this driver is not a constant, then the test fails
		 certainly. */
	    con = ivl_nexus_ptr_con(ptr);
	    if (con == 0)
		  return 0;

	      /* Get the pin number within the constant where this
		 nexus is connected. */
	    pin = ivl_nexus_ptr_pin(ptr);

	      /* The pin for the constant that we located is
		 guaranteed to really point to the nexus that we are
		 working with. */
	    assert(ivl_const_pins(con) > pin);
	    assert(ivl_const_pin(con,pin) == nex);

	      /* Get the bit value from the constant. If there are
		 multiple constants driving this nexus (an unlikely
		 situation) then allow for them only if their value
		 matches. But the more common case is that this is the
		 only driver for the nexus. Save the constant value in
		 the *val result that we pass back to the user. */
	    cbits = ivl_const_bits(con);
	    if (count_drivers > 1) {
		  if (val[0] != cbits[pin])
			return 0;
	    } else {
		  val[0] = cbits[pin];
	    }
      }

	/* If in the end there are no drivers at all for the nexus,
	   then assume the nexus has a constant HiZ value. */
      if (count_drivers == 0)
	    *val = 'z';

	/* Return TRUE */
      return 1;
}
