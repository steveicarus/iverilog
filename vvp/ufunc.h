#ifndef __ufunc_H
#define __ufunc_H
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

# include  "pointers.h"

/*
 * The .ufunc statement creates functors to represent user defined
 * functions. The function device itself is implemented as a thread
 * with a bunch of functors for the output bits. This thread has a set
 * of outputs, represented by output_functors and a set of inputs
 * connected to input functors. The input functors detect that a
 * change has occurred, and invoke the thread to process the new
 * values. The relationships work like this:
 *
 *  ufunc_input_functor_s --+--> ufunc_core --+--> ufunc_output_functor_s
 *                          |                 |
 *  ufunc_input_functor_s --+                 +--> ufunc_output_functor_s
 *                          |
 *  ufunc_input_functor_s --+
 */

class ufunc_core {

    public:
      ufunc_core(unsigned ow, vvp_ipoint_t ob, vvp_ipoint_t*op,
		 unsigned np, vvp_ipoint_t*p,
		 vvp_code_t start_address,
		 struct __vpiScope*run_scope);
      ~ufunc_core();

      void set_bit(unsigned port_idx, unsigned val);

      void assign_bits_to_ports();
      void finish_thread(vthread_t thr);

      struct __vpiScope*scope() { return scope_; }

    private:
	// The owid_ and obase_ point to the functor vector that makes
	// up the output of the function.
      unsigned owid_;
      vvp_ipoint_t obase_;
      vvp_ipoint_t*oports_;
	// Keep an array of vvp_ipoint_t pointers that point to .var
	// functors. These are the input ports of the function.
      unsigned nports_;
      vvp_ipoint_t*ports_;
	// This is a thread to execute the behavioral portion of the
	// function.
      vthread_t thread_;
      struct __vpiScope*scope_;
      vvp_code_t code_;

	// Save the input bits as I receive them.
      unsigned char*ibits_;
};

#endif
