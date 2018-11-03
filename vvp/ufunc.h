#ifndef IVL_ufunc_H
#define IVL_ufunc_H
/*
 * Copyright (c) 2002-2014 Stephen Williams (steve@icarus.com)
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

# include  "config.h"

class __vpiScope;

/*
 * The .ufunc statement creates functors to represent user defined
 * functions within the netlist (as opposed to within behavioral
 * code.) The function device itself is implemented as a thread with a
 * bunch of functors to receive input and a functor to deliver the
 * output. (Functions have exactly one output.) The input functors
 * detect that a change has occurred, and invoke the thread to process
 * the new values. The thread then uses the output functor to deliver
 * the result. The relationships look like this:
 *
 *  ufunc_input_functor --+--> ufunc_core --> ...
 *                        |
 *  ufunc_input_functor --+
 *                        |
 *  ufunc_input_functor --+
 *
 * There are enough input functors to take all the function inputs, 4
 * per functor. These inputs deliver the changed input value to the
 * ufunc_core, which carries the infrastructure for the thread. The
 * ufunc_core is also a functor whose output is connected to the rest
 * of the netlist. This is where the result is delivered back to the
 * netlist.
 *
 * This class relies to the vvp_wide_fun_* classes in vvp_net.h.
 */

class ufunc_core : public vvp_wide_fun_core {

    public:
      ufunc_core(unsigned ow, vvp_net_t*ptr,
		 unsigned nports, vvp_net_t**ports,
		 vvp_code_t start_address,
		 __vpiScope*call_scope,
		 char*scope_label);
      virtual ~ufunc_core() =0;

      __vpiScope*call_scope() { return call_scope_; }
      __vpiScope*func_scope() { return func_scope_; }

      void assign_bits_to_ports(vvp_context_t context);
      virtual void finish_thread() =0;

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t context);

    protected:
      void finish_thread_real_();
      void finish_thread_vec4_();

    private:
      void recv_vec4_from_inputs(unsigned port);
      void recv_real_from_inputs(unsigned port);

      void invoke_thread_(void);


    private:
	// output width of the function node.
      unsigned owid_;
	// The vvp_net_t* objects for the function input ports. We use
	// these to write the input values to the reg input variable
	// functors for the thread.
      vvp_net_t**ports_;

	// This is a thread to execute the behavioral portion of the
	// function.
      vthread_t thread_;
      __vpiScope*call_scope_;
      __vpiScope*func_scope_;
      vvp_code_t code_;
};

#endif /* IVL_ufunc_H */
