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
# include "sys_random.h"

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
static double normal(long *seed, long mean, long deviation);
static double exponential(long *seed, long mean);
static long poisson(long *seed, long mean);
static double chi_square(long *seed, long deg_of_free);
static double t(long *seed, long deg_of_free);
static double erlangian(long *seed, long k, long mean);

long rtl_dist_chi_square(long *seed, long df)
{
      double r;
      long i;

      if (df > 0) {
            r = chi_square(seed, df);
            if (r >= 0) {
                  i = (long) (r + 0.5);
            } else {
                  r = -r;
                  i = (long) (r + 0.5);
                  i = -i;
            }
      } else {
            vpi_printf("WARNING: Chi_square distribution must have "
                       "a positive degree of freedom\n");
            i = 0;
      }

      return i;
}

long rtl_dist_erlang(long *seed, long k, long mean)
{
      double r;
      long i;

      if (k > 0) {
            r = erlangian(seed, k, mean);
            if (r >= 0) {
                  i = (long) (r + 0.5);
            } else {
                  r = -r;
                  i = (long) (r + 0.5);
                  i = -i;
            }
      } else {
            vpi_printf("WARNING: K-stage erlangian distribution must have "
                       "a positive k\n");
            i = 0;
      }

      return i;
}

long rtl_dist_exponential(long *seed, long mean)
{
      double r;
      long i;

      if (mean > 0) {
            r = exponential(seed, mean);
            if (r >= 0) {
                  i = (long) (r + 0.5);
            } else {
                  r = -r;
                  i = (long) (r + 0.5);
                  i = -i;
            }
      } else {
            vpi_printf("WARNING: Exponential distribution must have "
                       "a positive mean\n");
            i = 0;
      }

      return i;
}

long rtl_dist_normal(long *seed, long mean, long sd)
{
      double r;
      long i;

      r = normal(seed, mean, sd);
      if (r >= 0) {
            i = (long) (r + 0.5);
      } else {
            r = -r;
            i = (long) (r + 0.5);
            i = -i;
      }

      return i;
}

long rtl_dist_poisson(long *seed, long mean)
{
      long i;

      if (mean > 0) {
            i = poisson(seed, mean);
      } else {
            vpi_printf("WARNING: Poisson distribution must have "
                       "a positive mean\n");
            i = 0;
      }

      return i;
}

long rtl_dist_t(long *seed, long df)
{
      double r;
      long i;

      if (df > 0) {
            r = t(seed, df);
            if (r >= 0) {
                  i = (long) (r + 0.5);
            } else {
                  r = -r;
                  i = (long) (r + 0.5);
                  i = -i;
            }
      } else {
            vpi_printf("WARNING: t distribution must have "
                       "a positive degree of freedom\n");
            i = 0;
      }

      return i;
}

/* copied from IEEE1364-2001, with slight modifications for 64bit machines. */
long rtl_dist_uniform(long *seed, long start, long end)
{
      double r;
      long i;

      if (start >= end) return(start);

      /* NOTE: The cast of r to i can overflow and generate strange
         values, so cast to unsigned long first. This eliminates
         the underflow and gets the twos complement value. That in
         turn can be cast to the long value that is expected. */

      if (end != UNIFORM_MAX) {
            end++;
            r = uniform(seed, start, end);
            if (r >= 0) {
                  i = (unsigned long) r;
            } else {
	          i = - ( (unsigned long) (-(r - 1)) );
            }
            if (i < start) i = start;
            if (i >= end) i = end - 1;
      } else if (start != UNIFORM_MIN) {
            start--;
            r = uniform( seed, start, end) + 1.0;
            if (r >= 0) {
                  i = (unsigned long) r;
            } else {
	          i = - ( (unsigned long) (-(r - 1)) );
            }
            if (i <= start) i = start + 1;
            if (i > end) i = end;
      } else {
            r = (uniform(seed, start, end) + 2147483648.0) / 4294967295.0;
            r = r * 4294967296.0 - 2147483648.0;

            if (r >= 0) {
                  i = (unsigned long) r;
            } else {
	            /* At least some compilers will notice that (r-1)
		       is <0 when castling to unsigned long and
		       replace the result with a zero. This causes
		       much wrongness, so do the casting to the
		       positive version and invert it back. */
	          i = - ( (unsigned long) (-(r - 1)) );
            }
      }

      return i;
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

static double normal(long *seed, long mean, long deviation)
{
      double v1, v2, s;

      s = 1.0;
      while ((s >= 1.0) || (s == 0.0)) {
            v1 = uniform(seed, -1, 1);
            v2 = uniform(seed, -1, 1);
            s = v1 * v1 + v2 * v2;
      }
      s = v1 * sqrt(-2.0 * log(s) / s);
      v1 = (double) deviation;
      v2 = (double) mean;

      return s * v1 + v2;
}

static double exponential(long *seed, long mean)
{
      double n;

      n = uniform(seed, 0, 1);
      if (n != 0.0) {
            n = -log(n) * mean;
      }

      return n;
}

static long poisson(long *seed, long mean)
{
      long n;
      double p, q;

      n = 0;
      q = -(double) mean;
      p = exp(q);
      q = uniform(seed, 0, 1);
      while (p < q) {
            n++;
            q = uniform(seed, 0, 1) * q;
      }

      return n;
}

static double chi_square(long *seed, long deg_of_free)
{
      double x;
      long k;

      if (deg_of_free % 2) {
            x = normal(seed, 0, 1);
            x = x * x;
      } else {
            x = 0.0;
      }
      for (k = 2; k <= deg_of_free; k = k + 2) {
            x = x + 2 * exponential(seed, 1);
      }

      return x;
}

static double t( long *seed, long deg_of_free)
{
      double x, chi2, dv, root;

      chi2 = chi_square(seed, deg_of_free);
      dv = chi2 / (double) deg_of_free;
      root = sqrt(dv);
      x = normal(seed, 0, 1) / root;

      return x;
}

static double erlangian(long *seed, long k, long mean)
{
      double x, a, b;
      long i;

      x = 1.0;
      for (i = 1; i <= k; i++) {
            x = x * uniform(seed, 0, 1);
      }
      a = (double) mean;
      b = (double) k;
      x = -a * log(x) / b;

      return x;
}

/* A seed can only be an integer/time variable or a register. */
static unsigned is_seed_obj(vpiHandle obj, vpiHandle callh, char *name)
{
      unsigned rtn = 0;

      assert(obj);

      switch (vpi_get(vpiType, obj)) {
	    case vpiTimeVar:
	    case vpiIntegerVar:
	    case vpiReg:
		  rtn = 1;
		  break;
	    default:
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s's seed must be an integer/time"
		             " variable or a register.\n", name);
		  vpi_control(vpiFinish, 1);
      }

      return rtn;
}

static PLI_INT32 sys_rand_two_args_compiletf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle seed, arg2;

      /* Check that there are arguments. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires two arguments.\n", name);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      /* Check that there are at least two arguments. */
      seed = vpi_scan(argv);  /* This should never be zero. */
      arg2 = vpi_scan(argv);
      if (arg2 == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires two arguments.\n", name);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      /* The seed must be a time/integer variable or a register. */
      if (! is_seed_obj(seed, callh, name)) return 0;

      /* The second argument must be numeric. */
      if (! is_numeric_obj(arg2)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s second argument must be numeric.\n", name);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      /* Check that there is at most two arguments. */
      check_for_extra_args(argv, callh, name, "two arguments", 0);

      return 0;
}

PLI_INT32 sys_rand_three_args_compiletf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle seed, arg2, arg3;

      /* Check that there are arguments. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires three arguments.\n", name);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      /* Check that there are at least three arguments. */
      seed = vpi_scan(argv);  /* This should never be zero. */
      arg2 = vpi_scan(argv);
      if (arg2) {
            arg3 = vpi_scan(argv);
      } else {
            arg3 = 0;
      }
      if (arg2 == 0 || arg3 == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires three arguments.\n", name);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      /* The seed must be a time/integer variable or a register. */
      if (! is_seed_obj(seed, callh, name)) return 0;

      /* The second argument must be numeric. */
      if (! is_numeric_obj(arg2)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s second argument must be numeric.\n", name);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      /* The third argument must be numeric. */
      if (! is_numeric_obj(arg3)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s third argument must be numeric.\n", name);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      /* Check that there is at most three arguments. */
      check_for_extra_args(argv, callh, name, "three arguments", 0);

      return 0;
}

PLI_INT32 sys_random_compiletf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);

      /* The seed is optional. */
      if (argv == 0) return 0;

      /* The seed must be a time/integer variable or a register. */
      if (! is_seed_obj(vpi_scan(argv), callh, name)) return 0;

      /* Check that there no extra arguments. */
      check_for_extra_args(argv, callh, name, "one argument", 1);

      return 0;
}

static PLI_INT32 sys_random_calltf(PLI_BYTE8 *name)
{
      vpiHandle callh, argv, seed = 0;
      s_vpi_value val;
      static long i_seed = 0;

      /* Get the argument list and look for a seed. If it is there,
         get the value and reseed the random number generator. */
      callh = vpi_handle(vpiSysTfCall, 0);
      argv = vpi_iterate(vpiArgument, callh);
      val.format = vpiIntVal;
      if (argv) {
            seed = vpi_scan(argv);
            vpi_free_object(argv);
            vpi_get_value(seed, &val);
            i_seed = val.value.integer;
      }

      /* Calculate and return the result. */
      val.value.integer = rtl_dist_uniform(&i_seed, INT_MIN, INT_MAX);
      vpi_put_value(callh, &val, 0, vpiNoDelay);

      /* If it exists send the updated seed back to seed parameter. */
      if (seed) {
            val.value.integer = i_seed;
            vpi_put_value(seed, &val, 0, vpiNoDelay);
      }

      return 0;
}

/* From System Verilog 3.1a. */
static PLI_INT32 sys_urandom_range_compiletf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;

      /* Check that there are arguments. */
      if (argv == 0) {
            vpi_printf("ERROR: %s requires two arguments.\n", name);
            vpi_control(vpiFinish, 1);
            return 0;
      }

      /* Check that there are at least two arguments. */
      arg = vpi_scan(argv);  /* This should never be zero. */
      assert(arg);
      arg = vpi_scan(argv);
      if (arg == 0) {
            vpi_printf("ERROR: %s requires two arguments.\n", name);
            vpi_control(vpiFinish, 1);
            return 0;
      }

      /* These functions takes at most two argument. */
      arg = vpi_scan(argv);
      if (arg != 0) {
            vpi_printf("ERROR: %s takes at most two argument.\n", name);
            vpi_control(vpiFinish, 1);
            return 0;
      }

      /* vpi_scan returning 0 (NULL) has already freed argv. */
      return 0;
}

/* From System Verilog 3.1a. */
static unsigned long urandom(long *seed, unsigned long max, unsigned long min)
{
      static long i_seed = 0;
      unsigned long result;
      long max_i, min_i;

      max_i =  max + INT_MIN;
      min_i =  min + INT_MIN;
      if (seed != 0) i_seed = *seed;
      result = rtl_dist_uniform(&i_seed, min_i, max_i) - INT_MIN;
      if (seed != 0) *seed = i_seed;
      return result;
}

/* From System Verilog 3.1a. */
static PLI_INT32 sys_urandom_calltf(PLI_BYTE8 *name)
{
      vpiHandle callh, argv, seed = 0;
      s_vpi_value val;
      long i_seed;

      /* Get the argument list and look for a seed. If it is there,
         get the value and reseed the random number generator. */
      callh = vpi_handle(vpiSysTfCall, 0);
      argv = vpi_iterate(vpiArgument, callh);
      val.format = vpiIntVal;
      if (argv) {
            seed = vpi_scan(argv);
            vpi_free_object(argv);
            vpi_get_value(seed, &val);
            i_seed = val.value.integer;
      }

      /* Calculate and return the result. */
      if (seed) {
            val.value.integer = urandom(&i_seed, UINT_MAX, 0);
      } else {
            val.value.integer = urandom(0, UINT_MAX, 0);
      }
      vpi_put_value(callh, &val, 0, vpiNoDelay);

      /* If it exists send the updated seed back to seed parameter. */
      if (seed) {
            val.value.integer = i_seed;
            vpi_put_value(seed, &val, 0, vpiNoDelay);
      }

      return 0;
}

/* From System Verilog 3.1a. */
static PLI_INT32 sys_urandom_range_calltf(PLI_BYTE8 *name)
{
      vpiHandle callh, argv, maxval, minval;
      s_vpi_value val;
      unsigned long i_maxval, i_minval, tmp;

      /* Get the argument handles and convert them. */
      callh = vpi_handle(vpiSysTfCall, 0);
      argv = vpi_iterate(vpiArgument, callh);
      maxval = vpi_scan(argv);
      minval = vpi_scan(argv);

      val.format = vpiIntVal;
      vpi_get_value(maxval, &val);
      i_maxval = val.value.integer;

      vpi_get_value(minval, &val);
      i_minval = val.value.integer;

      /* Swap the two arguments if they are out of order. */
      if (i_minval > i_maxval) {
            tmp = i_minval;
            i_minval = i_maxval;
            i_maxval = tmp;
      }

      /* Calculate and return the result. */
      val.value.integer = urandom(0, i_maxval, i_minval);
      vpi_put_value(callh, &val, 0, vpiNoDelay);
      vpi_free_object(argv);
      return 0;
}

static PLI_INT32 sys_dist_uniform_calltf(PLI_BYTE8 *name)
{
      vpiHandle callh, argv, seed, start, end;
      s_vpi_value val;
      long i_seed, i_start, i_end;

      /* Get the argument handles and convert them. */
      callh = vpi_handle(vpiSysTfCall, 0);
      argv = vpi_iterate(vpiArgument, callh);
      seed = vpi_scan(argv);
      start = vpi_scan(argv);
      end = vpi_scan(argv);

      val.format = vpiIntVal;
      vpi_get_value(seed, &val);
      i_seed = val.value.integer;

      vpi_get_value(start, &val);
      i_start = val.value.integer;

      vpi_get_value(end, &val);
      i_end = val.value.integer;

      /* Calculate and return the result. */
      val.value.integer = rtl_dist_uniform(&i_seed, i_start, i_end);
      vpi_put_value(callh, &val, 0, vpiNoDelay);

      /* Return the seed. */
      val.value.integer = i_seed;
      vpi_put_value(seed, &val, 0, vpiNoDelay);

      vpi_free_object(argv);
      return 0;
}

static PLI_INT32 sys_dist_normal_calltf(PLI_BYTE8 *name)
{
      vpiHandle callh, argv, seed, mean, sd;
      s_vpi_value val;
      long i_seed, i_mean, i_sd;

      /* Get the argument handles and convert them. */
      callh = vpi_handle(vpiSysTfCall, 0);
      argv = vpi_iterate(vpiArgument, callh);
      seed = vpi_scan(argv);
      mean = vpi_scan(argv);
      sd = vpi_scan(argv);

      val.format = vpiIntVal;
      vpi_get_value(seed, &val);
      i_seed = val.value.integer;

      vpi_get_value(mean, &val);
      i_mean = val.value.integer;

      vpi_get_value(sd, &val);
      i_sd = val.value.integer;

      /* Calculate and return the result. */
      val.value.integer = rtl_dist_normal(&i_seed, i_mean, i_sd);
      vpi_put_value(callh, &val, 0, vpiNoDelay);

      /* Return the seed. */
      val.value.integer = i_seed;
      vpi_put_value(seed, &val, 0, vpiNoDelay);

      vpi_free_object(argv);
      return 0;
}

static PLI_INT32 sys_dist_exponential_calltf(PLI_BYTE8 *name)
{
      vpiHandle callh, argv, seed, mean;
      s_vpi_value val;
      long i_seed, i_mean;

      /* Get the argument handles and convert them. */
      callh = vpi_handle(vpiSysTfCall, 0);
      argv = vpi_iterate(vpiArgument, callh);
      seed = vpi_scan(argv);
      mean = vpi_scan(argv);

      val.format = vpiIntVal;
      vpi_get_value(seed, &val);
      i_seed = val.value.integer;

      vpi_get_value(mean, &val);
      i_mean = val.value.integer;

      /* Calculate and return the result. */
      val.value.integer = rtl_dist_exponential(&i_seed, i_mean);
      vpi_put_value(callh, &val, 0, vpiNoDelay);

      /* Return the seed. */
      val.value.integer = i_seed;
      vpi_put_value(seed, &val, 0, vpiNoDelay);

      vpi_free_object(argv);
      return 0;
}

static PLI_INT32 sys_dist_poisson_calltf(PLI_BYTE8 *name)
{
      vpiHandle callh, argv, seed, mean;
      s_vpi_value val;
      long i_seed, i_mean;

      /* Get the argument handles and convert them. */
      callh = vpi_handle(vpiSysTfCall, 0);
      argv = vpi_iterate(vpiArgument, callh);
      seed = vpi_scan(argv);
      mean = vpi_scan(argv);

      val.format = vpiIntVal;
      vpi_get_value(seed, &val);
      i_seed = val.value.integer;

      vpi_get_value(mean, &val);
      i_mean = val.value.integer;

      /* Calculate and return the result. */
      val.value.integer = rtl_dist_poisson(&i_seed, i_mean);
      vpi_put_value(callh, &val, 0, vpiNoDelay);

      /* Return the seed. */
      val.value.integer = i_seed;
      vpi_put_value(seed, &val, 0, vpiNoDelay);

      vpi_free_object(argv);
      return 0;
}

static PLI_INT32 sys_dist_chi_square_calltf(PLI_BYTE8 *name)
{
      vpiHandle callh, argv, seed, df;
      s_vpi_value val;
      long i_seed, i_df;

      /* Get the argument handles and convert them. */
      callh = vpi_handle(vpiSysTfCall, 0);
      argv = vpi_iterate(vpiArgument, callh);
      seed = vpi_scan(argv);
      df = vpi_scan(argv);

      val.format = vpiIntVal;
      vpi_get_value(seed, &val);
      i_seed = val.value.integer;

      vpi_get_value(df, &val);
      i_df = val.value.integer;

      /* Calculate and return the result. */
      val.value.integer = rtl_dist_chi_square(&i_seed, i_df);
      vpi_put_value(callh, &val, 0, vpiNoDelay);

      /* Return the seed. */
      val.value.integer = i_seed;
      vpi_put_value(seed, &val, 0, vpiNoDelay);

      vpi_free_object(argv);
      return 0;
}

static PLI_INT32 sys_dist_t_calltf(PLI_BYTE8 *name)
{
      vpiHandle callh, argv, seed, df;
      s_vpi_value val;
      long i_seed, i_df;

      /* Get the argument handles and convert them. */
      callh = vpi_handle(vpiSysTfCall, 0);
      argv = vpi_iterate(vpiArgument, callh);
      seed = vpi_scan(argv);
      df = vpi_scan(argv);

      val.format = vpiIntVal;
      vpi_get_value(seed, &val);
      i_seed = val.value.integer;

      vpi_get_value(df, &val);
      i_df = val.value.integer;

      /* Calculate and return the result. */
      val.value.integer = rtl_dist_t(&i_seed, i_df);
      vpi_put_value(callh, &val, 0, vpiNoDelay);

      /* Return the seed. */
      val.value.integer = i_seed;
      vpi_put_value(seed, &val, 0, vpiNoDelay);

      vpi_free_object(argv);
      return 0;
}

static PLI_INT32 sys_dist_erlang_calltf(PLI_BYTE8 *name)
{
      vpiHandle callh, argv, seed, k, mean;
      s_vpi_value val;
      long i_seed, i_k, i_mean;

      /* Get the argument handles and convert them. */
      callh = vpi_handle(vpiSysTfCall, 0);
      argv = vpi_iterate(vpiArgument, callh);
      seed = vpi_scan(argv);
      k = vpi_scan(argv);
      mean = vpi_scan(argv);

      val.format = vpiIntVal;
      vpi_get_value(seed, &val);
      i_seed = val.value.integer;

      vpi_get_value(k, &val);
      i_k = val.value.integer;

      vpi_get_value(mean, &val);
      i_mean = val.value.integer;

      /* Calculate and return the result. */
      val.value.integer = rtl_dist_erlang(&i_seed, i_k, i_mean);
      vpi_put_value(callh, &val, 0, vpiNoDelay);

      /* Return the seed. */
      val.value.integer = i_seed;
      vpi_put_value(seed, &val, 0, vpiNoDelay);

      vpi_free_object(argv);
      return 0;
}

static PLI_INT32 sys_rand_func_sizetf(PLI_BYTE8 *x)
{
      return 32;
}

void sys_random_register()
{
      s_vpi_systf_data tf_data;

      tf_data.type = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname = "$random";
      tf_data.calltf = sys_random_calltf;
      tf_data.compiletf = sys_random_compiletf;
      tf_data.sizetf = sys_rand_func_sizetf;
      tf_data.user_data = "$random";
      vpi_register_systf(&tf_data);

      /* From System Verilog 3.1a. */
      tf_data.type = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncSized;
      tf_data.tfname = "$urandom";
      tf_data.calltf = sys_urandom_calltf;
      tf_data.compiletf = sys_random_compiletf;
      tf_data.sizetf = sys_rand_func_sizetf;
      tf_data.user_data = "$urandom";
      vpi_register_systf(&tf_data);

      /* From System Verilog 3.1a. */
      tf_data.type = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncSized;
      tf_data.tfname = "$urandom_range";
      tf_data.calltf = sys_urandom_range_calltf;
      tf_data.compiletf = sys_urandom_range_compiletf;
      tf_data.sizetf = sys_rand_func_sizetf;
      tf_data.user_data = "$urandom_range";
      vpi_register_systf(&tf_data);

      tf_data.type = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname = "$dist_uniform";
      tf_data.calltf = sys_dist_uniform_calltf;
      tf_data.compiletf = sys_rand_three_args_compiletf;
      tf_data.sizetf = sys_rand_func_sizetf;
      tf_data.user_data = "$dist_uniform";
      vpi_register_systf(&tf_data);

      tf_data.type = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname = "$dist_normal";
      tf_data.calltf = sys_dist_normal_calltf;
      tf_data.compiletf = sys_rand_three_args_compiletf;
      tf_data.sizetf = sys_rand_func_sizetf;
      tf_data.user_data = "$dist_normal";
      vpi_register_systf(&tf_data);

      tf_data.type = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname = "$dist_exponential";
      tf_data.calltf = sys_dist_exponential_calltf;
      tf_data.compiletf = sys_rand_two_args_compiletf;
      tf_data.sizetf = sys_rand_func_sizetf;
      tf_data.user_data = "$dist_exponential";
      vpi_register_systf(&tf_data);

      tf_data.type = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname = "$dist_poisson";
      tf_data.calltf = sys_dist_poisson_calltf;
      tf_data.compiletf = sys_rand_two_args_compiletf;
      tf_data.sizetf = sys_rand_func_sizetf;
      tf_data.user_data = "$dist_poisson";
      vpi_register_systf(&tf_data);

      tf_data.type = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname = "$dist_chi_square";
      tf_data.calltf = sys_dist_chi_square_calltf;
      tf_data.compiletf = sys_rand_two_args_compiletf;
      tf_data.sizetf = sys_rand_func_sizetf;
      tf_data.user_data = "$dist_chi_square";
      vpi_register_systf(&tf_data);

      tf_data.type = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname = "$dist_t";
      tf_data.calltf = sys_dist_t_calltf;
      tf_data.compiletf = sys_rand_two_args_compiletf;
      tf_data.sizetf = sys_rand_func_sizetf;
      tf_data.user_data = "$dist_t";
      vpi_register_systf(&tf_data);

      tf_data.type = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname = "$dist_erlang";
      tf_data.calltf = sys_dist_erlang_calltf;
      tf_data.compiletf = sys_rand_three_args_compiletf;
      tf_data.sizetf = sys_rand_func_sizetf;
      tf_data.user_data = "$dist_erlang";
      vpi_register_systf(&tf_data);
}
