/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: imain.c,v 1.11 2002/08/12 01:35:03 steve Exp $"
#endif

# include "config.h"

/*
 * This module generates a PAL that implements the design.
 */

# include  "priv.h"

#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdio.h>
# include  <stdlib.h>
# include  <assert.h>

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
      assert(bind_pin);

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

/*
 * $Log: imain.c,v $
 * Revision 1.11  2002/08/12 01:35:03  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.10  2001/09/30 16:45:10  steve
 *  Fix some Cygwin DLL handling. (Venkat Iyer)
 *
 * Revision 1.9  2001/09/15 18:27:04  steve
 *  Make configure detect malloc.h
 *
 * Revision 1.8  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.7  2001/05/20 15:09:40  steve
 *  Mingw32 support (Venkat Iyer)
 *
 * Revision 1.6  2001/01/15 00:05:39  steve
 *  Add client data pointer for scope and process scanners.
 *
 * Revision 1.5  2001/01/09 03:10:48  steve
 *  Generate the jedec to configure the macrocells.
 *
 * Revision 1.4  2000/12/14 23:37:47  steve
 *  Start support for fitting the logic.
 *
 * Revision 1.3  2000/12/09 05:40:42  steve
 *  documentation...
 *
 * Revision 1.2  2000/12/09 03:42:52  steve
 *  Stuff registers into macrocells.
 *
 * Revision 1.1  2000/12/09 01:17:38  steve
 *  Add the pal loadable target.
 *
 * Revision 1.1  2000/12/02 04:50:32  steve
 *  Make the null target into a loadable target.
 *
 */

