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
#ident "$Id: enables.c,v 1.6 2002/08/12 01:35:03 steve Exp $"
#endif

# include "config.h"
# include  "ivl_target.h"
# include  <assert.h>
# include  "priv.h"


/*
 * Given a pin index, look at the nexus for a bufif device that is
 * driving it, if any. Save that device in the enable slot for the
 * cell. We'll try to fit it later.
 */
static void absorb_pad_enable(unsigned idx)
{
      unsigned ndx;
      ivl_nexus_t nex = bind_pin[idx].nexus;

      for (ndx = 0 ;  ndx < ivl_nexus_ptrs(nex) ;  ndx += 1) {

	    unsigned pin;
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex, ndx);
	    ivl_net_logic_t log = ivl_nexus_ptr_log(ptr);

	    if (log == 0)
		  continue;

	    pin = ivl_nexus_ptr_pin(ptr);
	    assert(pin == 0);

	    switch (ivl_logic_type(log)) {

		case IVL_LO_BUFIF0:
		case IVL_LO_BUFIF1:
		  assert(bind_pin[idx].enable == 0);
		  bind_pin[idx].enable = log;
		  break;

		default:
	    }
      }
}

void absorb_pad_enables(void)
{
      unsigned idx;

      for (idx = 0 ;  idx < pins ;  idx += 1) {

	    if (bind_pin[idx].sop == 0)
		  continue;

	    if (bind_pin[idx].nexus == 0)
		  continue;

	    absorb_pad_enable(idx);
      }
}


/*
 * $Log: enables.c,v $
 * Revision 1.6  2002/08/12 01:35:03  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.5  2002/08/11 23:47:04  steve
 *  Add missing Log and Ident strings.
 *
 * Revision 1.4  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.3  2001/02/07 22:22:00  steve
 *  ivl_target header search path fixes.
 *
 * Revision 1.2  2000/12/09 03:42:52  steve
 *  Stuff registers into macrocells.
 *
 * Revision 1.1  2000/12/09 01:17:38  steve
 *  Add the pal loadable target.
 *
 */

