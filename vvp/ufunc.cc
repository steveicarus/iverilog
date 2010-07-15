/*
 * Copyright (c) 2002-2010 Stephen Williams (steve@icarus.com)
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

# include  "vvp_net.h"
# include  "compile.h"
# include  "symbols.h"
# include  "codes.h"
# include  "ufunc.h"
# include  "vthread.h"
# include  "schedule.h"
#ifdef CHECK_WITH_VALGRIND
# include  "vvp_cleanup.h"
#endif
# include  <cstdlib>
# include  <cstring>
# include  <iostream>
# include  <cassert>
# include  <map>

#ifdef __MINGW32__
#include <windows.h>
#endif

ufunc_core::ufunc_core(unsigned owid, vvp_net_t*ptr,
		       unsigned nports, vvp_net_t**ports,
		       vvp_code_t sa, struct __vpiScope*call_scope__,
		       char*result_label, char*scope_label)
: vvp_wide_fun_core(ptr, nports)
{
      owid_ = owid;
      ports_ = ports;
      code_ = sa;
      thread_ = 0;
      call_scope_ = call_scope__;

      functor_ref_lookup(&result_, result_label);

	/* A __vpiScope starts with a __vpiHandle structure so this is
	   a safe cast. We need the (void*) to avoid a dereferenced
	   type punned pointer warning from some gcc compilers. */
      compile_vpi_lookup((vpiHandle*)(void*)(&func_scope_), scope_label);
}

ufunc_core::~ufunc_core()
{
      delete [] ports_;
}

/*
 * This method is called by the %exec_ufunc function to prepare the
 * input variables of the function for execution. The method copies
 * the input values collected by the core to the variables.
 */
void ufunc_core::assign_bits_to_ports(vvp_context_t context)
{
      for (unsigned idx = 0 ; idx < port_count() ;  idx += 1) {
	    vvp_net_t*net = ports_[idx];
	    vvp_net_ptr_t pp (net, 0);
	    if (vvp_fun_signal_real*tmp = dynamic_cast<vvp_fun_signal_real*>(net->fun))
		  tmp->recv_real(pp, value_r(idx), context);
	    if (vvp_fun_signal_vec*tmp = dynamic_cast<vvp_fun_signal_vec*>(net->fun))
		  tmp->recv_vec4(pp, value(idx), context);
      }
}

/*
 * This method is called by the %exec_ufunc instruction to copy the
 * result from the return code variable and deliver it to the output
 * of the functor, back into the netlist.
 */
void ufunc_core::finish_thread(vthread_t thr)
{
      thread_ = 0;
      if (vvp_fun_signal_real*sig = dynamic_cast<vvp_fun_signal_real*>(result_->fun))

	    propagate_real(sig->real_value());

      if (vvp_fun_signal_vec*sig = dynamic_cast<vvp_fun_signal_vec*>(result_->fun))
	    propagate_vec4(sig->vec4_value());
}

/*
 * This method is only called when a trigger event occurs. Just arrange for
 * the function to be called.
 */
void ufunc_core::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                           vvp_context_t)
{
      invoke_thread_();
}

/*
 * The recv_vec4 methods of the input functors call this to assign the
 * input value to the port of the functor. I save the input value and
 * arrange for the function to be called.
 */
void ufunc_core::recv_vec4_from_inputs(unsigned port)
{
      invoke_thread_();
}

void ufunc_core::recv_real_from_inputs(unsigned port)
{
      invoke_thread_();
}

void ufunc_core::invoke_thread_()
{
      if (thread_ == 0) {
	    thread_ = vthread_new(code_, call_scope_);
	    schedule_vthread(thread_, 0);
      }
}

/*
 * This function compiles the .ufunc statement that is discovered in
 * the source file. Create all the functors and the thread, and
 * connect them all up.
 *
 * The argv list is a list of the inputs to the function.
 *
 * The portv list is a list of variables that the function reads as
 * inputs. The core assigns values to these nets as part of the startup.
 */
void compile_ufunc(char*label, char*code, unsigned wid,
		   unsigned argc,  struct symb_s*argv,
		   unsigned portc, struct symb_s*portv,
		   struct symb_s retv, char*scope_label,
                   char*trigger_label)
{
	/* The input argument list and port list must have the same
	   sizes, since internally we will be mapping the inputs list
	   to the ports list. */
      assert(argc == portc);

      struct __vpiScope*call_scope = vpip_peek_current_scope();
      assert(call_scope);

	/* Construct some phantom code that is the thread of the
	   function call. The first instruction, at the start_address
	   of the function, loads the points and calls the function.
	   The last instruction is the usual %end. So the thread looks
	   like this:

	      %exec_ufunc <core>;
	      %end;

	   The %exec_ufunc copies the input values into local regs,
           runs the function code, then copies the output values to
           the destination net functors. */

      vvp_code_t start_code = codespace_allocate();
      start_code->opcode = of_EXEC_UFUNC;
      code_label_lookup(start_code, code);

      vvp_code_t end_code = codespace_allocate();
      end_code->opcode = &of_END;

	/* Run through the function ports (which are related to but
	   not the same as the input ports) and arrange for their
	   binding. */
      vvp_net_t**ports = new vvp_net_t*[portc];
      for (unsigned idx = 0 ;  idx < portc ;  idx += 1) {
	    functor_ref_lookup(&ports[idx], portv[idx].text);
      }

	/* Create the output functor and attach it to the label. Tell
	   it about the start address of the code stub, and the scope
	   that will contain the execution. */
      vvp_net_t*ptr = new vvp_net_t;
      ufunc_core*fcore = new ufunc_core(wid, ptr, portc, ports,
					start_code, call_scope,
					retv.text, scope_label);
      ptr->fun = fcore;
      define_functor_symbol(label, ptr);
      free(label);

      start_code->ufunc_core_ptr = fcore;

      wide_inputs_connect(fcore, argc, argv);

        /* If this function has a trigger event, connect the functor to
           that event. */
      if (trigger_label)
            input_connect(ptr, 0, trigger_label);

      free(argv);
      free(portv);
}

#ifdef CHECK_WITH_VALGRIND
static map<ufunc_core*, bool> ufunc_map;

void exec_ufunc_delete(vvp_code_t euf_code)
{
      ufunc_map[euf_code->ufunc_core_ptr] = true;
}

void ufunc_pool_delete(void)
{
      map<ufunc_core*, bool>::iterator iter;
      for (iter = ufunc_map.begin(); iter != ufunc_map.end(); iter++) {
	    delete iter->first;
      }
}
#endif
