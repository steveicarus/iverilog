/*
 * Copyright (c) 2000-2003 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: sys_random.c,v 1.10 2004/03/15 18:35:37 steve Exp $"
#endif

# include "sys_priv.h"

# include  <vpi_user.h>
# include  <assert.h>
# include  <stdlib.h>
# include  <math.h>
# include  <limits.h>

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

/* make sure this matches N+1 in mti19937int.c */
#define NP1	624+1

/* Icarus seed cookie */
#define COOKIE	0x1ca1ca1c

static struct context_s global_context = {
#if defined(__GCC__)
    .mti =
#else
    // For MSVC simply use the fact that mti is located first
#endif
        NP1 };

static int sys_random_calltf(char*name)
{
      s_vpi_value val;
      vpiHandle call_handle;
      vpiHandle argv;
      vpiHandle seed = 0;
      int i_seed = 0;
      struct context_s *context;

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

	      /* Since there is a seed use the current 
	         context or create a new one */
	    context = (struct context_s *)vpi_get_userdata(call_handle);
	    if (!context) {
		  context = (struct context_s *)calloc(1, sizeof(*context));
		  context->mti = NP1;
		  assert(context);

		    /* squrrel away context */
		  vpi_put_userdata(call_handle, (void *)context);
	    }

	      /* If the argument is not the Icarus cookie, then
		 reseed context */
	    if (i_seed != COOKIE)
	          sgenrand(context, i_seed);
      } else {
	    /* use global context */
          context = &global_context;
      }

      val.format = vpiIntVal;
      val.value.integer = genrand(context);

      vpi_put_value(call_handle, &val, 0, vpiNoDelay);

        /* mark seed with cookie */
      if (seed && i_seed != COOKIE) {
	    val.format = vpiIntVal;
	    val.value.integer = COOKIE;		
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
 * Revision 1.10  2004/03/15 18:35:37  steve
 *  Assume struct initializers are GCC specific.
 *
 * Revision 1.9  2004/01/21 01:22:53  steve
 *  Give the vip directory its own configure and vpi_config.h
 *
 * Revision 1.8  2003/11/10 20:15:33  steve
 *  Simply MSVC compatibility patch.
 *
 * Revision 1.7  2003/05/15 00:38:29  steve
 *  Eliminate some redundant vpi_put_values.
 *
 * Revision 1.6  2003/05/14 04:18:16  steve
 *  Use seed to store random number context.
 *
 * Revision 1.5  2002/08/12 01:35:05  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.4  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
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

