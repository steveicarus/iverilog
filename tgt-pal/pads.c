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
#ident "$Id: pads.c,v 1.4 2002/08/12 01:35:03 steve Exp $"
#endif

# include "config.h"

# include  "priv.h"
# include  <stdio.h>
# include  <stdlib.h>
# include  <assert.h>

/*
 * This function scans the netlist for all the pin assignments that
 * are fixed by a PAD attribute. Search the scopes recursively,
 * looking for signals that may have PAD attributes.
 */
int get_pad_bindings(ivl_scope_t net, void*x)
{
      unsigned idx;

      int rc = ivl_scope_children(net, get_pad_bindings, 0);
      if (rc)
	    return rc;

      for (idx = 0 ;  idx < ivl_scope_sigs(net) ;  idx += 1) {

	    ivl_signal_t sig;
	    const char*pad;
	    int pin;

	    sig = ivl_scope_sig(net, idx);
	    pad = ivl_signal_attr(sig, "PAD");
	    if (pad == 0)
		  continue;

	    pin = strtol(pad+1, 0, 10);
	    if ((pin == 0) || (pin > pins)) {
		  printf("%s: Invalid PAD assignment: %s\n",
			 ivl_signal_name(sig), pad);
		  error_count += 1;
		  continue;
	    }

	    assert(ivl_signal_pins(sig) == 1);

	    if (bind_pin[pin-1].nexus) {

		  if (bind_pin[pin-1].nexus != ivl_signal_pin(sig, 0)) {

			printf("%s: Unconnected signals share pin %d\n",
			       ivl_signal_name(sig), pin);
			error_count += 1;
		  }

		  continue;
	    }

	    bind_pin[pin-1].nexus = ivl_signal_pin(sig, 0);
      }

      return 0;
}

/*
 * $Log: pads.c,v $
 * Revision 1.4  2002/08/12 01:35:03  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.3  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.2  2001/01/15 00:05:39  steve
 *  Add client data pointer for scope and process scanners.
 *
 * Revision 1.1  2000/12/09 01:17:38  steve
 *  Add the pal loadable target.
 *
 */

