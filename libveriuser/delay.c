/*
 * Copyright (c) 2003-2014 Stephen Williams (steve@icarus.com)
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

#include  <veriuser.h>
#include  <vpi_user.h>
#include  "priv.h"
#include  <assert.h>

static PLI_INT32 delay_callback(struct t_cb_data*cb)
{
      (void)cb; /* Parameter is not used. */
      vpi_printf("XXXX delay_callback called.\n");
      return 0;
}

int tf_isetdelay(PLI_INT32 delay, void*ss)
{
      vpiHandle sys = (vpiHandle)ss;
      int unit = vpi_get(vpiTimeUnit, sys);
      int prec = vpi_get(vpiTimePrecision, 0);

      struct t_cb_data cb;
      struct t_vpi_time ct;

      if (pli_trace) {
	    fprintf(pli_trace, "%s: tf_isetdelay(%d, ...)"
		    " <unit=%d, prec=%d>;\n",
		    vpi_get_str(vpiName, sys), (int)delay, unit, prec);
      }


	/* Convert the delay from the UNITS of the specified
	   task/function to the precision of the simulation. */
      assert(unit >= prec);

      while (unit > prec) {
	    PLI_INT32 tmp = delay * 10;
	    assert(tmp > delay);
	    delay = tmp;
	    unit -= 1;
      }

	/* Create a VPI callback to schedule the delay. */
      ct.type = vpiSimTime;
      ct.high = 0;
      ct.low  = delay;

      cb.reason = cbAfterDelay;
      cb.cb_rtn = delay_callback;
      cb.obj = 0;
      cb.time = &ct;
      cb.value = 0;
      cb.user_data = 0;
      vpi_register_cb(&cb);

      return 0;
}

int tf_setdelay(PLI_INT32 delay)
{
      return tf_isetdelay(delay, tf_getinstance());
}
