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
# include  <ctype.h>
# include  <veriuser.h>
# include  <vpi_user.h>
# include  "priv.h"

/*
 * tf_getp and friends, implemented using VPI interface
 */
PLI_INT32 tf_igetp(PLI_INT32 n, void *obj)
{
      vpiHandle sys_h, sys_i, arg_h = 0;
      s_vpi_value value;
      int rtn = 0;

      assert(n > 0);

      /* get task/func handle */
      sys_h = (vpiHandle)obj;
      sys_i = vpi_iterate(vpiArgument, sys_h);

      /* find nth arg */
      while (n > 0) {
	    if (!(arg_h = vpi_scan(sys_i))) { goto out; }
	    n--;
      }

      /* If it is a constant string, return a pointer to it else int value */
      if (vpi_get(vpiType, arg_h) == vpiConstant &&
	  vpi_get(vpiConstType, arg_h) == vpiStringConst)
      {
	    value.format = vpiStringVal;
	    vpi_get_value(arg_h, &value);
	    /* The following may generate a compilation warning, but this
	     * functionality is required by some versions of the standard. */
	    rtn = (int) value.value.str;	/* Oh my */
      } else {
	    value.format = vpiIntVal;
	    vpi_get_value(arg_h, &value);
	    rtn = value.value.integer;
      }

      vpi_free_object(sys_i);

out:
      if (pli_trace) {
	    fprintf(pli_trace, "tf_igetp(n=%d, obj=%p) --> %d\n",
		  (int)n, obj, rtn);
      }

      return rtn;
}

PLI_INT32 tf_getp(PLI_INT32 n)
{
      int rtn = tf_igetp(n, cur_instance);

      return rtn;
}


double tf_igetrealp(PLI_INT32 n, void *obj)
{
      vpiHandle sys_h, sys_i, arg_h = 0;
      s_vpi_value value;
      double rtn = 0.0;

      assert(n > 0);

      /* get task/func handle */
      sys_h = (vpiHandle)obj;
      sys_i = vpi_iterate(vpiArgument, sys_h);

      /* find nth arg */
      while (n > 0) {
	    if (!(arg_h = vpi_scan(sys_i))) { goto out; }
	    n--;
      }

      if (vpi_get(vpiType, arg_h) == vpiConstant &&
	  vpi_get(vpiConstType, arg_h) == vpiStringConst)
      {
	    rtn = 0.0;
      } else {
	    value.format = vpiRealVal;
	    vpi_get_value(arg_h, &value);
	    rtn = value.value.real;
      }

      vpi_free_object(sys_i);

out:
      if (pli_trace) {
	    fprintf(pli_trace, "tf_igetrealp(n=%d, obj=%p) --> %f\n",
		  (int)n, obj, rtn);
      }

      return rtn;
}

double tf_getrealp(PLI_INT32 n)
{
      double rtn = tf_igetrealp(n, cur_instance);

      return rtn;
}


char *tf_istrgetp(PLI_INT32 n, PLI_INT32 fmt, void *obj)
{
      vpiHandle sys_h, sys_i, arg_h = 0;
      s_vpi_value value;
      char *rtn = 0;

      assert(n > 0);

      /* get task/func handle */
      sys_h = (vpiHandle)obj;
      sys_i = vpi_iterate(vpiArgument, sys_h);

      /* find nth arg */
      while (n > 0) {
	    if (!(arg_h = vpi_scan(sys_i))) { goto out; }
	    n--;
      }

      if (vpi_get(vpiType, arg_h) == vpiConstant &&
	  vpi_get(vpiConstType, arg_h) == vpiStringConst)
      {
	    value.format = vpiStringVal;
	    vpi_get_value(arg_h, &value);
	    rtn = value.value.str;
      } else {
	    value.format = -1;
	    switch (tolower(fmt)) {
		  case 'b': value.format = vpiBinStrVal; break;
		  case 'o': value.format = vpiOctStrVal; break;
		  case 'd': value.format = vpiDecStrVal; break;
		  case 'h': value.format = vpiHexStrVal; break;
	    }
	    if (value.format > 0) {
		  vpi_get_value(arg_h, &value);
		  rtn = value.value.str;
	    }
      }

      vpi_free_object(sys_i);

out:
      if (pli_trace) {
	    fprintf(pli_trace, "tf_istrgetp(n=%d, fmt=%c, obj=%p) --> \"%s\"\n",
		  (int)n, (int)fmt, obj, rtn);
      }

      return rtn;
}

char *tf_strgetp(PLI_INT32 n, PLI_INT32 fmt)
{
      char *rtn = tf_istrgetp(n, fmt, cur_instance);

      return rtn;
}
