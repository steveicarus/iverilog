/*
 * Copyright (c) 2000-2014 Stephen Williams (steve@icarus.com)
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
# include  "ivl_alloc.h"

/*
 * Implement the $random system function using the ``Mersenne
 * Twister'' random number generator MT19937.
 */

/* make sure this matches N+1 in mti19937int.c */
#define NP1	624+1

/* Icarus seed cookie */
#define COOKIE	0x1ca1ca1c

static struct context_s global_context = {NP1, {0} };

static long mti_dist_uniform(long*seed, long start, long end)
{
      (void)seed; /* Parameter is not used. */
      if (start >= end) return start;

      if ((start > LONG_MIN) || (end < LONG_MAX)) {
	    long range = end - start;
	    return start + genrand(&global_context)%range;
      } else {
	    return genrand(&global_context);
      }
}

static PLI_INT32 sys_mti_dist_uniform_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh, argv, seed, start, end;
      s_vpi_value val;
      long i_seed, i_start, i_end;

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
      val.value.integer = mti_dist_uniform(&i_seed, i_start, i_end);
      vpi_put_value(callh, &val, 0, vpiNoDelay);

	/* Return the seed. */
      val.value.integer = i_seed;
      vpi_put_value(seed, &val, 0, vpiNoDelay);

      vpi_free_object(argv);
      return 0;
}

static PLI_INT32 sys_mti_random_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh, argv, seed = 0;
      s_vpi_value val;
      int i_seed = COOKIE;
      struct context_s *context;

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

	      /* Since there is a seed use the current
	         context or create a new one */
	    context = (struct context_s *)vpi_get_userdata(callh);
	    if (!context) {
		  context = (struct context_s *)calloc(1, sizeof(*context));
		  context->mti = NP1;

		    /* squirrel away context */
		  vpi_put_userdata(callh, (void *)context);
	    }

	      /* If the argument is not the Icarus cookie, then
		 reseed context */
	    if (i_seed != COOKIE) sgenrand(context, i_seed);
      } else {
	    /* use global context */
          context = &global_context;
      }

        /* Calculate and return the result */
      val.value.integer = genrand(context);
      vpi_put_value(callh, &val, 0, vpiNoDelay);

        /* mark seed with cookie */
      if (seed && i_seed != COOKIE) {
	    val.value.integer = COOKIE;
	    vpi_put_value(seed, &val, 0, vpiNoDelay);
      }

      return 0;
}

void sys_random_mti_register(void)
{
      s_vpi_systf_data tf_data;
      vpiHandle res;

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname      = "$mti_random";
      tf_data.calltf      = sys_mti_random_calltf;
      tf_data.compiletf   = sys_random_compiletf;
      tf_data.user_data   = "$mti_random";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type        = vpiSysFunc;
      tf_data.sysfunctype = vpiSysFuncInt;
      tf_data.tfname      = "$mti_dist_uniform";
      tf_data.calltf      = sys_mti_dist_uniform_calltf;
      tf_data.compiletf   = sys_rand_three_args_compiletf;
      tf_data.user_data   = "$mti_dist_uniform";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);
}
