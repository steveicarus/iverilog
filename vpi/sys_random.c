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
#ident "$Id: sys_random.c,v 1.3 2001/02/16 00:26:38 steve Exp $"
#endif

# include  <vpi_user.h>
# include  <assert.h>
# include  <stdlib.h>
# include  <math.h>
# include  <limits.h>

extern void sgenrand(unsigned long seed);
extern unsigned long genrand(void);


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
 * Implement the $random system function using the ``Mersenne
 * Twister'' random number generator MT19937.
 */
static int sys_random_calltf(char*name)
{
      s_vpi_value val;
      vpiHandle call_handle;
      vpiHandle argv;
      vpiHandle seed = 0;

      call_handle = vpi_handle(vpiSysTfCall, 0);
      assert(call_handle);

	/* Get the argument list and look for a seed. If it is there,
	   get the value and reseed the random number generator. */
      argv = vpi_iterate(vpiArgument, call_handle);
      if (argv) {
	    seed = vpi_scan(argv);
	    vpi_free_object(argv);

	    val.format = vpiIntVal;
	    vpi_get_value(seed, &val);
	    sgenrand(val.value.integer);
      }

      val.format = vpiIntVal;
      val.value.integer = genrand();

      vpi_put_value(call_handle, &val, 0, vpiNoDelay);

#if 0
      if (seed) {
	    val.format = vpiIntVal;
	    val.value.integer = i_seed;
	    vpi_put_value(seed, &val, 0, vpiNoDelay);
      }
#endif

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
 * Revision 1.3  2001/02/16 00:26:38  steve
 *  Use Mersenne Twister 19937 pseudo-random number generator
 *  for the $random system task, and support the seed paramter.
 *
 * Revision 1.2  2000/07/08 22:41:07  steve
 *  Add the dist_uniform function.
 *
 * Revision 1.1  2000/05/04 03:37:59  steve
 *  Add infrastructure for system functions, move
 *  $time to that structure and add $random.
 *
 */

