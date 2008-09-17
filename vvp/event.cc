/*
 * Copyright (c) 2004-2008 Stephen Williams (steve@icarus.com)
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

# include  "event.h"
# include  "compile.h"
# include  "vthread.h"
# include  "schedule.h"
# include  "vpi_priv.h"

# include  <string.h>
# include  <assert.h>
# include  <stdlib.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif

# include <iostream>

void waitable_hooks_s::run_waiting_threads_()
{
	// Run the non-blocking event controls.
      last = &event_ctls;
      for (evctl*cur = event_ctls; cur != 0;) {
	    if (cur->dec_and_run()) {
		  evctl*nxt = cur->next;
		  delete cur;
		  cur = nxt;
		  *last = cur;
	    } else {
		  last = &(cur->next);
		  cur = cur->next;
	    }
      }

      if (threads == 0)
	    return;

      vthread_t tmp = threads;
      threads = 0;
      vthread_schedule_list(tmp);
}

evctl::evctl(unsigned long ecount)
{
      ecount_ = ecount;
      next = 0;
}

bool evctl::dec_and_run()
{
      assert(ecount_ != 0);

      ecount_ -= 1;
      if (ecount_ == 0) run_run();

      return ecount_ == 0;
}

evctl_real::evctl_real(struct __vpiHandle*handle, double value,
                       unsigned long ecount)
:evctl(ecount)
{
      handle_ = handle;
      value_ = value;
}

void evctl_real::run_run()
{
      t_vpi_value val;

      val.format = vpiRealVal;
      val.value.real = value_;
      vpi_put_value(handle_, &val, 0, vpiNoDelay);
}

void schedule_evctl(struct __vpiHandle*handle, double value,
                    vvp_net_t*event, unsigned long ecount)
{
	// Get the functor we are going to wait on.
      waitable_hooks_s*ep = dynamic_cast<waitable_hooks_s*> (event->fun);
      assert(ep);
	// Now add this call to the end of the event list.
      *(ep->last) = new evctl_real(handle, value, ecount);
      ep->last = &((*(ep->last))->next);
}

evctl_vector::evctl_vector(vvp_net_ptr_t ptr, const vvp_vector4_t&value,
                           unsigned off, unsigned wid, unsigned long ecount)
:evctl(ecount), value_(value)
{
      ptr_ = ptr;
      off_ = off;
      wid_ = wid;
}

void evctl_vector::run_run()
{
      if (wid_ != 0) {
	    vvp_send_vec4_pv(ptr_, value_, off_, value_.size(), wid_);
      } else {
	    vvp_send_vec4(ptr_, value_);
      }
}

void schedule_evctl(vvp_net_ptr_t ptr, const vvp_vector4_t&value,
                    unsigned offset, unsigned wid,
                    vvp_net_t*event, unsigned long ecount)
{
	// Get the functor we are going to wait on.
      waitable_hooks_s*ep = dynamic_cast<waitable_hooks_s*> (event->fun);
      assert(ep);
	// Now add this call to the end of the event list.
      *(ep->last) = new evctl_vector(ptr, value, offset, wid, ecount);
      ep->last = &((*(ep->last))->next);
}


inline vvp_fun_edge::edge_t VVP_EDGE(vvp_bit4_t from, vvp_bit4_t to)
{
      return 1 << ((from << 2) | to);
}

const vvp_fun_edge::edge_t vvp_edge_posedge
      = VVP_EDGE(BIT4_0,BIT4_1)
      | VVP_EDGE(BIT4_0,BIT4_X)
      | VVP_EDGE(BIT4_0,BIT4_Z)
      | VVP_EDGE(BIT4_X,BIT4_1)
      | VVP_EDGE(BIT4_Z,BIT4_1)
      ;

const vvp_fun_edge::edge_t vvp_edge_negedge
      = VVP_EDGE(BIT4_1,BIT4_0)
      | VVP_EDGE(BIT4_1,BIT4_X)
      | VVP_EDGE(BIT4_1,BIT4_Z)
      | VVP_EDGE(BIT4_X,BIT4_0)
      | VVP_EDGE(BIT4_Z,BIT4_0)
      ;

const vvp_fun_edge::edge_t vvp_edge_none    = 0;

vvp_fun_edge::vvp_fun_edge(edge_t e, bool debug_flag)
: edge_(e), debug_(debug_flag)
{
      bits_[0] = BIT4_X;
      bits_[1] = BIT4_X;
      bits_[2] = BIT4_X;
      bits_[3] = BIT4_X;
}

vvp_fun_edge::~vvp_fun_edge()
{
}

void vvp_fun_edge::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
	/* See what kind of edge this represents. */
      edge_t mask = VVP_EDGE(bits_[port.port()], bit.value(0));

	/* Save the current input for the next time around. */
      bits_[port.port()] = bit.value(0);

      if ((edge_ == vvp_edge_none) || (edge_ & mask)) {
	    run_waiting_threads_();

	    vvp_net_t*net = port.ptr();
	    vvp_send_vec4(net->out, bit);
      }
}


vvp_fun_anyedge::vvp_fun_anyedge(bool debug_flag)
: debug_(debug_flag)
{
      for (unsigned idx = 0 ;  idx < 4 ;  idx += 1)
	    bitsr_[idx] = 0.0;
}

vvp_fun_anyedge::~vvp_fun_anyedge()
{
}

void vvp_fun_anyedge::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
      unsigned pdx = port.port();
      bool flag = false;

      if (bits_[pdx].size() != bit.size()) {
	    flag = true;

      } else {
	    for (unsigned idx = 0 ;  idx < bit.size() ;  idx += 1) {
		  if (bits_[pdx].value(idx) != bit.value(idx)) {
			flag = true;
			break;
		  }
	    }
      }

      if (flag) {
	    bits_[pdx] = bit;
	    run_waiting_threads_();
	    vvp_net_t*net = port.ptr();
	    vvp_send_vec4(net->out, bit);
      }
}

void vvp_fun_anyedge::recv_real(vvp_net_ptr_t port, double bit)
{
      unsigned pdx = port.port();
      bool flag = false;

      if (bitsr_[pdx] != bit) {
	    flag = true;
	    bitsr_[pdx] = bit;
      }

      if (flag) {
	    run_waiting_threads_();
	    vvp_net_t*net = port.ptr();
	    vvp_send_vec4(net->out, vvp_vector4_t());
      }
}

vvp_fun_event_or::vvp_fun_event_or()
{
}

vvp_fun_event_or::~vvp_fun_event_or()
{
}

void vvp_fun_event_or::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
      run_waiting_threads_();
      vvp_net_t*net = port.ptr();
      vvp_send_vec4(net->out, bit);
}

vvp_named_event::vvp_named_event(struct __vpiHandle*h)
{
      handle_ = h;
}

vvp_named_event::~vvp_named_event()
{
}

void vvp_named_event::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
      run_waiting_threads_();
      vvp_net_t*net = port.ptr();
      vvp_send_vec4(net->out, bit);

      vpip_run_named_event_callbacks(handle_);
}

/*
**  Create an event functor
**  edge:  compile_event(label, type, argc, argv, debug_flag)
**  or:    compile_event(label, NULL, argc, argv, debug_flag)
**
**  Named events are handled elsewhere.
*/

static void compile_event_or(char*label, unsigned argc, struct symb_s*argv);

void compile_event(char*label, char*type,
		   unsigned argc, struct symb_s*argv,
		   bool debug_flag)
{
      vvp_net_fun_t*fun = 0;

      if (type == 0) {
	    compile_event_or(label, argc, argv);
	    return;
      }

      if (strcmp(type,"edge") == 0) {

	    free(type);
	    fun = new vvp_fun_anyedge(debug_flag);

      } else {

	    vvp_fun_edge::edge_t edge = vvp_edge_none;

	    if (strcmp(type,"posedge") == 0)
		  edge = vvp_edge_posedge;
	    else if (strcmp(type,"negedge") == 0)
		  edge = vvp_edge_negedge;

	    assert(argc <= 4);
	    free(type);

	    fun = new vvp_fun_edge(edge, debug_flag);
      }

      vvp_net_t* ptr = new vvp_net_t;
      ptr->fun = fun;

      define_functor_symbol(label, ptr);
      free(label);

      inputs_connect(ptr, argc, argv);
}

static void compile_event_or(char*label, unsigned argc, struct symb_s*argv)
{
      vvp_net_fun_t*fun = new vvp_fun_event_or;
      vvp_net_t* ptr = new vvp_net_t;
      ptr->fun = fun;

      define_functor_symbol(label, ptr);
      free(label);

	/* This is a very special case. Point all the source inputs to
	   the same input. It doesn't matter that the streams get
	   tangled because data values are irrelevant. */
      for (unsigned idx = 0 ;  idx < argc ;  idx += 1) {
	    input_connect(ptr, 0, argv[idx].text);
      }
}

/*
 * This handles the compile of named events. This functor has no
 * inputs, it is only accessed by behavioral trigger statements, which
 * in vvp are %set instructions.
 */
void compile_named_event(char*label, char*name)
{
      vvp_net_t*ptr = new vvp_net_t;

      vpiHandle obj = vpip_make_named_event(name, ptr);
      ptr->fun = new vvp_named_event(obj);

      define_functor_symbol(label, ptr);
      compile_vpi_symbol(label, obj);
      vpip_attach_to_current_scope(obj);

      free(label);
      free(name);
}
