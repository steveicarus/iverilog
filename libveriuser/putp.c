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

# include  <assert.h>
# include  <veriuser.h>
# include  <vpi_user.h>
# include  "priv.h"

/*
 * tf_putp and friends implemented using VPI interface
 */
PLI_INT32 tf_iputp(PLI_INT32 n, PLI_INT32 value, void *obj)
{
      vpiHandle sys_h, sys_i, arg_h = 0;
      s_vpi_value val;
      int rtn = 0, type;

      assert(n >= 0);

	/* get task/func handle */
      sys_h = (vpiHandle)obj;
      type  = vpi_get(vpiType, sys_h);

	/* Special case, we are putting the return value. */
      if ((type == vpiSysFuncCall) && (n == 0)) {
	    val.format = vpiIntVal;
	    val.value.integer = value;
	    vpi_put_value(sys_h, &val, 0, vpiNoDelay);

	    if (pli_trace) {
		  fprintf(pli_trace, "tf_iputp(<return>, value=%d, func=%s) "
			  "--> %d\n", (int)value, vpi_get_str(vpiName, obj), 0);
	    }

	    return 0;
      }

      if ((n == 0) && (type != vpiSysFuncCall)) {
	    if (pli_trace) {
		  fprintf(pli_trace, "tf_iputp(<ERROR>, value=%d, func=%s) "
			  "--> %d\n", (int)value, vpi_get_str(vpiName, obj), 1);
	    }

	    return 1;
      }

      sys_i = vpi_iterate(vpiArgument, sys_h);

      /* find nth arg */
      while (n > 0) {
	    if (!(arg_h = vpi_scan(sys_i))) { rtn = 1; goto out; }
	    n--;
      }

      /* fill in vpi_value */
      val.format = vpiIntVal;
      val.value.integer = value;
      vpi_put_value(arg_h, &val, 0, vpiNoDelay);

      if (arg_h)
	    vpi_free_object(sys_i);

 out:
      if (pli_trace) {
	    fprintf(pli_trace, "tf_iputp(n=%d, value=%d, obj=%p) --> %d\n",
		  (int)n, (int)value, obj, rtn);
      }

      return rtn;
}

PLI_INT32 tf_putp(PLI_INT32 n, PLI_INT32 value)
{
      int rtn = tf_iputp(n, value, cur_instance);

      return rtn;
}


PLI_INT32 tf_iputrealp(PLI_INT32 n, double value, void *obj)
{
      vpiHandle sys_h, sys_i, arg_h = 0;
      s_vpi_value val;
      int rtn = 0, type;

      assert(n >= 0);

      /* get task/func handle */
      sys_h = (vpiHandle)obj;
      sys_i = vpi_iterate(vpiArgument, sys_h);

      type = vpi_get(vpiType, sys_h);

      /* verify function */
      if (n == 0 && type != vpiSysFuncCall) { rtn = 1; goto free; }

      /* find nth arg */
      while (n > 0) {
	    if (!(arg_h = vpi_scan(sys_i))) { rtn = 1; goto out; }
	    n--;
      }
      if (!arg_h) arg_h = sys_h;

      /* fill in vpi_value */
      val.format = vpiRealVal;
      val.value.real = value;
      vpi_put_value(arg_h, &val, 0, vpiNoDelay);

free:
      vpi_free_object(sys_i);

out:
      if (pli_trace) {
	    fprintf(pli_trace, "tf_iputrealp(n=%d, value=%f, obj=%p) --> %d\n",
		  (int)n, value, obj, rtn);
      }

      return rtn;
}

PLI_INT32 tf_putrealp(PLI_INT32 n, double value)
{
      int rtn = tf_iputrealp(n, value, cur_instance);

      return rtn;
}
