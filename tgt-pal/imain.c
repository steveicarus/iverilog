/*
 * Copyright (c) 2000-2010 Stephen Williams (steve@icarus.com)
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
 * This module generates a PAL that implements the design.
 */

# include  "priv.h"

# include  <stdio.h>
# include  <stdlib.h>
# include  <assert.h>
# include  "ivl_alloc.h"

extern void dump_final_design(FILE*out);

/*
 * As processing proceeds, this variable is incremented as errors are
 * encountered. This allows the code generator to give up if it
 * detects errors deep within recursive functions.
 */
unsigned pal_errors = 0;

/*
 * This is the pal device that the user asked for.
 */
pal_t pal = 0;

/*
 * These variables are the global pin assignment array. Everything
 * operates to build this up.
 */
unsigned pins = 0;
struct pal_bind_s* bind_pin = 0;


/*
 * This is the main entry point that Icarus Verilog calls to generate
 * code for a pal.
 */
int target_design(ivl_design_t des)
{
      unsigned idx;
      const char*part;
      ivl_scope_t root;

	/* Get the part type from the design, using the "part"
	   key. Given the part type, try to open the pal description
	   so that we can figure out the device. */
      part = ivl_design_flag(des, "part");
      if ((part == 0) || (*part == 0)) {
	    fprintf(stderr, "error: part must be specified. Specify a\n");
	    fprintf(stderr, "     : type with the -fpart=<type> option.\n");
	    return -1;
      }

      pal = pal_alloc(part);
      if (pal == 0) {
	    fprintf(stderr, "error: %s is not a valid part type.\n", part);
	    return -1;
      }

      assert(pal);

      pins = pal_pins(pal);
      assert(pins > 0);

	/* Allocate the pin array, ready to start assigning resources. */
      bind_pin = calloc(pins, sizeof (struct pal_bind_s));

	/* Connect all the macrocells that drive pins to the pin that
	   they drive. This doesn't yet look at the design, but is
	   initializing the bind_pin array with part information. */
      for (idx = 0 ;  idx < pal_sops(pal) ;  idx += 1) {
	    pal_sop_t sop = pal_sop(pal, idx);
	    int spin = pal_sop_pin(sop);

	    if (spin == 0)
		  continue;

	    assert(spin > 0);
	    bind_pin[spin-1].sop = sop;
      }


	/* Get pin assignments from the user. This is the first and
	   most constrained step. Everything else must work around the
	   results of these bindings. */
      root = ivl_design_root(des);
      get_pad_bindings(root, 0);

      if (pal_errors) {
	    fprintf(stderr, "PAD assignment failed.\n");
	    pal_free(pal);
	    return -1;
      }

	/* Run through the assigned output pins and absorb the output
	   enables that are connected to them. */
      absorb_pad_enables();

	/* Scan all the registers, and assign them to
	   macro-cells. */
      root = ivl_design_root(des);
      fit_registers(root, 0);
      if (pal_errors) {
	    fprintf(stderr, "Register fitting failed.\n");
	    pal_free(pal);
	    return -1;
      }

      fit_logic();
      if (pal_errors) {
	    fprintf(stderr, "Logic fitting failed.\n");
	    pal_free(pal);
	    return -1;
      }

      dump_final_design(stdout);
      emit_jedec(ivl_design_flag(des, "-o"));

      pal_free(pal);
      return 0;
}
