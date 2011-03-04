/*
 * Copyright (c) 2000-2010 Stephen Williams (steve@icarus.com)
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

# include "sys_priv.h"

# include  <vpi_user.h>
# include  <assert.h>
# include  <stdlib.h>
# include  <math.h>
# include  <limits.h>

#if ULONG_MAX > 4294967295UL
# define UNIFORM_MAX INT_MAX
# define UNIFORM_MIN INT_MIN
#else
# define UNIFORM_MAX LONG_MAX
# define UNIFORM_MIN LONG_MIN
#endif

static double uniform(long *seed, long start, long end);
static long poisson(long *seed, long mean);

long rtl_dist_poisson(long*seed, long mean)
{
      long i;

      if (mean > 0) {
	    i = poisson(seed,mean);

      } else {
	    vpi_printf("WARNING: Poisson distribution must have "
		       "a positive mean\n");
	    i = 0;
      }

      return 0;
}

/* copied from IEEE1364-2001, with slight modifications for 64bit machines. */
long rtl_dist_uniform(long*seed, long start, long end)
{
	double r;
	long i;

	if (start >= end) return(start);

	  /* NOTE: The cast of r to i can overflow and generate
	     strange values, so cast to unsigned long
	     first. This eliminates the underflow and gets the
	     twos complement value. That in turn can be cast
	     to the long value that is expected. */

	if (end != UNIFORM_MAX)
	{
		end++;
		r = uniform( seed, start, end );
		if (r >= 0)
		{
			i = (unsigned long) r;
		}
		else
		{
			i = - ( (unsigned long) (-(r - 1)) );
		}
		if (i<start) i = start;
		if (i>=end) i = end-1;
	}
	else if (start!=UNIFORM_MIN)
	{
		start--;
		r = uniform( seed, start, end) + 1.0;
		if (r>=0)
		{
			i = (unsigned long) r;
		}
		else
		{
			i = - ( (unsigned long) (-(r - 1)) );
		}
		if (i<=start) i = start+1;
		if (i>end) i = end;
	}
	else
	{
		r = (uniform(seed,start,end)+2147483648.0)/4294967295.0;
		r = r*4294967296.0-2147483648.0;

		if (r>=0)
		{
			i = (unsigned long) r;
		}
		else
		{
			  /* At least some compilers will notice that (r-1)
			     is <0 when castling to unsigned long and
			     replace the result with a zero. This causes
			     much wrongness, so do the casting to the
			     positive version and invert it back. */
			i = - ( (unsigned long) (-(r - 1)) );

		}
	}

	return (i);
}

static double uniform(long *seed, long start, long end )
{
	double d = 0.00000011920928955078125;
	double a, b, c;
	unsigned long oldseed, newseed;

	oldseed = *seed;
	if (oldseed == 0)
	      oldseed = 259341593;

	if (start >= end) {
	      a = 0.0;
	      b = 2147483647.0;
	} else {
	      a = (double)start;
	      b = (double)end;
	}

	/* Original routine used signed arithmetic, and the (frequent)
	 * overflows trigger "Undefined Behavior" according to the
	 * C standard (both c89 and c99).  Using unsigned arithmetic
	 * forces a conforming C implementation to get the result
	 * that the IEEE-1364-2001 committee wants.
	 */
	newseed = 69069 * oldseed + 1;

	/* Emulate a 32-bit unsigned long, even if the native machine
	 * uses wider words.
	 */
#if ULONG_MAX > 4294967295UL
	newseed = newseed & 4294967295UL;
#endif
	*seed = newseed;


#if 0
	/* Cadence-donated conversion from unsigned int to double */
	{
		union { float s; unsigned stemp; } u;
		u.stemp = (newseed >> 9) | 0x3f800000;
		c = (double) u.s;
	}
#else
	/* Equivalent conversion without assuming IEEE 32-bit float */
	/* constant is 2^(-23) */
	c = 1.0 + (newseed >> 9) * 0.00000011920928955078125;
#endif


	c = c + (c*d);
	c = ((b - a) * (c - 1.0)) + a;

	return c;
}

/* copied from IEEE1364-2001, with slight modifications for 64bit machines. */
static long poisson(long*seed, long mean)
{
      long n;
      double p, q;

      n = 0;
      q = -(double)mean;
      p = exp(q);
      q = uniform(seed, 0, 1);
      while (p < q) {
	    n++;
	    q = uniform(seed,0,1) * q;
      }

      return n;
}

static PLI_INT32 sys_dist_poisson_calltf(char*name)
{
      s_vpi_value val;
      vpiHandle call_handle;
      vpiHandle argv;
      vpiHandle seed, mean;

      long i_seed, i_mean;

      call_handle = vpi_handle(vpiSysTfCall, 0);
      assert(call_handle);

	/* The presence of correct parameters should be checked at
	   compile time by the compiletf function. */
      argv = vpi_iterate(vpiArgument, call_handle);
      assert(argv);
      seed = vpi_scan(argv);
      assert(seed);
      mean = vpi_scan(argv);
      assert(mean);
      vpi_free_object(argv);

      val.format = vpiIntVal;
      vpi_get_value(seed, &val);
      i_seed = val.value.integer;

      vpi_get_value(mean, &val);
      i_mean = val.value.integer;

      val.format = vpiIntVal;
      val.value.integer = rtl_dist_poisson(&i_seed, i_mean);
      vpi_put_value(call_handle, &val, 0, vpiNoDelay);

      val.format = vpiIntVal;
      val.value.integer = i_seed;
      vpi_put_value(seed, &val, 0, vpiNoDelay);

      return 0;
}

static PLI_INT32 sys_dist_poisson_sizetf(char*x)
{
      return 32;
}

static PLI_INT32 sys_dist_uniform_calltf(char*name)
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

static PLI_INT32 sys_dist_uniform_sizetf(char*x)
{
      return 32;
}

static PLI_INT32 sys_random_calltf(char*name)
{
      s_vpi_value val;
      vpiHandle call_handle;
      vpiHandle argv;
      vpiHandle seed = 0;
      static long i_seed = 0;

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
	    i_seed = val.value.integer;
      }

      val.format = vpiIntVal;
      val.value.integer = rtl_dist_uniform(&i_seed, INT_MIN, INT_MAX);

      vpi_put_value(call_handle, &val, 0, vpiNoDelay);

        /* Send updated seed back to seed parameter. */
      if (seed) {
	    val.format = vpiIntVal;
	    val.value.integer = i_seed;
	    vpi_put_value(seed, &val, 0, vpiNoDelay);
      }

      return 0;
}

static PLI_INT32 sys_random_sizetf(char*x)
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
      tf_data.tfname = "$dist_poisson";
      tf_data.calltf = sys_dist_poisson_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf = sys_dist_poisson_sizetf;
      tf_data.user_data = "$dist_poisson";

      tf_data.type   = vpiSysFunc;
      tf_data.tfname = "$dist_uniform";
      tf_data.calltf = sys_dist_uniform_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf = sys_dist_uniform_sizetf;
      tf_data.user_data = "$dist_uniform";
      vpi_register_systf(&tf_data);
}
