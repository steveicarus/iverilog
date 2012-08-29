/*
 * Copyright (c) 2002 Michael Ruff (mruff at chiaro.com)
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

#include  <assert.h>
#include  <vpi_user.h>
#include  <acc_user.h>

#undef NULL
#define NULL 0

/*
 * acc_next_topmod implemented using VPI interface
 */
handle acc_next_topmod(handle prev_topmod)
{
      static vpiHandle last = NULL;
      static vpiHandle mod_i = NULL;

      if (!prev_topmod) {
	    /* start over */
	    mod_i = vpi_iterate(vpiModule, NULL);
      } else {
	    /* subsequent time through */
	    assert(prev_topmod == last);
      }

      last = vpi_scan(mod_i);
      return last;
}
