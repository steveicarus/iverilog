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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

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
