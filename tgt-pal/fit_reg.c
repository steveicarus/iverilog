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
#ident "$Id: fit_reg.c,v 1.7 2002/08/12 01:35:03 steve Exp $"
#endif

# include "config.h"

# include  "ivl_target.h"
# include  <stdio.h>
# include  <assert.h>
# include  "priv.h"

/*
 * The fit_registers function scans all the scopes for flip-flop
 * devices to be assigned to macrocells. First look to see if the
 * device is connected to a PAD directly or through a bufif device. If
 * not, then just pick a free macrocell and drop it there.
 */

static int scan_ff_q(ivl_lpm_t ff, unsigned q);

int fit_registers(ivl_scope_t scope, void*x)
{
      int rc;
      unsigned idx;
      unsigned lpms;

	/* Scan child scopes first... */
      rc = ivl_scope_children(scope, fit_registers, 0);
      if (rc != 0)
	    return rc;


	/* Scan the current scope for flip-flop devices. Pass the
	   devices we find to the scan_ff_q function to assign to a
	   macrocell. */

      lpms = ivl_scope_lpms(scope);
      for (idx = 0 ;  idx < lpms ;  idx += 1) {
	    ivl_lpm_t lpm = ivl_scope_lpm(scope, idx);
	    unsigned wid, q;

	    if (ivl_lpm_type(lpm) != IVL_LPM_FF)
		  continue;

	    wid = ivl_lpm_width(lpm);

	    for (q = 0 ;  q < wid ;  q += 1) {
		  rc = scan_ff_q(lpm, q);
		  if (rc != 0)
			return rc;
	    }
      }

      return 0;
}

/*
 * This is the part that actually assigns the single bit of a single
 * flip-flop to a single macrocell.
 */
int scan_ff_q(ivl_lpm_t ff, unsigned q)
{
      unsigned idx;
      ivl_nexus_t nex;

      nex = ivl_lpm_q(ff, q);

	/* First, look to see if the Q is already connected to a pin
	   or an enable. If I find such a connection, then immediately
	   finish. */

      for (idx = 0 ; idx < pins ;  idx += 1) {
	    struct pal_bind_s*pin = bind_pin + idx;

	    if (pin->sop == 0)
		  continue;

	    if (pin->enable) {

		  if (nex == ivl_logic_pin(pin->enable, 1)) {
			pin->reg = ff;
			pin->reg_q = q;
			return 0;
		  }

	    } else if (pin->nexus == nex) {

		  pin->reg = ff;
		  pin->reg_q = q;
		  return 0;
	    }
      }

	/* There is no pin connection, so try setting this to an
	   unbound sop cell. We know that a sop is unbound if there
	   are no enables, nexus or ff devices connected to it. */

      for (idx = 0 ;  idx < pins ;  idx += 1) {
	    struct pal_bind_s*pin = bind_pin + idx;

	    if (pin->sop == 0)
		  continue;

	    if (pin->enable || pin->nexus || pin->reg)
		  continue;

	      /* Found one. Put the reg here. Leave the nexus empty so
		 that the code generator knows to disable the pin. */
	    pin->reg = ff;
	    pin->reg_q = q;
	    return 0;
      }

      fprintf(stderr, "No room for this ff.\n");
      error_count += 1;
      return 0;
}

/*
 * $Log: fit_reg.c,v $
 * Revision 1.7  2002/08/12 01:35:03  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.6  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.5  2001/05/16 03:55:30  steve
 *  Update to new LPM API for flip-flops.
 *
 * Revision 1.4  2001/02/07 22:22:00  steve
 *  ivl_target header search path fixes.
 *
 * Revision 1.3  2001/01/15 00:05:39  steve
 *  Add client data pointer for scope and process scanners.
 *
 * Revision 1.2  2000/12/09 05:40:42  steve
 *  documentation...
 *
 * Revision 1.1  2000/12/09 03:42:52  steve
 *  Stuff registers into macrocells.
 *
 */

