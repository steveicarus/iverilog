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

#include  <stdlib.h>
#include  <string.h>
#include  <assert.h>
#include  <veriuser.h>
#include  <vpi_user.h>
#include  "priv.h"

/*
 * tf_getlongp implemented using VPI interface
 */
int tf_getlongp(int *highvalue, int n)
{
      vpiHandle sys_i, arg_h = 0;
      s_vpi_value value;
      int len, rtn;

      assert(highvalue);
      assert(n > 0);

      /* get task/func handle */
      sys_i = vpi_iterate(vpiArgument, cur_instance);

      /* find nth arg */
      while (n > 0) {
	    if (!(arg_h = vpi_scan(sys_i))) assert(0);
	    n--;
      }

      /* get the value */
      value.format = vpiHexStrVal;
      vpi_get_value(arg_h, &value);

      /* convert string to int(s) */
      len = strlen(value.value.str);
      if (len > 8) {
	    char *str;
	    /* low word */
	    str = value.value.str + (len - 8);
	    rtn = (int) strtoul(str, 0, 16);
	    /* high word */
	    *str = '\0';
	    *highvalue = (int) strtoul(value.value.str, 0, 16);
      } else  {
	    *highvalue = 0;
	    rtn = (int) strtoul(value.value.str, 0, 16);
      }

      vpi_free_object(sys_i);

      return rtn;
}
