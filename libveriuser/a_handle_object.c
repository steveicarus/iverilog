/*
 * Copyright (c) 2003-2020 Stephen Williams (steve@icarus.com)
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

#include  <vpi_user.h>
#include  <acc_user.h>
#include  "priv.h"

static vpiHandle search_scope = 0;

handle acc_handle_object(const char*name)
{
      vpiHandle scope = search_scope? search_scope : vpi_handle(vpiScope, cur_instance);
      vpiHandle res = vpi_handle_by_name(name, scope);

      if (pli_trace) {
	    fprintf(pli_trace, "acc_handle_object(%s <scope=%s>) --> .\n",
		    name, acc_fetch_fullname(scope));
      }

      return res;
}

char* acc_set_scope(handle ref, ...)
{
      char*name;
      search_scope = ref;

      name = acc_fetch_fullname(search_scope);
      if (pli_trace) {
	    fprintf(pli_trace, "acc_set_scope(<scope=%s>)\n", name);
      }

      return acc_fetch_fullname(ref);
}
