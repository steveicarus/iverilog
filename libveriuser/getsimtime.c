/*
 * Copyright (c) 2002-2024 Michael Ruff (mruff at chiaro.com)
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
#include  <stdlib.h>
#include  <math.h>
#include  "config.h"
#include  "priv.h"
#include  <assert.h>

/*
 * some TF time routines implemented using VPI interface
 */

// On some platforms (e.g. MinGW), pow() may not always generate an
// exact integer result when supplied with integer operands. Converting
// the result to an integer before we use it seems to be enough to work
// round this issue.
static ivl_u64_t pow10u(PLI_INT32 val)
{
      return (ivl_u64_t)pow(10, val);
}

static ivl_u64_t
scale(int high, int low, void*obj)
{
      ivl_u64_t scaled;

      vpiHandle use_obj = obj;
      if (use_obj == 0) {
	      /* If object is not passed in, then use current scope. */
	    vpiHandle hand = vpi_handle(vpiScope, cur_instance);
	    use_obj = hand;
      } else {
	      /* If object IS passed in, make sure it is a scope. If
		 it is not, then get the scope of the object. We need
		 a scope handle to go on. */
	    switch (vpi_get(vpiType,use_obj)) {
		case vpiModule:
		case vpiGenScope:
		case vpiFunction:
		case vpiTask:
		case vpiNamedBegin:
		case vpiNamedFork:
		  break;
		default:
		  use_obj = vpi_handle(vpiScope, use_obj);
		  break;
	    }
      }

      scaled = high;
      scaled = (scaled << 32) | low;
      scaled /= pow10u(vpi_get(vpiTimeUnit, use_obj) - vpi_get(vpiTimePrecision,0));

      return scaled;
}


PLI_INT32 tf_gettime(void)
{
      s_vpi_time timerec;
      timerec.type = vpiSimTime;
      vpi_get_time (0, &timerec);
      return scale(timerec.high, timerec.low, 0) & 0xffffffff;
}

char *tf_strgettime(void)
{
      static char buf[32];
      s_vpi_time timerec;

      timerec.type = vpiSimTime;
      vpi_get_time (0, &timerec);
      if (timerec.high)
	    snprintf(buf, sizeof(buf)-1, "%u%08u", (unsigned int)timerec.high,
	             (unsigned int)timerec.low);
      else
	    snprintf(buf, sizeof(buf)-1, "%u", (unsigned int)timerec.low);
      return buf;
}

PLI_INT32 tf_igetlongtime(PLI_INT32 *high, void*obj)
{
      s_vpi_time timerec;
      ivl_u64_t scaled;
      timerec.type = vpiSimTime;
      vpi_get_time ((vpiHandle)obj, &timerec);
      scaled = scale(timerec.high, timerec.low, obj);

      *high = (scaled >> 32) & 0xffffffff;
      return scaled & 0xffffffff;
}

PLI_INT32 tf_getlongtime(PLI_INT32 *high)
{
      return tf_igetlongtime(high, 0);
}

/*
 * This function is not defined in the IEEE standard, but is provided for
 * compatibility with other simulators. On platforms that support this,
 * make it a weak symbol just in case the user has defined their own
 * function for this.
 */
#if !defined(__CYGWIN__) && !defined(__MINGW32__)
PLI_INT32 tf_getlongsimtime(PLI_INT32 *high) __attribute__ ((weak));
#endif
PLI_INT32 tf_getlongsimtime(PLI_INT32 *high)
{
      s_vpi_time timerec;
      timerec.type = vpiSimTime;
      vpi_get_time (0, &timerec);

      *high = timerec.high;
      return timerec.low;
}

void tf_scale_longdelay(void*obj, PLI_INT32 low, PLI_INT32 high,
			PLI_INT32 *alow, PLI_INT32 *ahigh)
{
      ivl_u64_t scaled = scale(high, low, obj);
      *ahigh = (scaled >> 32) & 0xffffffff;
      *alow = scaled & 0xffffffff;
}

void tf_unscale_longdelay(void*obj, PLI_INT32 low, PLI_INT32 high,
			  PLI_INT32 *alow, PLI_INT32 *ahigh)
{
      ivl_u64_t unscaled;
      vpiHandle hand = vpi_handle(vpiScope, cur_instance);

      (void)obj; /* Parameter is not used. */

      unscaled = high;
      unscaled = (unscaled << 32) | low;
      unscaled *= pow(10, vpi_get(vpiTimeUnit, hand) -
			  vpi_get(vpiTimePrecision, 0));

      *ahigh = (unscaled >> 32) & 0xffffffff;
      *alow = unscaled & 0xffffffff;
}

void tf_scale_realdelay(void*obj, double real, double *areal)
{
      vpiHandle hand = vpi_handle(vpiScope, cur_instance);

      (void)obj; /* Parameter is not used. */

      *areal = real / pow(10, vpi_get(vpiTimeUnit, hand) -
			      vpi_get(vpiTimePrecision, 0));
}

void tf_unscale_realdelay(void*obj, double real, double *areal)
{
      vpiHandle hand = vpi_handle(vpiScope, cur_instance);

      (void)obj; /* Parameter is not used. */

      *areal = real * pow(10, vpi_get(vpiTimeUnit, hand) -
			      vpi_get(vpiTimePrecision, 0));
}

PLI_INT32 tf_gettimeprecision(void)
{
      PLI_INT32 rc;
      vpiHandle hand;

      hand = vpi_handle(vpiScope, cur_instance);
      rc = vpi_get(vpiTimePrecision, hand);

      if (pli_trace)
	    fprintf(pli_trace, "tf_gettimeprecision(<%s>) --> %d\n",
		    vpi_get_str(vpiName, cur_instance), (int)rc);

      return rc;
}

PLI_INT32 tf_igettimeprecision(void*obj)
{
      PLI_INT32 rc;

      if (obj == 0) {
	      /* If the obj pointer is null, then get the simulation
		 time precision. */
	    rc = vpi_get(vpiTimePrecision, 0);

      } else {

	    vpiHandle scope = vpi_handle(vpiScope, (vpiHandle)obj);
	    assert(scope);
	    rc = vpi_get(vpiTimePrecision, scope);
      }

      if (pli_trace)
	    fprintf(pli_trace, "tf_igettimeprecision(<%s>) --> %d\n",
		    obj? vpi_get_str(vpiName, obj) : ".", (int)rc);

      return rc;
}


PLI_INT32 tf_gettimeunit(void)
{
      vpiHandle hand = vpi_handle(vpiScope, cur_instance);
      return vpi_get(vpiTimeUnit, hand);
}

PLI_INT32 tf_igettimeunit(void*obj)
{
      return vpi_get(!obj ? vpiTimePrecision : vpiTimeUnit, (vpiHandle)obj);
}
