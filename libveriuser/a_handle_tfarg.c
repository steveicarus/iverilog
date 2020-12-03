/*
 * Copyright (c) 2002-2020 Michael Ruff (mruff at chiaro.com)
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

#include  <acc_user.h>
#include  <vpi_user.h>
#include  "priv.h"

/*
 * acc_handle_tfarg implemented using VPI interface
 */
handle acc_handle_tfarg(int n)
{
      vpiHandle rtn_h = 0;

      if (n > 0) {
	    vpiHandle sys_i;

	    sys_i = vpi_iterate(vpiArgument, cur_instance);

	    /* find nth arg */
	    while (n > 0) {
		  rtn_h = vpi_scan(sys_i);
		  if (rtn_h == 0)
			break;
		  n--;
	    }

	    if (rtn_h) vpi_free_object(sys_i);

      } else {
	    rtn_h = (vpiHandle) 0;
      }

      return rtn_h;
}

handle acc_handle_tfinst(void)
{
      return cur_instance;
}
