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
#if !defined(WINNT)
#ident "$Id: fit_reg.c,v 1.1 2000/12/09 03:42:52 steve Exp $"
#endif

# include  <ivl_target.h>
# include  <stdio.h>
# include  <assert.h>
# include  "priv.h"

static int scan_ff_q(ivl_lpm_ff_t ff, unsigned q);

int fit_registers(ivl_scope_t scope)
{
      int rc;
      unsigned idx;
      unsigned lpms;

      rc = ivl_scope_children(scope, fit_registers);
      if (rc != 0)
	    return rc;

      lpms = ivl_scope_lpms(scope);

      for (idx = 0 ;  idx < lpms ;  idx += 1) {
	    ivl_lpm_t lpm = ivl_scope_lpm(scope, idx);
	    ivl_lpm_ff_t ff;
	    unsigned wid, q;

	    if (ivl_lpm_type(lpm) != IVL_LPM_FF)
		  continue;

	    wid = ivl_lpm_width(lpm);
	    ff = ivl_lpm_ff(lpm);

	    for (q = 0 ;  q < wid ;  q += 1) {
		  rc = scan_ff_q(ff, q);
		  if (rc != 0)
			return rc;
	    }
      }

      return 0;
}

int scan_ff_q(ivl_lpm_ff_t ff, unsigned q)
{
      unsigned idx;
      ivl_nexus_t nex;

      nex = ivl_lpm_ff_q(ff, q);

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

	/* There is no poin connection, so try setting this to an
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
 * Revision 1.1  2000/12/09 03:42:52  steve
 *  Stuff registers into macrocells.
 *
 */

