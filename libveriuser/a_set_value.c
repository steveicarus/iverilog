/*
 * Copyright (c) 2002-2009 Michael Ruff (mruff at chiaro.com)
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
#include  <acc_user.h>
#include  <vpi_user.h>

/*
 * acc_set_value implemented using VPI interface
 */
int acc_set_value(handle object, p_setval_value value, p_setval_delay delay)
{
      s_vpi_time when, *whenp;
      s_vpi_value val;
      int flags;

      assert(delay);
      assert(value);
      assert(object);

      /* map setval_delay.model to flags */
      switch (delay->model) {
	    case accNoDelay:            flags = vpiNoDelay; break;
	    case accInertialDelay:      flags = vpiInertialDelay; break;
	    case accTransportDelay:     flags = vpiTransportDelay; break;
	    case accPureTransportDelay: flags = vpiPureTransportDelay; break;
	    case accForceFlag:          flags = vpiForceFlag; break;
	    case accReleaseFlag:        flags = vpiReleaseFlag; break;
	    default: flags = -1; assert(0); break;
      }

      /* map acc_time to vpi_time */
      if (delay->model != accNoDelay) {
	    switch (delay->time.type) {
		  case accSimTime:  when.type = vpiSimTime; break;
		  case accRealTime: when.type = vpiScaledRealTime; break;
		  default: assert(0); break;
	    }
	    when.high = delay->time.high;
	    when.low = delay->time.low;
	    when.real = delay->time.real;

	    whenp = &when;
      } else
	    whenp = 0;

      /* map setval_value to vpi_value and flags */
      switch (value->format) {
	    case accBinStrVal:
		  val.format = vpiBinStrVal;
		  val.value.str = value->value.str;
		  break;
	    case accOctStrVal:
		  val.format = vpiOctStrVal;
		  val.value.str = value->value.str;
		  break;
	    case accDecStrVal:
		  val.format = vpiDecStrVal;
		  val.value.str = value->value.str;
		  break;
	    case accHexStrVal:
		  val.format = vpiHexStrVal;
		  val.value.str = value->value.str;
		  break;
	    case accScalarVal:
		  val.format = vpiScalarVal;
		  val.value.scalar = value->value.scalar;
		  break;
	    case accIntVal:
		  val.format = vpiIntVal;
		  val.value.integer = value->value.integer;
		  break;
	    case accRealVal:
		  val.format = vpiRealVal;
		  val.value.real = value->value.real;
		  break;
	    case accStringVal:
		  val.format = vpiStringVal;
		  val.value.str = value->value.str;
		  break;
	    case accVectorVal:
		  val.format = vpiVectorVal;
		  val.value.vector = (p_vpi_vecval)value->value.vector;
		  break;
	    default:
	      vpi_printf("XXXX acc_set_value(value->format=%d)\n",
			 value->format);
	      assert(0);
	      break;
      }

      /* put value */
      vpi_put_value(object, &val, whenp, flags);

      return 1;
}
