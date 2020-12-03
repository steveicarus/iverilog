/*
 * Copyright (c) 2003-2020 Michael Ruff (mruff at chiaro.com)
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

# include  <acc_user.h>
# include  <vpi_user.h>
# include  "priv.h"
# include  <assert.h>
# include  <string.h>

/*
 * acc_handle_by_name implemented using VPI interface
 */
handle acc_handle_by_name(const char*obj_name, handle scope)
{
      vpiHandle res;

      /* if no scope provided, use tasks scope */
      if (!scope) {
	    scope = vpi_handle(vpiScope, cur_instance);
      }

      res = vpi_handle_by_name(obj_name, scope);

      if (pli_trace) {
	    fprintf(pli_trace, "acc_handle_by_name(\"%s\", scope=%s) "
		    " --> %p\n", obj_name,
		    vpi_get_str(vpiFullName, scope), res);
      }

      return res;
}
