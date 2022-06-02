/*
 * Copyright (c) 2004-2021 Stephen Williams (steve@icarus.com)
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

# include  "event.h"
# include  "compile.h"
# include  "vthread.h"
# include  "schedule.h"
# include  "vpi_priv.h"
# include  "config.h"
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

evctl_real::evctl_real(__vpiHandle*handle, double value,
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

void schedule_evctl(__vpiHandle*handle, double value,
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
:evctl(ecount), ptr_(ptr), value_(value)
{
      off_ = off;
      wid_ = wid;
}

void evctl_vector::run_run()
{
      if (wid_ != 0) {
	    vvp_send_vec4_pv(ptr_, value_, off_, wid_, 0);
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
      mem_->set_word(idx_, off_, value_);
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

evctl_array_r::evctl_array_r(vvp_array_t memory, unsigned index,
                             double value, unsigned long ecount)
:evctl(ecount)
{
      mem_ = memory;
      idx_ = index;
      value_ = value;
}

void evctl_array_r::run_run()
{
      mem_->set_word(idx_, value_);
}

void schedule_evctl(vvp_array_t memory, unsigned index,
                    double value,
                    vvp_net_t*event, unsigned long ecount)
{
	// Get the functor we are going to wait on.
      waitable_hooks_s*ep = dynamic_cast<waitable_hooks_s*> (event->fun);
      assert(ep);
	// Now add this call to the end of the event list.
      *(ep->last) = new evctl_array_r(memory, index, value, ecount);
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

const vvp_fun_edge::edge_t vvp_edge_edge
      = VVP_EDGE(BIT4_0,BIT4_1)
      | VVP_EDGE(BIT4_1,BIT4_0)
      | VVP_EDGE(BIT4_0,BIT4_X)
      | VVP_EDGE(BIT4_X,BIT4_0)
      | VVP_EDGE(BIT4_0,BIT4_Z)
      | VVP_EDGE(BIT4_Z,BIT4_0)
      | VVP_EDGE(BIT4_X,BIT4_1)
      | VVP_EDGE(BIT4_1,BIT4_X)
      | VVP_EDGE(BIT4_Z,BIT4_1)
      | VVP_EDGE(BIT4_1,BIT4_Z)
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

bool vvp_fun_edge::recv_vec4_(const vvp_vector4_t&bit,
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
      if (recv_vec4_(bit, bits_[port.port()], threads_)) {
	    vvp_net_t*net = port.ptr();
	    net->send_vec4(bit, 0);
      }
}

void vvp_fun_edge_sa::recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
				   unsigned base, unsigned vwid, vvp_context_t)
{
      assert(base == 0);
      if (recv_vec4_(bit, bits_[port.port()], threads_)) {
	    vvp_net_t*net = port.ptr();
	    net->send_vec4_pv(bit, base, vwid, 0);
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

            if (recv_vec4_(bit, state->bits[port.port()], state->threads)) {
                  vvp_net_t*net = port.ptr();
                  net->send_vec4(bit, context);
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

class anyedge_value {

    public:
      anyedge_value() {};
      virtual ~anyedge_value() {};

      virtual void reset() = 0;

      virtual void duplicate(anyedge_value*&dup) = 0;
};

class anyedge_vec4_value : public anyedge_value {

    public:
      anyedge_vec4_value() {};
      virtual ~anyedge_vec4_value() {};

      void reset() { old_bits.set_to_x(); }

      void set(const vvp_vector4_t&bit) { old_bits = bit; };

      void duplicate(anyedge_value*&dup);

      bool recv_vec4(const vvp_vector4_t&bit);

      bool recv_vec4_pv(const vvp_vector4_t&bit, unsigned base,
			unsigned vwid);

    private:
      vvp_vector4_t old_bits;
};

static anyedge_vec4_value*get_vec4_value(anyedge_value*&value)
{
      anyedge_vec4_value*vec4_value = dynamic_cast<anyedge_vec4_value*>(value);
      if (!value) {
	    vec4_value = new anyedge_vec4_value();
	    delete value;
	    value = vec4_value;
      }
      return vec4_value;
}

class anyedge_real_value : public anyedge_value {

    public:
      anyedge_real_value() : old_bits(0.0) {};
      virtual ~anyedge_real_value() {};

      void reset() { old_bits = 0.0; }

      void set(double bit) { old_bits = bit; };

      void duplicate(anyedge_value*&dup);

      bool recv_real(double bit);

    private:
      double old_bits;
};

static anyedge_real_value*get_real_value(anyedge_value*&value)
{
      anyedge_real_value*real_value = dynamic_cast<anyedge_real_value*>(value);
      if (!value) {
	    real_value = new anyedge_real_value();
	    delete value;
	    value = real_value;
      }
      return real_value;
}

class anyedge_string_value : public anyedge_value {

    public:
      anyedge_string_value() {};
      virtual ~anyedge_string_value() {};

      void reset() { old_bits.clear(); }

      void set(const std::string&bit) { old_bits = bit; };

      void duplicate(anyedge_value*&dup);

      bool recv_string(const std::string&bit);

    private:
      std::string old_bits;
};

static anyedge_string_value*get_string_value(anyedge_value*&value)
{
      anyedge_string_value*string_value = dynamic_cast<anyedge_string_value*>(value);
      if (!value) {
	    string_value = new anyedge_string_value();
	    delete value;
	    value = string_value;
      }
      return string_value;
}

struct vvp_fun_anyedge_state_s : public waitable_state_s {
      vvp_fun_anyedge_state_s()
      {
            for (unsigned idx = 0 ;  idx < 4 ;  idx += 1)
                  last_value_[idx] = 0;
      }

      ~vvp_fun_anyedge_state_s()
      {
            for (unsigned idx = 0 ;  idx < 4 ;  idx += 1)
                  delete last_value_[idx];
      }

      anyedge_value *last_value_[4];
};

vvp_fun_anyedge::vvp_fun_anyedge()
{
      for (unsigned idx = 0 ;  idx < 4 ;  idx += 1)
	    last_value_[idx] = 0;
}

vvp_fun_anyedge::~vvp_fun_anyedge()
{
      for (unsigned idx = 0 ;  idx < 4 ;  idx += 1)
	    delete last_value_[idx];
}

void anyedge_vec4_value::duplicate(anyedge_value*&dup)
{
      anyedge_vec4_value*dup_vec4 = get_vec4_value(dup);
      assert(dup_vec4);
      dup_vec4->set(old_bits);
}

bool anyedge_vec4_value::recv_vec4(const vvp_vector4_t&bit)
{
      bool flag = false;

      if (old_bits.size() != bit.size()) {
	    if (old_bits.size() == 0) {
		    // Special case: If we've not seen any input yet
		    // (old_bits.size()==0) then replace it will a reference
		    // vector that is 'bx. Then compare that with the input
		    // to see if we are processing a change from 'bx.
		  old_bits = vvp_vector4_t(bit.size(), BIT4_X);
		  if (old_bits.eeq(bit))
			flag = false;
		  else
			flag = true;

	    } else {
		  flag = true;
	    }

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
      }

      return flag;
}

bool anyedge_vec4_value::recv_vec4_pv(const vvp_vector4_t&bit, unsigned base,
				      unsigned vwid)
{
      vvp_vector4_t tmp = old_bits;
      if (tmp.size() == 0)
	    tmp = vvp_vector4_t(vwid, BIT4_Z);
      assert(base + bit.size()<= vwid);
      assert(tmp.size() == vwid);
      tmp.set_vec(base, bit);

      return recv_vec4(tmp);
}

void anyedge_real_value::duplicate(anyedge_value*&dup)
{
      anyedge_real_value*dup_real = get_real_value(dup);
      assert(dup_real);
      dup_real->set(old_bits);
}

bool anyedge_real_value::recv_real(double bit)
{
      if (old_bits != bit) {
	    old_bits = bit;
            return true;
      }
      return false;
}

void anyedge_string_value::duplicate(anyedge_value*&dup)
{
      anyedge_string_value*dup_string = get_string_value(dup);
      assert(dup_string);
      dup_string->set(old_bits);
}

bool anyedge_string_value::recv_string(const std::string&bit)
{
      if (old_bits != bit) {
	    old_bits = bit;
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
      anyedge_vec4_value*value = get_vec4_value(last_value_[port.port()]);
      assert(value);
      if (value->recv_vec4(bit)) {
	    run_waiting_threads_(threads_);
	    vvp_net_t*net = port.ptr();
	    net->send_vec4(bit, 0);
      }
}

void vvp_fun_anyedge_sa::recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
				      unsigned base, unsigned vwid, vvp_context_t)
{
      anyedge_vec4_value*value = get_vec4_value(last_value_[port.port()]);
      assert(value);
      if (value->recv_vec4_pv(bit, base, vwid)) {
	    run_waiting_threads_(threads_);
	    vvp_net_t*net = port.ptr();
	    net->send_vec4(bit, 0);
      }
}

void vvp_fun_anyedge_sa::recv_real(vvp_net_ptr_t port, double bit,
                                   vvp_context_t)
{
      anyedge_real_value*value = get_real_value(last_value_[port.port()]);
      assert(value);
      if (value->recv_real(bit)) {
	    run_waiting_threads_(threads_);
	    vvp_net_t*net = port.ptr();
	    net->send_vec4(vvp_vector4_t(), 0);
      }
}

void vvp_fun_anyedge_sa::recv_string(vvp_net_ptr_t port, const std::string&bit,
				     vvp_context_t)
{
      anyedge_string_value*value = get_string_value(last_value_[port.port()]);
      assert(value);
      if (value->recv_string(bit)) {
	    run_waiting_threads_(threads_);
	    vvp_net_t*net = port.ptr();
	    net->send_vec4(vvp_vector4_t(), 0);
      }
}

/*
 * An anyedge receiving an object should do nothing with it, but should
 * trigger waiting threads.
 */
void vvp_fun_anyedge_sa::recv_object(vvp_net_ptr_t port, vvp_object_t,
				     vvp_context_t)
{
      run_waiting_threads_(threads_);
      vvp_net_t*net = port.ptr();
      net->send_vec4(vvp_vector4_t(), 0);
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
	    if (last_value_[idx])
	          last_value_[idx]->duplicate(state->last_value_[idx]);
	    else if (state->last_value_[idx])
		  state->last_value_[idx]->reset();
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

            anyedge_vec4_value*value = get_vec4_value(state->last_value_[port.port()]);
            assert(value);
            if (value->recv_vec4(bit)) {
                  run_waiting_threads_(state->threads);
                  vvp_net_t*net = port.ptr();
                  net->send_vec4(bit, context);
            }
      } else {
            context = context_scope_->live_contexts;
            while (context) {
                  recv_vec4(port, bit, context);
                  context = vvp_get_next_context(context);
            }
            anyedge_vec4_value*value = get_vec4_value(last_value_[port.port()]);
            assert(value);
            value->set(bit);
      }
}

void vvp_fun_anyedge_aa::recv_real(vvp_net_ptr_t port, double bit,
                                   vvp_context_t context)
{
      if (context) {
            vvp_fun_anyedge_state_s*state = static_cast<vvp_fun_anyedge_state_s*>
                  (vvp_get_context_item(context, context_idx_));

            anyedge_real_value*value = get_real_value(state->last_value_[port.port()]);
            assert(value);
            if (value->recv_real(bit)) {
                  run_waiting_threads_(state->threads);
                  vvp_net_t*net = port.ptr();
                  net->send_vec4(vvp_vector4_t(), context);
            }
      } else {
            context = context_scope_->live_contexts;
            while (context) {
                  recv_real(port, bit, context);
                  context = vvp_get_next_context(context);
            }
            anyedge_real_value*value = get_real_value(last_value_[port.port()]);
            assert(value);
            value->set(bit);
      }
}

void vvp_fun_anyedge_aa::recv_string(vvp_net_ptr_t port, const std::string&bit,
				     vvp_context_t context)
{
      if (context) {
            vvp_fun_anyedge_state_s*state = static_cast<vvp_fun_anyedge_state_s*>
                  (vvp_get_context_item(context, context_idx_));

            anyedge_string_value*value = get_string_value(state->last_value_[port.port()]);
            assert(value);
            if (value->recv_string(bit)) {
                  run_waiting_threads_(state->threads);
                  vvp_net_t*net = port.ptr();
                  net->send_vec4(vvp_vector4_t(), context);
            }
      } else {
            context = context_scope_->live_contexts;
            while (context) {
                  recv_string(port, bit, context);
                  context = vvp_get_next_context(context);
            }
            anyedge_string_value*value = get_string_value(last_value_[port.port()]);
            assert(value);
            value->set(bit);
      }
}

vvp_fun_event_or::vvp_fun_event_or(vvp_net_t*base_net)
: base_net_(base_net)
{
}

vvp_fun_event_or::~vvp_fun_event_or()
{
}

vvp_fun_event_or_sa::vvp_fun_event_or_sa(vvp_net_t*base_net)
: vvp_fun_event_or(base_net), threads_(0)
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

void vvp_fun_event_or_sa::recv_vec4(vvp_net_ptr_t, const vvp_vector4_t&bit,
                                    vvp_context_t)
{
      run_waiting_threads_(threads_);
      base_net_->send_vec4(bit, 0);
}

vvp_fun_event_or_aa::vvp_fun_event_or_aa(vvp_net_t*base_net)
: vvp_fun_event_or(base_net)
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
            base_net_->send_vec4(bit, context);
      } else {
            context = context_scope_->live_contexts;
            while (context) {
                  recv_vec4(port, bit, context);
                  context = vvp_get_next_context(context);
            }
      }
}

vvp_named_event::vvp_named_event(__vpiHandle*h)
{
      handle_ = h;
}

vvp_named_event::~vvp_named_event()
{
}

vvp_named_event_sa::vvp_named_event_sa(__vpiHandle*h)
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
      net->send_vec4(bit, 0);

      __vpiNamedEvent*obj = dynamic_cast<__vpiNamedEvent*>(handle_);
      assert(obj);
      obj->run_vpi_callbacks();
}

vvp_named_event_aa::vvp_named_event_aa(__vpiHandle*h)
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
      net->send_vec4(bit, context);
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

      if (strcmp(type,"anyedge") == 0) {

	    free(type);

            if (vpip_peek_current_scope()->is_automatic()) {
                  fun = new vvp_fun_anyedge_aa;
            } else {
                  fun = new vvp_fun_anyedge_sa;
            }

      } else {

	    vvp_fun_edge::edge_t edge_type = vvp_edge_none;

	    if (strcmp(type,"posedge") == 0)
		  edge_type = vvp_edge_posedge;
	    else if (strcmp(type,"negedge") == 0)
		  edge_type = vvp_edge_negedge;
	    else if (strcmp(type,"edge") == 0)
		  edge_type = vvp_edge_edge;

	    assert(argc <= 4);
	    free(type);

            if (vpip_peek_current_scope()->is_automatic()) {
                  fun = new vvp_fun_edge_aa(edge_type);
            } else {
                  fun = new vvp_fun_edge_sa(edge_type);
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
      vvp_net_t*base_net = new vvp_net_t;
      if (vpip_peek_current_scope()->is_automatic()) {
            base_net->fun = new vvp_fun_event_or_aa(base_net);
      } else {
            base_net->fun = new vvp_fun_event_or_sa(base_net);
      }
      define_functor_symbol(label, base_net);
      free(label);

	/* This is a simplified version of a wide functor. We don't
	   care about the data values or what port they arrived on,
	   so we can use a single shared functor. */
      vvp_net_t*curr_net = base_net;
      for (unsigned idx = 0 ;  idx < argc ;  idx += 1) {
	    if (idx > 0 && (idx % 4) == 0) {
		  curr_net = new vvp_net_t;
		  curr_net->fun = base_net->fun;
	    }
	    input_connect(curr_net, idx % 4, argv[idx].text);
      }
      free(argv);
}

/*
 * This handles the compile of named events. This functor has no
 * inputs, it is only accessed by behavioral trigger statements, which
 * in vvp are %set instructions.
 */
void compile_named_event(char*label, char*name, bool local_flag)
{
      vvp_net_t*ptr = new vvp_net_t;

      vpiHandle obj = vpip_make_named_event(name, ptr);

      if (vpip_peek_current_scope()->is_automatic()) {
            ptr->fun = new vvp_named_event_aa(obj);
      } else {
            ptr->fun = new vvp_named_event_sa(obj);
      }
      define_functor_symbol(label, ptr);
      compile_vpi_symbol(label, obj);
      if (! local_flag) vpip_attach_to_current_scope(obj);

      free(label);
      delete[] name;
}

#ifdef CHECK_WITH_VALGRIND
void named_event_delete(__vpiHandle*handle)
{
      delete dynamic_cast<__vpiNamedEvent *>(handle);
}
#endif
