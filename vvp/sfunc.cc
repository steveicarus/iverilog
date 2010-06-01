/*
 * Copyright (c) 2006-2009 Stephen Williams (steve@icarus.com)
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

# include  "compile.h"
# include  "config.h"
#ifdef CHECK_WITH_VALGRIND
# include "vvp_cleanup.h"
#endif
# include  "sfunc.h"
# include  <cstdlib>
# include  <cstring>
# include  <iostream>
# include  <cassert>


sfunc_core::sfunc_core(vvp_net_t*net, vpiHandle sys,
		       unsigned argc, vpiHandle*argv)
: vvp_wide_fun_core(net, argc)
{
      sys_  = sys;
      argc_ = argc;
      argv_ = argv;
}

sfunc_core::~sfunc_core()
{
      delete sys_;
#ifdef CHECK_WITH_VALGRIND
      for (unsigned i = 0; i < argc_; i += 1) {
	    constant_delete(argv_[i]);
      }
#endif
      delete [] argv_;
}

/*
 * This method is only called when a trigger event occurs. Just arrange for
 * the function to be called.
 */
void sfunc_core::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                           vvp_context_t)
{
      schedule_generic(this, 0, false);
}

void sfunc_core::recv_vec4_from_inputs(unsigned port)
{
      vpiHandle vpi = argv_[port];
      assert(vpi_get(vpiConstType,vpi) == vpiBinaryConst);

      struct __vpiBinaryConst*obj
	    = (struct __vpiBinaryConst*)vpi;

      obj->bits = value(port);

        /* Schedule the actual call after this finishes. */
      schedule_generic(this, 0, false);
}

void sfunc_core::recv_real_from_inputs(unsigned port)
{
      vpiHandle vpi = argv_[port];
      assert(vpi_get(vpiConstType,vpi) == vpiRealConst);

      struct __vpiRealConst*obj
	    = (struct __vpiRealConst*)vpi;

      obj->value = value_r(port);

        /* Schedule the actual call after this finishes. */
      schedule_generic(this, 0, false);
}

void sfunc_core::run_run()
{
      vpip_execute_vpi_call(0, sys_);
}

static int make_vpi_argv(unsigned argc, vpiHandle*vpi_argv,
			const char*arg_string)
{
      unsigned idx = 0;
      const char*cp = arg_string;
      int return_type = 0;

      switch (*cp) {
	  case 'r': // real result
	    cp += 1;
	    return_type = -vpiRealConst;
	    break;

	  case 'v': // vector4_t
	    cp += 1;
	    return_type = strtoul(cp, 0, 10);
	    cp += strspn(cp, "0123456789");
	    break;

	  default:
	    fprintf(stderr, "Unsupported type %c(%d).\n", *cp, *cp);
	    assert(0);
	    break;
      }

      while (*cp) {
	    assert(idx < argc);

	    switch (*cp) {
		case 'r': // real
		  cp += 1;
		  vpi_argv[idx] = vpip_make_real_const(0.0);
		  break;

		case 'v': { // vector4_t (v<n>)
		      cp += 1;
		      unsigned wid = strtoul(cp, 0, 10);
		      cp += strspn(cp, "0123456789");
		      vpi_argv[idx] = vpip_make_binary_const(wid, "x");
		      break;
		}

		default:
		  fprintf(stderr, "Unsupported type %c(%d).\n", *cp, *cp);
		  assert(0);
	    }
	    idx += 1;
      }

      assert(idx == argc);
      return return_type;
}


void compile_sfunc(char*label, char*name,  char*format_string,
		   long file_idx, long lineno,
		   unsigned argc, struct symb_s*argv,
                   char*trigger_label)
{
      vpiHandle*vpi_argv = new vpiHandle[argc];
      int width_code = make_vpi_argv(argc, vpi_argv, format_string);
      delete[] format_string;

      vvp_net_t*ptr = new vvp_net_t;

      vpiHandle sys = vpip_build_vpi_call(name, 0, width_code, ptr,
					  argc, vpi_argv, file_idx, lineno);
      assert(sys);

	/* Create and connect the functor to the label. */
      sfunc_core*score = new sfunc_core(ptr, sys, argc, vpi_argv);
      ptr->fun = score;
      define_functor_symbol(label, ptr);
      free(label);

	/* Link the inputs to the functor. */
      wide_inputs_connect(score, argc, argv);
      free(argv);

        /* If this function has a trigger event, connect the functor to
           that event. */
      if (trigger_label)
            input_connect(ptr, 0, trigger_label);
      delete[] name;
}
