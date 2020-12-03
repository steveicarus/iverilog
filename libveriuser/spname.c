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

#include  <assert.h>
#include  <vpi_user.h>
#include  <veriuser.h>
#include  "priv.h"

char* tf_spname(void)
{
      char*rtn;

      vpiHandle scope = vpi_handle(vpiScope, cur_instance);

      rtn = __acc_newstring(vpi_get_str(vpiFullName, scope));

      if (pli_trace) {
	    fprintf(pli_trace, "%s: tf_spname() --> %s\n",
		    vpi_get_str(vpiName, cur_instance), rtn);
      }

      return rtn;
}

char *tf_imipname(void *obj)
{
      return vpi_get_str(vpiFullName, vpi_handle(vpiScope, (vpiHandle)obj));
}

char *tf_mipname(void)
{
      return tf_imipname(cur_instance);
}
