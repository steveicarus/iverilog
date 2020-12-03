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

#include  <stdio.h>
#include  <assert.h>
#include  <veriuser.h>
#include  <vpi_user.h>
#include  "priv.h"

/*
 * tf_putlongp implemented using VPI interface
 */
void tf_putlongp(int n, int lowvalue, int highvalue)
{
      vpiHandle sys_i, arg_h = 0;
      s_vpi_value val;
      int type;
      char str[20];


      assert(n >= 0);

      /* get task/func handle */
      sys_i = vpi_iterate(vpiArgument, cur_instance);

      type = vpi_get(vpiType, cur_instance);

      /* verify function */
      assert(!(n == 0 && type != vpiSysFuncCall));

      /* find nth arg */
      while (n > 0) {
	    if (!(arg_h = vpi_scan(sys_i))) assert(0);
	    n--;
      }
      if (!arg_h) arg_h = cur_instance;

      /* fill in vpi_value */
      sprintf(str, "%x%08x", highvalue, lowvalue);
      val.format = vpiHexStrVal;
      val.value.str = str;
      vpi_put_value(arg_h, &val, 0, vpiNoDelay);

      vpi_free_object(sys_i);
}
