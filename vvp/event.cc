/*
 * Copyright (c) 2004-2010 Stephen Williams (steve@icarus.com)
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
# include  "config.h"
#ifdef CHECK_WITH_VALGRIND
# include  "vvp_cleanup.h"
#endif
# include  <cstring>
# include  <cassert>
# include  <cstdlib>

# include <iostream>

void waitable_hooks_s::run_waiting_threads_(vthread_t&threads)
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

      vthread_t tmp = threads;
      if (tmp == 0) return;
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
	    vvp_send_vec4_pv(ptr_, value_, off_, value_.size(), wid_, 0);
      } else {
	    vvp_send_vec4(ptr_, value_, 0);
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

evctl_array::evctl_array(vvp_array_t memory, unsigned index,
                         const vvp_vector4_t&value, unsigned off,
                         unsigned long ecount)
:evctl(ecount), value_(value)
{
      mem_ = memory;
      idx_ = index;
      off_ = off;
}

void evctl_array::run_run()
{
      array_set_word(mem_, idx_, off_, value_);
}

void schedule_evctl(vvp_array_t memory, unsigned index,
                    const vvp_vector4_t&value, unsigned offset,
                    vvp_net_t*event, unsigned long ecount)
{
	// Get the functor we are going to wait on.
      waitable_hooks_s*ep = dynamic_cast<waitable_hooks_s*> (event->fun);
      assert(ep);
	// Now add this call to the end of the event list.
      *(ep->last) = new evctl_array(memory, index, value, offset, ecount);
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

struct vvp_fun_edge_state_s : public waitable_state_s {
      vvp_fun_edge_state_s()
      {
            for (unsigned idx = 0 ;  idx < 4 ;  idx += 1)
                  bits[idx] = BIT4_X;
      }

      vvp_bit4_t bits[4];
};

vvp_fun_edge::vvp_fun_edge(edge_t e)
: edge_(e)
{
      for (unsigned idx = 0 ;  idx < 4 ;  idx += 1)
            bits_[idx] = BIT4_X;
}

vvp_fun_edge::~vvp_fun_edge()
{
}

bool vvp_fun_edge::recv_vec4_(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                              vvp_bit4_t&old_bit, vthread_t&threads)
{
	/* See what kind of edge this represents. */
      edge_t mask = VVP_EDGE(old_bit, bit.value(0));

	/* Save the current input for the next time around. */
      old_bit = bit.value(0);

      if ((edge_ == vvp_edge_none) || (edge_ & mask)) {
	    run_waiting_threads_(threads);
            return true;
      }
      return false;
}

vvp_fun_edge_sa::vvp_fun_edge_sa(edge_t e)
: vvp_fun_edge(e), threads_(0)
{
}

vvp_fun_edge_sa::~vvp_fun_edge_sa()
{
}

vthread_t vvp_fun_edge_sa::add_waiting_thread(vthread_t thread)
{
      vthread_t tmp = threads_;
      threads_ = thread;

      return tmp;
}

void vvp_fun_edge_sa::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                                vvp_context_t)
{
      if (recv_vec4_(port, bit, bits_[port.port()], threads_)) {
	    vvp_net_t*net = port.ptr();
	    vvp_send_vec4(net->out, bit, 0);
      }
}

vvp_fun_edge_aa::vvp_fun_edge_aa(edge_t e)
: vvp_fun_edge(e)
{
      context_scope_ = vpip_peek_context_scope();
      context_idx_ = vpip_add_item_to_context(this, context_scope_);
}

vvp_fun_edge_aa::~vvp_fun_edge_aa()
{
}

void vvp_fun_edge_aa::alloc_instance(vvp_context_t context)
{
      vvp_set_context_item(context, context_idx_, new vvp_fun_edge_state_s);
      reset_instance(context);
}

void vvp_fun_edge_aa::reset_instance(vvp_context_t context)
{
      vvp_fun_edge_state_s*state = static_cast<vvp_fun_edge_state_s*>
            (vvp_get_context_item(context, context_idx_));

      state->threads = 0;
      for (unsigned idx = 0 ;  idx < 4 ;  idx += 1)
            state->bits[idx] = bits_[idx];
}

#ifdef CHECK_WITH_VALGRIND
void vvp_fun_edge_aa::free_instance(vvp_context_t context)
{
      vvp_fun_edge_state_s*state = static_cast<vvp_fun_edge_state_s*>
            (vvp_get_context_item(context, context_idx_));
      delete state;
}
#endif

vthread_t vvp_fun_edge_aa::add_waiting_thread(vthread_t thread)
{
      vvp_fun_edge_state_s*state = static_cast<vvp_fun_edge_state_s*>
            (vthread_get_wt_context_item(context_idx_));

      vthread_t tmp = state->threads;
      state->threads = thread;

      return tmp;
}

void vvp_fun_edge_aa::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                                vvp_context_t context)
{
      if (context) {
            vvp_fun_edge_state_s*state = static_cast<vvp_fun_edge_state_s*>
                  (vvp_get_context_item(context, context_idx_));

            if (recv_vec4_(port, bit, state->bits[port.port()], state->threads)) {
                  vvp_net_t*net = port.ptr();
                  vvp_send_vec4(net->out, bit, context);
            }
      } else {
            context = context_scope_->live_contexts;
            while (context) {
                  recv_vec4(port, bit, context);
                  context = vvp_get_next_context(context);
            }
            bits_[port.port()] = bit.value(0);
      }
}

struct vvp_fun_anyedge_state_s : public waitable_state_s {
      vvp_fun_anyedge_state_s()
      {
            for (unsigned idx = 0 ;  idx < 4 ;  idx += 1)
                  bitsr[idx] = 0.0;
      }

      vvp_vector4_t bits[4];
      double bitsr[4];
};

vvp_fun_anyedge::vvp_fun_anyedge()
{
      for (unsigned idx = 0 ;  idx < 4 ;  idx += 1)
	    bitsr_[idx] = 0.0;
}

vvp_fun_anyedge::~vvp_fun_anyedge()
{
}

bool vvp_fun_anyedge::recv_vec4_(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                                 vvp_vector4_t&old_bits, vthread_t&threads)
{
      bool flag = false;

      if (old_bits.size() != bit.size()) {
	    flag = true;

      } else {
	    for (unsigned idx = 0 ;  idx < bit.size() ;  idx += 1) {
		  if (old_bits.value(idx) != bit.value(idx)) {
			flag = true;
			break;
		  }
	    }
      }

      if (flag) {
	    old_bits = bit;
	    run_waiting_threads_(threads);
      }

      return flag;
}

bool vvp_fun_anyedge::recv_real_(vvp_net_ptr_t port, double bit,
                                 double&old_bits, vthread_t&threads)
{
      if (old_bits != bit) {
	    old_bits = bit;
	    run_waiting_threads_(threads);
            return true;
      }
      return false;
}

vvp_fun_anyedge_sa::vvp_fun_anyedge_sa()
: threads_(0)
{
}

vvp_fun_anyedge_sa::~vvp_fun_anyedge_sa()
{
}

vthread_t vvp_fun_anyedge_sa::add_waiting_thread(vthread_t thread)
{
      vthread_t tmp = threads_;
      threads_ = thread;

      return tmp;
}

void vvp_fun_anyedge_sa::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                                   vvp_context_t)
{
      if (recv_vec4_(port, bit, bits_[port.port()], threads_)) {
	    vvp_net_t*net = port.ptr();
	    vvp_send_vec4(net->out, bit, 0);
      }
}

void vvp_fun_anyedge_sa::recv_real(vvp_net_ptr_t port, double bit,
                                   vvp_context_t)
{
      if (recv_real_(port, bit, bitsr_[port.port()], threads_)) {
	    vvp_net_t*net = port.ptr();
	    vvp_send_vec4(net->out, vvp_vector4_t(), 0);
      }
}

vvp_fun_anyedge_aa::vvp_fun_anyedge_aa()
{
      context_scope_ = vpip_peek_context_scope();
      context_idx_ = vpip_add_item_to_context(this, context_scope_);
}

vvp_fun_anyedge_aa::~vvp_fun_anyedge_aa()
{
}

void vvp_fun_anyedge_aa::alloc_instance(vvp_context_t context)
{
      vvp_set_context_item(context, context_idx_, new vvp_fun_anyedge_state_s);
      reset_instance(context);
}

void vvp_fun_anyedge_aa::reset_instance(vvp_context_t context)
{
      vvp_fun_anyedge_state_s*state = static_cast<vvp_fun_anyedge_state_s*>
            (vvp_get_context_item(context, context_idx_));

      state->threads = 0;
      for (unsigned idx = 0 ;  idx < 4 ;  idx += 1) {
            state->bits[idx] = bits_[idx];
	    state->bitsr[idx] = bitsr_[idx];
      }
}

#ifdef CHECK_WITH_VALGRIND
void vvp_fun_anyedge_aa::free_instance(vvp_context_t context)
{
      vvp_fun_anyedge_state_s*state = static_cast<vvp_fun_anyedge_state_s*>
            (vvp_get_context_item(context, context_idx_));
      delete state;
}
#endif

vthread_t vvp_fun_anyedge_aa::add_waiting_thread(vthread_t thread)
{
      vvp_fun_anyedge_state_s*state = static_cast<vvp_fun_anyedge_state_s*>
            (vthread_get_wt_context_item(context_idx_));

      vthread_t tmp = state->threads;
      state->threads = thread;

      return tmp;
}

void vvp_fun_anyedge_aa::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                                   vvp_context_t context)
{
      if (context) {
            vvp_fun_anyedge_state_s*state = static_cast<vvp_fun_anyedge_state_s*>
                  (vvp_get_context_item(context, context_idx_));

            if (recv_vec4_(port, bit, state->bits[port.port()], state->threads)) {
                  vvp_net_t*net = port.ptr();
                  vvp_send_vec4(net->out, bit, context);
            }
      } else {
            context = context_scope_->live_contexts;
            while (context) {
                  recv_vec4(port, bit, context);
                  context = vvp_get_next_context(context);
            }
            bits_[port.port()] = bit;
      }
}

void vvp_fun_anyedge_aa::recv_real(vvp_net_ptr_t port, double bit,
                                   vvp_context_t context)
{
      if (context) {
            vvp_fun_anyedge_state_s*state = static_cast<vvp_fun_anyedge_state_s*>
                  (vvp_get_context_item(context, context_idx_));

            if (recv_real_(port, bit, state->bitsr[port.port()], state->threads)) {
                  vvp_net_t*net = port.ptr();
                  vvp_send_vec4(net->out, vvp_vector4_t(), context);
            }
      } else {
            context = context_scope_->live_contexts;
            while (context) {
                  recv_real(port, bit, context);
                  context = vvp_get_next_context(context);
            }
            bitsr_[port.port()] = bit;
      }
}

vvp_fun_event_or::vvp_fun_event_or()
{
}

vvp_fun_event_or::~vvp_fun_event_or()
{
}

vvp_fun_event_or_sa::vvp_fun_event_or_sa()
: threads_(0)
{
}

vvp_fun_event_or_sa::~vvp_fun_event_or_sa()
{
}

vthread_t vvp_fun_event_or_sa::add_waiting_thread(vthread_t thread)
{
      vthread_t tmp = threads_;
      threads_ = thread;

      return tmp;
}

void vvp_fun_event_or_sa::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                                    vvp_context_t)
{
      run_waiting_threads_(threads_);
      vvp_net_t*net = port.ptr();
      vvp_send_vec4(net->out, bit, 0);
}

vvp_fun_event_or_aa::vvp_fun_event_or_aa()
{
      context_scope_ = vpip_peek_context_scope();
      context_idx_ = vpip_add_item_to_context(this, context_scope_);
}

vvp_fun_event_or_aa::~vvp_fun_event_or_aa()
{
}

void vvp_fun_event_or_aa::alloc_instance(vvp_context_t context)
{
      vvp_set_context_item(context, context_idx_, new waitable_state_s);
}

void vvp_fun_event_or_aa::reset_instance(vvp_context_t context)
{
      waitable_state_s*state = static_cast<waitable_state_s*>
            (vvp_get_context_item(context, context_idx_));

      state->threads = 0;
}

#ifdef CHECK_WITH_VALGRIND
void vvp_fun_event_or_aa::free_instance(vvp_context_t context)
{
      waitable_state_s*state = static_cast<waitable_state_s*>
            (vvp_get_context_item(context, context_idx_));
      delete state;
}
#endif

vthread_t vvp_fun_event_or_aa::add_waiting_thread(vthread_t thread)
{
      waitable_state_s*state = static_cast<waitable_state_s*>
            (vthread_get_wt_context_item(context_idx_));

      vthread_t tmp = state->threads;
      state->threads = thread;

      return tmp;
}

void vvp_fun_event_or_aa::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                                    vvp_context_t context)
{
      if (context) {
            waitable_state_s*state = static_cast<waitable_state_s*>
                  (vvp_get_context_item(context, context_idx_));

            run_waiting_threads_(state->threads);
            vvp_net_t*net = port.ptr();
            vvp_send_vec4(net->out, bit, context);
      } else {
            context = context_scope_->live_contexts;
            while (context) {
                  recv_vec4(port, bit, context);
                  context = vvp_get_next_context(context);
            }
      }
}

vvp_named_event::vvp_named_event(struct __vpiHandle*h)
{
      handle_ = h;
}

vvp_named_event::~vvp_named_event()
{
}

vvp_named_event_sa::vvp_named_event_sa(struct __vpiHandle*h)
: vvp_named_event(h), threads_(0)
{
}

vvp_named_event_sa::~vvp_named_event_sa()
{
}

vthread_t vvp_named_event_sa::add_waiting_thread(vthread_t thread)
{
      vthread_t tmp = threads_;
      threads_ = thread;

      return tmp;
}

void vvp_named_event_sa::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                                   vvp_context_t)
{
      run_waiting_threads_(threads_);
      vvp_net_t*net = port.ptr();
      vvp_send_vec4(net->out, bit, 0);

      vpip_run_named_event_callbacks(handle_);
}

vvp_named_event_aa::vvp_named_event_aa(struct __vpiHandle*h)
: vvp_named_event(h)
{
      context_idx_ = vpip_add_item_to_context(this, vpip_peek_context_scope());
}

vvp_named_event_aa::~vvp_named_event_aa()
{
}

void vvp_named_event_aa::alloc_instance(vvp_context_t context)
{
      vvp_set_context_item(context, context_idx_, new waitable_state_s);
}

void vvp_named_event_aa::reset_instance(vvp_context_t context)
{
      waitable_state_s*state = static_cast<waitable_state_s*>
            (vvp_get_context_item(context, context_idx_));

      state->threads = 0;
}

#ifdef CHECK_WITH_VALGRIND
void vvp_named_event_aa::free_instance(vvp_context_t context)
{
      waitable_state_s*state = static_cast<waitable_state_s*>
            (vvp_get_context_item(context, context_idx_));
      delete state;
}
#endif

vthread_t vvp_named_event_aa::add_waiting_thread(vthread_t thread)
{
      waitable_state_s*state = static_cast<waitable_state_s*>
            (vthread_get_wt_context_item(context_idx_));

      vthread_t tmp = state->threads;
      state->threads = thread;

      return tmp;
}

void vvp_named_event_aa::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                                   vvp_context_t context)
{
      assert(context);

      waitable_state_s*state = static_cast<waitable_state_s*>
            (vvp_get_context_item(context, context_idx_));

      run_waiting_threads_(state->threads);
      vvp_net_t*net = port.ptr();
      vvp_send_vec4(net->out, bit, context);
}

/*
**  Create an event functor
**  edge:  compile_event(label, type, argc, argv, debug_flag)
**  or:    compile_event(label, NULL, argc, argv, debug_flag)
**
**  Named events are handled elsewhere.
*/

static void compile_event_or(char*label, unsigned argc, struct symb_s*argv);

void compile_event(char*label, char*type, unsigned argc, struct symb_s*argv)
{
      vvp_net_fun_t*fun = 0;

      if (type == 0) {
	    compile_event_or(label, argc, argv);
	    return;
      }

      if (strcmp(type,"edge") == 0) {

	    free(type);

            if (vpip_peek_current_scope()->is_automatic) {
                  fun = new vvp_fun_anyedge_aa;
            } else {
                  fun = new vvp_fun_anyedge_sa;
            }

      } else {

	    vvp_fun_edge::edge_t edge = vvp_edge_none;

	    if (strcmp(type,"posedge") == 0)
		  edge = vvp_edge_posedge;
	    else if (strcmp(type,"negedge") == 0)
		  edge = vvp_edge_negedge;

	    assert(argc <= 4);
	    free(type);

            if (vpip_peek_current_scope()->is_automatic) {
                  fun = new vvp_fun_edge_aa(edge);
            } else {
                  fun = new vvp_fun_edge_sa(edge);
            }

      }

      vvp_net_t* ptr = new vvp_net_t;
      ptr->fun = fun;

      define_functor_symbol(label, ptr);
      free(label);

      inputs_connect(ptr, argc, argv);
      free(argv);
}

static void compile_event_or(char*label, unsigned argc, struct symb_s*argv)
{
      vvp_net_t* ptr = new vvp_net_t;
      if (vpip_peek_current_scope()->is_automatic) {
            ptr->fun = new vvp_fun_event_or_aa;
      } else {
            ptr->fun = new vvp_fun_event_or_sa;
      }
      define_functor_symbol(label, ptr);
      free(label);

	/* This is a very special case. Point all the source inputs to
	   the same input. It doesn't matter that the streams get
	   tangled because data values are irrelevant. */
      for (unsigned idx = 0 ;  idx < argc ;  idx += 1) {
	    input_connect(ptr, 0, argv[idx].text);
      }
      free(argv);
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

      if (vpip_peek_current_scope()->is_automatic) {
            ptr->fun = new vvp_named_event_aa(obj);
      } else {
            ptr->fun = new vvp_named_event_sa(obj);
      }
      define_functor_symbol(label, ptr);
      compile_vpi_symbol(label, obj);
      vpip_attach_to_current_scope(obj);

      free(label);
      delete[] name;
}

#ifdef CHECK_WITH_VALGRIND
void named_event_delete(struct __vpiHandle*handle)
{
      struct __vpiNamedEvent *obj = (struct __vpiNamedEvent *) handle;

      while (obj->callbacks) {
	    struct __vpiCallback*tmp = obj->callbacks->next;
	    delete_vpi_callback(obj->callbacks);
	    obj->callbacks = tmp;
      }

      free(obj);
}
#endif
