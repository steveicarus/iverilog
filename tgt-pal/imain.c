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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: imain.c,v 1.1 2000/12/09 01:17:38 steve Exp $"
#endif

/*
 * This module generates a PAL that implements the design.
 */

# include  "priv.h"

# include  <malloc.h>
# include  <stdio.h>
# include  <stdlib.h>
# include  <assert.h>

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

static void dump_final_design(FILE*out)
{
      unsigned idx;
      for (idx = 0 ;  idx < pins ;  idx += 1) {
	    if (bind_pin[idx].sop) {
		  fprintf(out, "Output pin %u:\n", idx+1);
		  fprintf(out, "    pin nexus=%s\n",
			  bind_pin[idx].nexus?
			  ivl_nexus_name(bind_pin[idx].nexus) : "");
		  fprintf(out, "    pin enable=%s\n",
			  bind_pin[idx].enable ?
			  ivl_logic_name(bind_pin[idx].enable) : "1");

	    } else {
		  fprintf(out, "Input pin %u:\n", idx+1);
		  fprintf(out, "    pin nexus=%s\n",
			  bind_pin[idx].nexus?
			  ivl_nexus_name(bind_pin[idx].nexus) : "");
	    }
      }
}



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
      assert(part);
      pal = pal_alloc(part);
      assert(pal);

      pins = pal_pins(pal);
      assert(pins > 0);

	/* Allocate the pin array, ready to start assigning resources. */
      bind_pin = calloc(pins, sizeof (struct pal_bind_s));
      assert(bind_pin);

	/* Connect all the macrocells that drive pins to the pin that
	   they drive. */
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
      get_pad_bindings(root);

      if (pal_errors) {
	    fprintf(stderr, "code generator failed.\n");
	    pal_free(pal);
	    return -1;
      }

	/* Run through the assigned output pins and absorb the output
	   enables that are connected to them. */
      absorb_pad_enables();


      dump_final_design(stdout);

      pal_free(pal);
      return 0;
}

#ifdef __CYGWIN32__
#include <cygwin/cygwin_dll.h>
DECLARE_CYGWIN_DLL(DllMain);
#endif

/*
 * $Log: imain.c,v $
 * Revision 1.1  2000/12/09 01:17:38  steve
 *  Add the pal loadable target.
 *
 * Revision 1.1  2000/12/02 04:50:32  steve
 *  Make the null target into a loadable target.
 *
 */

