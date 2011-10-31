/*
 * Copyright (c) 2001-2011 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include  "config.h"
# include  "vpi_priv.h"
# include  "schedule.h"
# include  <cstdio>
# include  <cmath>
# include  <cassert>

/*
 * The $time system function is supported in VPI contexts (i.e. an
 * argument to a system task/function) as a vpiSysFuncCall object. The
 * $display function divines that this is a function call and uses a
 * vpi_get_value to get the value.
 */

/*
 * vpi_time_precision is the precision of the simulation clock. It is
 * set by the :vpi_time_precision directive in the vvp source file.
 */
static int vpi_time_precision = 0;

static struct __vpiSystemTime global_simtime;

void vpip_time_to_timestruct(struct t_vpi_time*ts, vvp_time64_t ti)
{
      ts->low  = ti & 0xFFFFFFFF;
      ts->high = (ti >> 32) & 0xFFFFFFFF;
}

vvp_time64_t vpip_timestruct_to_time(const struct t_vpi_time*ts)
{
      vvp_time64_t ti = ts->high;
      ti <<= 32;
      ti += ts->low & 0xffffffff;
      return ti;
}

double vpip_time_to_scaled_real(vvp_time64_t ti, struct __vpiScope*scope)
{
      double val;
      int scale = 0;
      if (scope) scale = vpi_time_precision - scope->time_units;

      if (scale >= 0) val = (double)ti * pow(10.0, scale);
      else val = (double) ti / pow(10.0, -scale);

      return val;
}

vvp_time64_t vpip_scaled_real_to_time64(double val, struct __vpiScope*scope)
{
      int units;
      if (scope)
	    units = scope->time_units;
      else
	    units = vpi_time_precision;

      double scale = pow(10.0L, units - vpi_time_precision);
      val *= scale;

      return (vvp_time64_t) val;
}

static int timevar_time_get(int code, vpiHandle ref)
{
      switch (code) {
          case vpiSize:
	    return 64;

          case vpiSigned:
	    return 0;

	  case vpiFuncType:
	    return vpiTimeFunc;

          case vpiAutomatic:
	    return 0;

	  default:
	    fprintf(stderr, "Code: %d\n", code);
	    assert(0);
	    return 0;
      }
}

static int timevar_stime_get(int code, vpiHandle ref)
{
      switch (code) {
          case vpiSize:
	    return 32;

	  default:
	    return timevar_time_get(code, ref);
      }
}

static char* timevar_time_get_str(int code, vpiHandle ref)
{
      switch (code) {
	  case vpiName:
	    return simple_set_rbuf_str("$time");
	  default:
	    fprintf(stderr, "Code: %d\n", code);
	    assert(0);
	    return 0;
      }
}

static char* timevar_stime_get_str(int code, vpiHandle ref)
{
      switch (code) {
	  case vpiName:
	    return simple_set_rbuf_str("$stime");
	  default:
	    fprintf(stderr, "Code: %d\n", code);
	    assert(0);
	    return 0;
      }
}

static char* timevar_simtime_get_str(int code, vpiHandle ref)
{
      switch (code) {
	  case vpiName:
	    return simple_set_rbuf_str("$simtime");
	  default:
	    fprintf(stderr, "Code: %d\n", code);
	    assert(0);
	    return 0;
      }
}

static char* timevar_realtime_get_str(int code, vpiHandle ref)
{
      switch (code) {
	  case vpiName:
	    return simple_set_rbuf_str("$realtime");
	  default:
	    fprintf(stderr, "Code: %d\n", code);
	    assert(0);
	    return 0;
      }
}

static int timevar_realtime_get(int code, vpiHandle ref)
{
      switch (code) {
          case vpiSize:
	    return 1;

          case vpiSigned:
	    return 0;

	  case vpiFuncType:
	    return vpiRealFunc;

          case vpiAutomatic:
	    return 0;

	  default:
	    fprintf(stderr, "Code: %d\n", code);
	    assert(0);
	    return 0;
      }
}

static vpiHandle timevar_handle(int code, vpiHandle ref)
{
      struct __vpiSystemTime*rfp
	    = reinterpret_cast<struct __vpiSystemTime*>(ref);

      switch (code) {
	  case vpiScope:
	    return &rfp->scope->base;
	  default:
	    return 0;
      }
}

static void timevar_get_value(vpiHandle ref, s_vpi_value*vp, bool is_int_func,
                              bool is_stime)
{
	/* Keep a persistent structure for passing time values back to
	   the caller. */
      static struct t_vpi_time time_value;

      struct __vpiSystemTime*rfp
	    = reinterpret_cast<struct __vpiSystemTime*>(ref);
      unsigned long num_bits;
      vvp_time64_t x, simtime = schedule_simtime();
      int units = rfp->scope? rfp->scope->time_units : vpi_time_precision;

      char*rbuf = need_result_buf(128, RBUF_VAL);

	/* Calculate the divisor needed to scale the simulation time
	   (in time_precision units) to time units of the scope. */
      vvp_time64_t divisor = 1;
      while (units > vpi_time_precision) {
	    divisor *= 10;
	    units -= 1;
      }

	/* Scale the simtime, and use the modulus to round up if
	   appropriate. */
      vvp_time64_t simtime_fraction = simtime % divisor;
      simtime /= divisor;

      if ((divisor >= 10) && (simtime_fraction >= (divisor/2)))
	    simtime += 1;

	/* If this is a call to $stime only return the lower 32 bits. */
      if (is_stime) simtime &= 0xffffffff;

      switch (vp->format) {
	  case vpiObjTypeVal:
	      /* The default format is vpiTimeVal. */
	    vp->format = vpiTimeVal;
	  case vpiTimeVal:
	    vp->value.time = &time_value;
	    vp->value.time->type = vpiSimTime;
	    vpip_time_to_timestruct(vp->value.time, simtime);
	    break;

	  case vpiRealVal:
	      /* If this is an integer based call (anything but $realtime)
	       * just return the value as a double. */
	    if (is_int_func)  vp->value.real = double (simtime);
	      /* This is a call to $realtime to return a real value so
	       * scale this using the scaled real rules. */
	    else vp->value.real = vpip_time_to_scaled_real(schedule_simtime(),
						           rfp->scope);
	    break;

	  case vpiBinStrVal:
	    x = simtime;
	    num_bits = 8 * sizeof(vvp_time64_t);

	    rbuf[num_bits] = 0;
	    for (unsigned i = 1; i <= num_bits; i++) {
	      rbuf[num_bits-i] = x  & 1 ? '1' : '0';
	      x = x >> 1;
	    }

	    vp->value.str = rbuf;
	    break;

	  case vpiDecStrVal:
	    sprintf(rbuf, "%" TIME_FMT_U, simtime);
	    vp->value.str = rbuf;
	    break;

	  case vpiOctStrVal:
	    sprintf(rbuf, "%" TIME_FMT_O, simtime);
	    vp->value.str = rbuf;
	    break;

	  case vpiHexStrVal:
	    sprintf(rbuf, "%" TIME_FMT_X, simtime);
	    vp->value.str = rbuf;
	    break;

	  default:
	    fprintf(stderr, "vpi_time: unknown format: %d\n", (int)vp->format);
	    assert(0);
      }
}

static void timevar_get_ivalue(vpiHandle ref, s_vpi_value*vp)
{
      timevar_get_value(ref, vp, true, false);
}

static void timevar_get_svalue(vpiHandle ref, s_vpi_value*vp)
{
      timevar_get_value(ref, vp, true, true);
}

static void timevar_get_rvalue(vpiHandle ref, s_vpi_value*vp)
{
      timevar_get_value(ref, vp, false, false);
}

static const struct __vpirt vpip_system_time_rt = {
      vpiSysFuncCall,
      timevar_time_get,
      timevar_time_get_str,
      timevar_get_ivalue,
      0,
      timevar_handle,
      0
};

static const struct __vpirt vpip_system_stime_rt = {
      vpiSysFuncCall,
      timevar_stime_get,
      timevar_stime_get_str,
      timevar_get_svalue,
      0,
      timevar_handle,
      0
};

static const struct __vpirt vpip_system_simtime_rt = {
      vpiSysFuncCall,
      timevar_time_get,
      timevar_simtime_get_str,
      timevar_get_ivalue,
      0,
      timevar_handle,
      0
};

static const struct __vpirt vpip_system_realtime_rt = {
      vpiSysFuncCall,
      timevar_realtime_get,
      timevar_realtime_get_str,
      timevar_get_rvalue,
      0,
      timevar_handle,
      0
};

/*
 * Create a handle to represent a call to $time/$stime/$simtime. The
 * $time and $stime system functions return a value scaled to a scope,
 * and the $simtime returns the unscaled time.
 */
vpiHandle vpip_sim_time(struct __vpiScope*scope, bool is_stime)
{
      if (scope) {
	    if (is_stime) {
		  scope->scoped_stime.base.vpi_type = &vpip_system_stime_rt;
		  scope->scoped_stime.scope = scope;
		  return &scope->scoped_stime.base;
	    } else {
		  scope->scoped_time.base.vpi_type = &vpip_system_time_rt;
		  scope->scoped_time.scope = scope;
		  return &scope->scoped_time.base;
	    }
      } else {
	    global_simtime.base.vpi_type = &vpip_system_simtime_rt;
	    global_simtime.scope = 0;
	    return &global_simtime.base;
      }
}

vpiHandle vpip_sim_realtime(struct __vpiScope*scope)
{
      scope->scoped_realtime.base.vpi_type = &vpip_system_realtime_rt;
      scope->scoped_realtime.scope = scope;
      return &scope->scoped_realtime.base;
}

int vpip_get_time_precision(void)
{
      return vpi_time_precision;
}

void vpip_set_time_precision(int pre)
{
      vpi_time_precision = pre;
}
