/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: sys_random.c,v 1.2 2000/07/08 22:41:07 steve Exp $"
#endif

# include  <vpi_user.h>
# include  <assert.h>
# include  <stdlib.h>
# include  <math.h>
# include  <limits.h>

#if 0
double uniform(long*seed, long start, long end)
{
      union u_s {
	    float s;
	    unsigned stemp;
      } u;

      double d = 0.00000011920928955078125;
      double a, b, c;

      if ((*seed) == 0)
	    *seed = 259341593;

      if (start >= end) {
	    a = 0.0;
	    b = 2147483647.0;

      } else {
	    a = (double) start;
	    b = (double) end;
      }

      *seed = 69069 * (*seed) + 1;
      u.stemp = *seed;

      u.stemp = (u.stemp >> 9) | 0x3f800000;

      c = (double) u.s;
      c = c + (c*d);
      c = ((b - a) * (c - 1.0)) + a;
      return c;
}
#endif

#if 0
static long rtl_dist_uniform(long*seed, long start, long end)
{
      double r;
      long i;

      if (start >= end)
	    return start;

      if (end != LONG_MAX) {
	    end += 1;
	    r = uniform(seed, start, end);
	    if (r >= 0)
		  i = (long) r;
	    else
		  i = (long) (r-1);

	    if (i < start) i = start;
	    if (i >= end) i = end - 1;

      } else if (start != LONG_MIN) {
	    start -= 1;
	    r = uniform(seed, start, end);
	    if (r >= 0)
		  i = (long) r;
	    else
		  i = (long) (r-1);

	    if (i <= start) i = start;
	    if (i >  end) i = end;

      } else {
	    r = (uniform(seed, start, end) + 2147483648.0) / 4294967295.0;
	    r = r * 4294967296.0 - 2147483648.0;
	    if (r >= 0)
		  i = (long) r;
	    else
		  i = (long) (r-1);
      }

      return i;
}
#else
static long rtl_dist_uniform(long*seed, long start, long end)
{
      if (start >= end)
	    return start;

      if ((start > LONG_MIN) || (end < LONG_MAX)) {
	    long range = end - start;
	    return start + random()%range;
      } else {
	    return random();
      }
}
#endif


static int sys_dist_uniform_calltf(char*name)
{
      s_vpi_value val;
      vpiHandle call_handle;
      vpiHandle argv;
      vpiHandle seed = 0, start, end;

      long i_seed, i_start, i_end;

      call_handle = vpi_handle(vpiSysTfCall, 0);
      assert(call_handle);

      argv = vpi_iterate(vpiArgument, call_handle);
      if (argv == 0) {
	    vpi_printf("ERROR: %s requires parameters "
		       "(seed, start, end)\n", name);
	    return 0;
      }

      seed = vpi_scan(argv);
      assert(seed);
      start = vpi_scan(argv);
      assert(start);
      end = vpi_scan(argv);
      assert(end);

      vpi_free_object(argv);

      val.format = vpiIntVal;
      vpi_get_value(seed, &val);
      i_seed = val.value.integer;

      vpi_get_value(start, &val);
      i_start = val.value.integer;

      vpi_get_value(end, &val);
      i_end = val.value.integer;

      val.format = vpiIntVal;
      val.value.integer = rtl_dist_uniform(&i_seed, i_start, i_end);
      vpi_put_value(call_handle, &val, 0, vpiNoDelay);

      val.format = vpiIntVal;
      val.value.integer = i_seed;
      vpi_put_value(seed, &val, 0, vpiNoDelay);

      return 0;
}

static int sys_dist_uniform_sizetf(char*x)
{
      return 32;
}

/*
 * Implement the $random system function. For now, ignore any
 * parameters and only produce a random number.
 */
static int sys_random_calltf(char*name)
{
      static long i_seed = 0;
      s_vpi_value val;
      vpiHandle call_handle;
      vpiHandle argv;
      vpiHandle seed = 0;

      call_handle = vpi_handle(vpiSysTfCall, 0);
      assert(call_handle);

      argv = vpi_iterate(vpiArgument, call_handle);
      if (argv) {
	    seed = vpi_scan(argv);
	    vpi_free_object(argv);

	    val.format = vpiIntVal;
	    vpi_get_value(seed, &val);
	    i_seed = val.value.integer;
      }

      val.format = vpiIntVal;
      val.value.integer = rtl_dist_uniform(&i_seed, LONG_MIN, LONG_MAX);

      vpi_put_value(call_handle, &val, 0, vpiNoDelay);

      if (seed) {
	    val.format = vpiIntVal;
	    val.value.integer = i_seed;
	    vpi_put_value(seed, &val, 0, vpiNoDelay);
      }

      return 0;
}

static int sys_random_sizetf(char*x)
{
      return 32;
}

void sys_random_register()
{
      s_vpi_systf_data tf_data;

      tf_data.type   = vpiSysFunc;
      tf_data.tfname = "$random";
      tf_data.calltf = sys_random_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf = sys_random_sizetf;
      tf_data.user_data = "$random";
      vpi_register_systf(&tf_data);

      tf_data.type   = vpiSysFunc;
      tf_data.tfname = "$dist_uniform";
      tf_data.calltf = sys_dist_uniform_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf = sys_dist_uniform_sizetf;
      tf_data.user_data = "$dist_uniform";
      vpi_register_systf(&tf_data);
}

/*
 * $Log: sys_random.c,v $
 * Revision 1.2  2000/07/08 22:41:07  steve
 *  Add the dist_uniform function.
 *
 * Revision 1.1  2000/05/04 03:37:59  steve
 *  Add infrastructure for system functions, move
 *  $time to that structure and add $random.
 *
 */

