/*
 * Copyright (c) 2000-2022 Stephen Williams (steve@icarus.com)
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

# include "sys_priv.h"
# include "sys_random.h"

# include  <assert.h>
# include  <stdlib.h>
# include  <math.h>
# include  <limits.h>

/*
 * The following code is largely copied from the IEEE standard (section 17.9.3
 * in IEEE 1364-2005, Annex N in IEEE 1800-2017). The code in the IEEE standard
 * predates the widespread availability of 64-bit processors, so assumes 'long'
 * is a 32 bit value (https://accellera.mantishub.io/view.php?id=1136). So we
 * replace 'long' with 'int32_t' to ensure correct and consistent behaviour
 * regardless of how the code is built.
 */

static double uniform(int32_t *seed, int32_t start, int32_t end);
static double normal(int32_t *seed, int32_t mean, int32_t deviation);
static double exponential(int32_t *seed, int32_t mean);
static int32_t poisson(int32_t *seed, int32_t mean);
static double chi_square(int32_t *seed, int32_t deg_of_free);
static double t(int32_t *seed, int32_t deg_of_free);
static double erlangian(int32_t *seed, int32_t k, int32_t mean);

static int32_t rtl_dist_chi_square(int32_t *seed, int32_t df)
{
      double r;
      int32_t i;

      if (df > 0) {
            r = chi_square(seed, df);
            if (r >= 0) {
                  i = (int32_t) (r + 0.5);
            } else {
                  r = -r;
                  i = (int32_t) (r + 0.5);
                  i = -i;
            }
      } else {
            vpi_printf("WARNING: Chi_square distribution must have "
                       "a positive degree of freedom\n");
            i = 0;
      }

      return i;
}

static int32_t rtl_dist_erlang(int32_t *seed, int32_t k, int32_t mean)
{
      double r;
      int32_t i;

      if (k > 0) {
            r = erlangian(seed, k, mean);
            if (r >= 0) {
                  i = (int32_t) (r + 0.5);
            } else {
                  r = -r;
                  i = (int32_t) (r + 0.5);
                  i = -i;
            }
      } else {
            vpi_printf("WARNING: K-stage erlangian distribution must have "
                       "a positive k\n");
            i = 0;
      }

      return i;
}

static int32_t rtl_dist_exponential(int32_t *seed, int32_t mean)
{
      double r;
      int32_t i;

      if (mean > 0) {
            r = exponential(seed, mean);
            if (r >= 0) {
                  i = (int32_t) (r + 0.5);
            } else {
                  r = -r;
                  i = (int32_t) (r + 0.5);
                  i = -i;
            }
      } else {
            vpi_printf("WARNING: Exponential distribution must have "
                       "a positive mean\n");
            i = 0;
      }

      return i;
}

static int32_t rtl_dist_normal(int32_t *seed, int32_t mean, int32_t sd)
{
      double r;
      int32_t i;

      r = normal(seed, mean, sd);
      if (r >= 0) {
            i = (int32_t) (r + 0.5);
      } else {
            r = -r;
            i = (int32_t) (r + 0.5);
            i = -i;
      }

      return i;
}

static int32_t rtl_dist_poisson(int32_t *seed, int32_t mean)
{
      int32_t i;

      if (mean > 0) {
            i = poisson(seed, mean);
      } else {
            vpi_printf("WARNING: Poisson distribution must have "
                       "a positive mean\n");
            i = 0;
      }

      return i;
}

static int32_t rtl_dist_t(int32_t *seed, int32_t df)
{
      double r;
      int32_t i;

      if (df > 0) {
            r = t(seed, df);
            if (r >= 0) {
                  i = (int32_t) (r + 0.5);
            } else {
                  r = -r;
                  i = (int32_t) (r + 0.5);
                  i = -i;
            }
      } else {
            vpi_printf("WARNING: t distribution must have "
                       "a positive degree of freedom\n");
            i = 0;
      }

      return i;
}

static int32_t rtl_dist_uniform(int32_t *seed, int32_t start, int32_t end)
{
      double r;
      int32_t i;

      if (start >= end) return start;

      if (end != INT32_MAX) {
            end++;
            r = uniform(seed, start, end);
            if (r >= 0) {
                  i = (int32_t) r;
            } else {
	          i = (int32_t) (r - 1);
            }
            if (i < start) i = start;
            if (i >= end) i = end - 1;
      } else if (start != INT32_MIN) {
            start--;
            r = uniform(seed, start, end) + 1.0;
            if (r >= 0) {
                  i = (int32_t) r;
            } else {
	          i = (int32_t) (r - 1);
            }
            if (i <= start) i = start + 1;
            if (i > end) i = end;
      } else {
            r = (uniform(seed, start, end) + 2147483648.0) / 4294967295.0;
            r = r * 4294967296.0 - 2147483648.0;

            if (r >= 0) {
                  i = (int32_t) r;
            } else {
	          i = (int32_t) (r - 1);
            }
      }

      return i;
}

static double uniform(int32_t *seed, int32_t start, int32_t end)
{
      double d = 0.00000011920928955078125;
      double a, b, c;
      uint32_t oldseed, newseed;

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

static double normal(int32_t *seed, int32_t mean, int32_t deviation)
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

static double exponential(int32_t *seed, int32_t mean)
{
      double n;

      n = uniform(seed, 0, 1);
      if (n != 0.0) {
            n = -log(n) * mean;
      }

      return n;
}

static int32_t poisson(int32_t *seed, int32_t mean)
{
      int32_t n;
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

static double chi_square(int32_t *seed, int32_t deg_of_free)
{
      double x;
      int32_t k;

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

static double t( int32_t *seed, int32_t deg_of_free)
{
      double x, chi2, dv, root;

      chi2 = chi_square(seed, deg_of_free);
      dv = chi2 / (double) deg_of_free;
      root = sqrt(dv);
      x = normal(seed, 0, 1) / root;

      return x;
}

static double erlangian(int32_t *seed, int32_t k, int32_t mean)
{
      double x, a, b;
      int32_t i;

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
static unsigned is_seed_obj(vpiHandle obj, vpiHandle callh, const char *name)
{
      unsigned rtn = 0;

      assert(obj);

      switch (vpi_get(vpiType, obj)) {
	    case vpiTimeVar:
	    case vpiIntegerVar:
	    case vpiIntVar:
	    case vpiLongIntVar:
		  rtn = 1;
		  break;
	    case vpiBitVar:
	    case vpiReg:
		  if (vpi_get(vpiSize, obj) < 32) {
			vpi_printf("Error: %s:%d: ",
			           vpi_get_str(vpiFile, callh),
			           (int)vpi_get(vpiLineNo, callh));
			vpi_printf("%s's seed variable is less than 32 bits "
			           " (%d).\n", name,
			           (int)vpi_get(vpiSize, obj));
			vpip_set_return_value(1);
			vpi_control(vpiFinish, 1);
		  } else rtn = 1;
		  break;
	    default:
		  vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
		             (int)vpi_get(vpiLineNo, callh));
		  vpi_printf("%s's seed must be an integer/time"
		             " variable or a register.\n", name);
		  vpip_set_return_value(1);
		  vpi_control(vpiFinish, 1);
      }

      return rtn;
}

static PLI_INT32 sys_rand_two_args_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle seed, arg2;

      /* Check that there are arguments. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires two arguments.\n", name);
	    vpip_set_return_value(1);
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
	    vpip_set_return_value(1);
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
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      /* Check that there is at most two arguments. */
      check_for_extra_args(argv, callh, name, "two arguments", 0);

      return 0;
}

PLI_INT32 sys_rand_three_args_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle seed, arg2, arg3;

      /* Check that there are arguments. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires three arguments.\n", name);
	    vpip_set_return_value(1);
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
	    vpip_set_return_value(1);
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
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      /* The third argument must be numeric. */
      if (! is_numeric_obj(arg3)) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s third argument must be numeric.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      /* Check that there is at most three arguments. */
      check_for_extra_args(argv, callh, name, "three arguments", 0);

      return 0;
}

PLI_INT32 sys_random_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
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

static PLI_INT32 sys_random_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh, argv, seed = 0;
      s_vpi_value val;
      static int32_t i_seed = 0;
      int32_t a_seed;

      (void)name; /* Parameter is not used. */

      /* Get the argument list and look for a seed. If it is there,
         get the value and reseed the random number generator. */
      callh = vpi_handle(vpiSysTfCall, 0);
      argv = vpi_iterate(vpiArgument, callh);
      val.format = vpiIntVal;
      if (argv) {
            seed = vpi_scan(argv);
            vpi_free_object(argv);
            vpi_get_value(seed, &val);
            a_seed = val.value.integer;
      } else a_seed = i_seed;

      /* Calculate and return the result. */
      val.value.integer = rtl_dist_uniform(&a_seed, INT_MIN, INT_MAX);
      vpi_put_value(callh, &val, 0, vpiNoDelay);

      /* If it exists send the updated seed back to seed parameter. */
      if (seed) {
            val.value.integer = a_seed;
            vpi_put_value(seed, &val, 0, vpiNoDelay);
      } else i_seed = a_seed;

      return 0;
}

/* From SystemVerilog. */
static PLI_INT32 sys_urandom_range_compiletf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle arg;

      /* Check that there are arguments. */
      if (argv == 0) {
            vpi_printf("ERROR: %s requires one or two arguments.\n", name);
	    vpip_set_return_value(1);
            vpi_control(vpiFinish, 1);
            return 0;
      }

      /* Check that there is at least one argument. */
      arg = vpi_scan(argv);  /* This should never be zero. */
      assert(arg);
      arg = vpi_scan(argv);
      /* Is this a single argument function call? */
      if (arg == 0) return 0;

      /* These functions takes at most two argument. */
      arg = vpi_scan(argv);
      if (arg != 0) {
            vpi_printf("ERROR: %s takes at most two argument.\n", name);
	    vpip_set_return_value(1);
            vpi_control(vpiFinish, 1);
            return 0;
      }

      /* vpi_scan returning 0 (NULL) has already freed argv. */
      return 0;
}

/* From SystemVerilog. */
static uint32_t urandom(int32_t *seed, uint32_t max, uint32_t min)
{
      static int32_t i_seed = 0;
      int32_t max_i, min_i;
      uint32_t result;

      max_i = max + INT32_MIN;
      min_i = min + INT32_MIN;
      if (seed != 0) i_seed = *seed;
      result = (uint32_t)rtl_dist_uniform(&i_seed, min_i, max_i) - INT32_MIN;
      if (seed != 0) *seed = i_seed;
      return result;
}

/* From SystemVerilog. */
static PLI_INT32 sys_urandom_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh, argv, seed = 0;
      s_vpi_value val;
      int32_t i_seed;

      (void)name; /* Parameter is not used. */

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
            val.value.integer = urandom(&i_seed, UINT32_MAX, 0);
      } else {
            val.value.integer = urandom(0, UINT32_MAX, 0);
      }
      vpi_put_value(callh, &val, 0, vpiNoDelay);

      /* If it exists send the updated seed back to seed parameter. */
      if (seed) {
            val.value.integer = i_seed;
            vpi_put_value(seed, &val, 0, vpiNoDelay);
      }

      return 0;
}

/* From SystemVerilog. */
static PLI_INT32 sys_urandom_range_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh, argv, maxval, minval;
      s_vpi_value val;
      uint32_t i_maxval, i_minval;

      (void)name; /* Parameter is not used. */

      /* Get the argument handles and convert them. */
      callh = vpi_handle(vpiSysTfCall, 0);
      argv = vpi_iterate(vpiArgument, callh);
      maxval = vpi_scan(argv);
      minval = vpi_scan(argv);

      val.format = vpiIntVal;
      vpi_get_value(maxval, &val);
      i_maxval = val.value.integer;

      /* Is this a two or one argument function call? */
      if (minval) {
	    vpi_get_value(minval, &val);
	    i_minval = val.value.integer;
	    vpi_free_object(argv);
      } else {
	    i_minval = 0;
      }

      /* Swap the two arguments if they are out of order. */
      if (i_minval > i_maxval) {
            uint32_t tmp = i_minval;
            i_minval = i_maxval;
            i_maxval = tmp;
      }

      /* Calculate and return the result. */
      val.value.integer = urandom(0, i_maxval, i_minval);
      vpi_put_value(callh, &val, 0, vpiNoDelay);
      return 0;
}

static PLI_INT32 sys_dist_uniform_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh, argv, seed, start, end;
      s_vpi_value val;
      int32_t i_seed, i_start, i_end;

      (void)name; /* Parameter is not used. */

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

static PLI_INT32 sys_dist_normal_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh, argv, seed, mean, sd;
      s_vpi_value val;
      int32_t i_seed, i_mean, i_sd;

      (void)name; /* Parameter is not used. */

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

static PLI_INT32 sys_dist_exponential_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh, argv, seed, mean;
      s_vpi_value val;
      int32_t i_seed, i_mean;

      (void)name; /* Parameter is not used. */

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

static PLI_INT32 sys_dist_poisson_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh, argv, seed, mean;
      s_vpi_value val;
      int32_t i_seed, i_mean;

      (void)name; /* Parameter is not used. */

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

static PLI_INT32 sys_dist_chi_square_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh, argv, seed, df;
      s_vpi_value val;
      int32_t i_seed, i_df;

      (void)name; /* Parameter is not used. */

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

static PLI_INT32 sys_dist_t_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh, argv, seed, df;
      s_vpi_value val;
      int32_t i_seed, i_df;

      (void)name; /* Parameter is not used. */

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

static PLI_INT32 sys_dist_erlang_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh, argv, seed, k, mean;
      s_vpi_value val;
      int32_t i_seed, i_k, i_mean;

      (void)name; /* Parameter is not used. */

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

static PLI_INT32 sys_rand_func_sizetf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      (void)name; /* Parameter is not used. */
      return 32;
}

void sys_random_register(void)
{
      s_vpi_systf_data tf_data;
      vpiHandle res;

      tf_data.type = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname = "$random";
      tf_data.calltf = sys_random_calltf;
      tf_data.compiletf = sys_random_compiletf;
      tf_data.sizetf = sys_rand_func_sizetf;
      tf_data.user_data = "$random";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /* From SystemVerilog. */
      tf_data.type = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncSized;
      tf_data.tfname = "$urandom";
      tf_data.calltf = sys_urandom_calltf;
      tf_data.compiletf = sys_random_compiletf;
      tf_data.sizetf = sys_rand_func_sizetf;
      tf_data.user_data = "$urandom";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      /* From SystemVerilog. */
      tf_data.type = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncSized;
      tf_data.tfname = "$urandom_range";
      tf_data.calltf = sys_urandom_range_calltf;
      tf_data.compiletf = sys_urandom_range_compiletf;
      tf_data.sizetf = sys_rand_func_sizetf;
      tf_data.user_data = "$urandom_range";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname = "$dist_uniform";
      tf_data.calltf = sys_dist_uniform_calltf;
      tf_data.compiletf = sys_rand_three_args_compiletf;
      tf_data.sizetf = sys_rand_func_sizetf;
      tf_data.user_data = "$dist_uniform";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname = "$dist_normal";
      tf_data.calltf = sys_dist_normal_calltf;
      tf_data.compiletf = sys_rand_three_args_compiletf;
      tf_data.sizetf = sys_rand_func_sizetf;
      tf_data.user_data = "$dist_normal";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname = "$dist_exponential";
      tf_data.calltf = sys_dist_exponential_calltf;
      tf_data.compiletf = sys_rand_two_args_compiletf;
      tf_data.sizetf = sys_rand_func_sizetf;
      tf_data.user_data = "$dist_exponential";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname = "$dist_poisson";
      tf_data.calltf = sys_dist_poisson_calltf;
      tf_data.compiletf = sys_rand_two_args_compiletf;
      tf_data.sizetf = sys_rand_func_sizetf;
      tf_data.user_data = "$dist_poisson";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname = "$dist_chi_square";
      tf_data.calltf = sys_dist_chi_square_calltf;
      tf_data.compiletf = sys_rand_two_args_compiletf;
      tf_data.sizetf = sys_rand_func_sizetf;
      tf_data.user_data = "$dist_chi_square";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname = "$dist_t";
      tf_data.calltf = sys_dist_t_calltf;
      tf_data.compiletf = sys_rand_two_args_compiletf;
      tf_data.sizetf = sys_rand_func_sizetf;
      tf_data.user_data = "$dist_t";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname = "$dist_erlang";
      tf_data.calltf = sys_dist_erlang_calltf;
      tf_data.compiletf = sys_rand_three_args_compiletf;
      tf_data.sizetf = sys_rand_func_sizetf;
      tf_data.user_data = "$dist_erlang";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);
}
