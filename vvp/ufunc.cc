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

# include  "compile.h"
# include  "symbols.h"
# include  "codes.h"
# include  "functor.h"
# include  "ufunc.h"
# include  "vthread.h"
# include  "schedule.h"
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>
# include  <string.h>
# include  <iostream>
# include  <assert.h>

#ifdef __MINGW32__
#include <windows.h>
#endif

ufunc_core::ufunc_core(unsigned ow, vvp_ipoint_t ob, vvp_ipoint_t*op,
		       unsigned np, vvp_ipoint_t*p,
		       vvp_code_t start_address,
		       struct __vpiScope*run_scope)
: owid_(ow), obase_(ob), oports_(op), nports_(np), ports_(p)
{
      thread_ = 0;
      scope_ = run_scope;
      code_ = start_address;

      ibits_ = new unsigned char[(nports_+3) / 4];
      memset(ibits_, 0xaa, (nports_+3) / 4);
}

ufunc_core::~ufunc_core()
{
      delete[] ports_;
}

void ufunc_core::set_bit(unsigned port_idx, unsigned val)
{
      unsigned idx = port_idx / 4;
      unsigned pp  = port_idx % 4;

      static const unsigned char mask[4] = {0xfc, 0xf3, 0xcf, 0x3f};

      ibits_[idx] &= mask[pp];
      ibits_[idx] |= (val&3) << pp*2;

      if (thread_ == 0) {
	    thread_ = vthread_new(code_, scope_);
	    schedule_vthread(thread_, 0);
      }
}

static const unsigned char strong_values[4] = {St0, St1, StX, HiZ};

void ufunc_core::assign_bits_to_ports(void)
{
      for (unsigned idx = 0 ;  idx < nports_ ;  idx += 1) {
	    unsigned bit_val = ibits_[idx/4] >> (idx%4)*2;
	    bit_val &= 3;

	    functor_set(ports_[idx], bit_val, strong_values[bit_val], true);
      }
}

void ufunc_core::finish_thread(vthread_t thr)
{
      assert(thread_ == thr);
      thread_ = 0;

      for (unsigned idx = 0 ;  idx < owid_ ;  idx += 1) {
	    unsigned val = functor_get(oports_[idx]);
	    vvp_ipoint_t ptr = ipoint_index(obase_, idx);
	    functor_set(ptr, val, strong_values[val], false);
      }
}

/*
 * There is an instance of ufunc_output_functor_s for each output bit
 * of the function. This is the functor that passes the output bits to
 * the rest of the design. The functor simply puts its input to its
 * output.
 */
struct ufunc_output_functor_s  : public functor_s {
      void set(vvp_ipoint_t, bool push, unsigned val, unsigned str = 0);
};

void ufunc_output_functor_s::set(vvp_ipoint_t, bool push, unsigned
				 val, unsigned str)
{
      put_oval(val, push);
}

struct ufunc_input_functor_s  : public functor_s {
      void set(vvp_ipoint_t, bool push, unsigned val, unsigned str = 0);

      unsigned core_base_;
      ufunc_core*core_;
};

void ufunc_input_functor_s::set(vvp_ipoint_t ptr, bool,
				unsigned val, unsigned str)
{
      unsigned pp = ipoint_port(ptr);
      core_->set_bit(core_base_+pp, val);
}

/*
 * This function compiles the .ufunc statement that is discovered in
 * the source file. Create all the functors and the thread, and
 * connect them all up.
 */
void compile_ufunc(char*label, char*code, unsigned wid,
		   unsigned argc,  struct symb_s*argv,
		   unsigned portc, struct symb_s*portv,
		   unsigned retc,  struct symb_s*retv)
{

	/* Create an array of vvp_ipoint_t pointers, that point to the
	   .var bits of the function ports. Do this for the input
	   ports and the output port. */
      assert(argc == portc);
      vvp_ipoint_t* ports = new vvp_ipoint_t [portc];

      for (unsigned idx = 0 ;  idx < portc ;  idx += 1) {
	    functor_ref_lookup(ports+idx, portv[idx].text, portv[idx].idx);
      }

      assert(retc == wid);
      vvp_ipoint_t* rets = new vvp_ipoint_t [retc];

      for (unsigned idx = 0 ;  idx < retc ;  idx += 1) {
	    functor_ref_lookup(rets+idx, retv[idx].text, retv[idx].idx);
      }

	/* Create enough output functors for the output bits of the
	   function. */
      vvp_ipoint_t obase = functor_allocate(wid);
      struct ufunc_output_functor_s*fpa
	    = new struct ufunc_output_functor_s[wid];

      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(obase,idx);
	    functor_define(ptr, fpa+idx);
      }

      define_functor_symbol(label, obase);

	/* Construct some phantom code that is the thread of the
	   function call. The first instruction, at the start_address
	   of the function, loads the points and calls the function.
	   The last instruction is the usual %end. So the thread looks
	   like this:

	      %fork_ufunc <core>;
	      %join;
	      %join_ufunc;
	      %end;

	   The %fork_ufunc starts the user defined function by copying
	   the input values into local regs, forking a thread and
	   pushing that thread. The %join waits on that thread. The
	   $join_ufunc then copies the output values to the
	   destination net functors. */

      vvp_code_t start_code = codespace_allocate();
      start_code->opcode = of_FORK_UFUNC;
      code_label_lookup(start_code, code);

      { vvp_code_t codep = codespace_allocate();
	codep->opcode = &of_JOIN;
      }

      vvp_code_t ujoin_code;
      ujoin_code = codespace_allocate();
      ujoin_code->opcode = &of_JOIN_UFUNC;

      { vvp_code_t codep = codespace_allocate();
	codep->opcode = &of_END;
      }


	/* Create the function core object that references the output
	   functors and the function ports. The input functors will
	   point to this core to deliver input. */
      ufunc_core*core = new ufunc_core(wid, obase, rets,
				       portc, ports,
				       start_code,
				       vpip_peek_current_scope());
      start_code->ufunc_core_ptr = core;
      ujoin_code->ufunc_core_ptr = core;

	/* create enough input functors to connect to all the input
	   bits of the function. These are used to detect changes and
	   trigger the function thread. */
      unsigned icnt = (argc + 3) / 4;
      vvp_ipoint_t ibase = functor_allocate(icnt);
      struct ufunc_input_functor_s*ifp
	    = new struct ufunc_input_functor_s[icnt];

      for (unsigned idx = 0 ;  idx < icnt ;  idx += 1) {
	    vvp_ipoint_t ptr = ipoint_index(ibase,idx);
	    struct ufunc_input_functor_s*cur = ifp+idx;

	    cur->core_base_ = idx*4;
	    cur->core_ = core;

	    functor_define(ptr, ifp+idx);
      }

      inputs_connect(ibase, argc, argv);
}
