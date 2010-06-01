/*
 * Copyright (c) 2004-2009 Stephen Williams (steve@icarus.com)
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
# include  "part.h"
# include  <cstdlib>
# include  <climits>
# include  <iostream>
# include  <cassert>

struct vvp_fun_part_state_s {
      vvp_fun_part_state_s() : bitsr(0.0) {}

      vvp_vector4_t bits;
      double bitsr;
};

vvp_fun_part::vvp_fun_part(unsigned base, unsigned wid)
: base_(base), wid_(wid)
{
}

vvp_fun_part::~vvp_fun_part()
{
}

vvp_fun_part_sa::vvp_fun_part_sa(unsigned base, unsigned wid)
: vvp_fun_part(base, wid)
{
      net_ = 0;
}

vvp_fun_part_sa::~vvp_fun_part_sa()
{
}

void vvp_fun_part_sa::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                                vvp_context_t)
{
      assert(port.port() == 0);

      vvp_vector4_t tmp (wid_, BIT4_X);
      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    if (idx + base_ < bit.size())
		  tmp.set_bit(idx, bit.value(base_+idx));
      }
      if (val_ .eeq( tmp ))
	    return;

      val_ = tmp;

      if (net_ == 0) {
	    net_ = port.ptr();
	    schedule_generic(this, 0, false);
      }
}

/*
 * Handle the case that the part select node is actually fed by a part
 * select assignment. It's not exactly clear what might make this
 * happen, but is does seem to happen and this should have well
 * defined behavior.
 */
void vvp_fun_part_sa::recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
				   unsigned base, unsigned wid, unsigned vwid,
                                   vvp_context_t)
{
      assert(bit.size() == wid);

      vvp_vector4_t tmp (vwid, BIT4_Z);
      tmp.set_vec(base_, val_);
      tmp.set_vec(base, bit);
      recv_vec4(port, tmp, 0);
}

void vvp_fun_part_sa::run_run()
{
      vvp_net_t*ptr = net_;
      net_ = 0;

      vvp_send_vec4(ptr->out, val_, 0);
}

vvp_fun_part_aa::vvp_fun_part_aa(unsigned base, unsigned wid)
: vvp_fun_part(base, wid)
{
      context_scope_ = vpip_peek_context_scope();
      context_idx_ = vpip_add_item_to_context(this, context_scope_);
}

vvp_fun_part_aa::~vvp_fun_part_aa()
{
}

void vvp_fun_part_aa::alloc_instance(vvp_context_t context)
{
      vvp_set_context_item(context, context_idx_, new vvp_vector4_t);
}

void vvp_fun_part_aa::reset_instance(vvp_context_t context)
{
      vvp_vector4_t*val = static_cast<vvp_vector4_t*>
            (vvp_get_context_item(context, context_idx_));

      val->set_to_x();
}

#ifdef CHECK_WITH_VALGRIND
void vvp_fun_part_aa::free_instance(vvp_context_t context)
{
      vvp_vector4_t*val = static_cast<vvp_vector4_t*>
            (vvp_get_context_item(context, context_idx_));
      delete val;
}
#endif

void vvp_fun_part_aa::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                                vvp_context_t context)
{
      if (context) {
            assert(port.port() == 0);

            vvp_vector4_t*val = static_cast<vvp_vector4_t*>
                  (vvp_get_context_item(context, context_idx_));

            vvp_vector4_t tmp (wid_, BIT4_X);
            for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
                  if (idx + base_ < bit.size())
                        tmp.set_bit(idx, bit.value(base_+idx));
            }
            if (!val->eeq( tmp )) {
                  *val = tmp;
                  vvp_send_vec4(port.ptr()->out, tmp, context);
            }
      } else {
            context = context_scope_->live_contexts;
            while (context) {
                  recv_vec4(port, bit, context);
                  context = vvp_get_next_context(context);
            }
      }
}

/*
 * Handle the case that the part select node is actually fed by a part
 * select assignment. It's not exactly clear what might make this
 * happen, but is does seem to happen and this should have well
 * defined behavior.
 */
void vvp_fun_part_aa::recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
				   unsigned base, unsigned wid, unsigned vwid,
                                   vvp_context_t context)
{
      if (context) {
            assert(bit.size() == wid);

            vvp_vector4_t*val = static_cast<vvp_vector4_t*>
                  (vvp_get_context_item(context, context_idx_));

            vvp_vector4_t tmp (vwid, BIT4_Z);
            tmp.set_vec(base_, *val);
            tmp.set_vec(base, bit);
            recv_vec4(port, tmp, context);
      } else {
            context = context_scope_->live_contexts;
            while (context) {
                  recv_vec4_pv(port, bit, base, wid, vwid, context);
                  context = vvp_get_next_context(context);
            }
      }
}

vvp_fun_part_pv::vvp_fun_part_pv(unsigned b, unsigned w, unsigned v)
: base_(b), wid_(w), vwid_(v)
{
}

vvp_fun_part_pv::~vvp_fun_part_pv()
{
}

void vvp_fun_part_pv::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                                vvp_context_t context)
{
      assert(port.port() == 0);

      if (bit.size() != wid_) {
	    cerr << "internal error: part_pv data mismatch. "
		 << "base_=" << base_ << ", wid_=" << wid_
		 << ", vwid_=" << vwid_ << ", bit=" << bit
		 << endl;
      }
      assert(bit.size() == wid_);

      vvp_send_vec4_pv(port.ptr()->out, bit, base_, wid_, vwid_, context);
}

void vvp_fun_part_pv::recv_vec8(vvp_net_ptr_t port, const vvp_vector8_t&bit)
{
      assert(port.port() == 0);

      if (bit.size() != wid_) {
	    cerr << "internal error: part_pv (strength-aware) data mismatch. "
		 << "base_=" << base_ << ", wid_=" << wid_
		 << ", vwid_=" << vwid_ << ", bit=" << bit
		 << endl;
      }
      assert(bit.size() == wid_);

      vvp_send_vec8_pv(port.ptr()->out, bit, base_, wid_, vwid_);
}

vvp_fun_part_var::vvp_fun_part_var(unsigned w)
: wid_(w)
{
}

vvp_fun_part_var::~vvp_fun_part_var()
{
}

bool vvp_fun_part_var::recv_vec4_(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                                  int&base, vvp_vector4_t&source,
                                  vvp_vector4_t&ref)
{
      long tmp;
      switch (port.port()) {
	  case 0:
	    source = bit;
	    break;
	  case 1:
	      // INT_MIN is before the vector and is used to
	      // represent a 'bx value on the select input.
	    tmp = INT_MIN;
	      // We need a new .part/v that knows if the index is signed.
	      // For now this will work for a normal integer value.
	    vector4_to_value(bit, tmp, false);
	    if ((int)tmp == base) return false;
	    base = tmp;
	    break;
	  default:
	    fprintf(stderr, "Unsupported port type %d.\n", port.port());
	    assert(0);
	    break;
      }

      vvp_vector4_t res (wid_);

      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    int adr = base+idx;
	    if (adr < 0) continue;
	    if ((unsigned)adr >= source.size()) break;

	    res.set_bit(idx, source.value((unsigned)adr));
      }

      if (! ref.eeq(res)) {
	    ref = res;
            return true;
      }
      return false;
}

vvp_fun_part_var_sa::vvp_fun_part_var_sa(unsigned w)
: vvp_fun_part_var(w), base_(0)
{
}

vvp_fun_part_var_sa::~vvp_fun_part_var_sa()
{
}

void vvp_fun_part_var_sa::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                                    vvp_context_t)
{
      if (recv_vec4_(port, bit, base_, source_, ref_)) {
	    vvp_send_vec4(port.ptr()->out, ref_, 0);
      }
}

void vvp_fun_part_var_sa::recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
				       unsigned base, unsigned wid, unsigned vwid,
                                       vvp_context_t)
{
      assert(bit.size() == wid);

      vvp_vector4_t tmp = source_;
      if (tmp.size() == 0)
	    tmp = vvp_vector4_t(vwid);

      assert(tmp.size() == vwid);
      tmp.set_vec(base, bit);
      recv_vec4(port, tmp, 0);
}

struct vvp_fun_part_var_state_s {
      vvp_fun_part_var_state_s() : base(0) { }

      int base;
      vvp_vector4_t source;
      vvp_vector4_t ref;
};

vvp_fun_part_var_aa::vvp_fun_part_var_aa(unsigned w)
: vvp_fun_part_var(w)
{
      context_scope_ = vpip_peek_context_scope();
      context_idx_ = vpip_add_item_to_context(this, context_scope_);
}

vvp_fun_part_var_aa::~vvp_fun_part_var_aa()
{
}

void vvp_fun_part_var_aa::alloc_instance(vvp_context_t context)
{
      vvp_set_context_item(context, context_idx_, new vvp_fun_part_var_state_s);
}

void vvp_fun_part_var_aa::reset_instance(vvp_context_t context)
{
      vvp_fun_part_var_state_s*state = static_cast<vvp_fun_part_var_state_s*>
            (vvp_get_context_item(context, context_idx_));

      state->base = 0;
      state->source.set_to_x();
      state->ref.set_to_x();
}

#ifdef CHECK_WITH_VALGRIND
void vvp_fun_part_var_aa::free_instance(vvp_context_t context)
{
      vvp_fun_part_var_state_s*state = static_cast<vvp_fun_part_var_state_s*>
            (vvp_get_context_item(context, context_idx_));
      delete state;
}
#endif

void vvp_fun_part_var_aa::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                                    vvp_context_t context)
{
      if (context) {
            vvp_fun_part_var_state_s*state = static_cast<vvp_fun_part_var_state_s*>
                  (vvp_get_context_item(context, context_idx_));

            if (recv_vec4_(port, bit, state->base, state->source, state->ref)) {
                  vvp_send_vec4(port.ptr()->out, state->ref, context);
            }
      } else {
            context = context_scope_->live_contexts;
            while (context) {
                  recv_vec4(port, bit, context);
                  context = vvp_get_next_context(context);
            }
      }
}

void vvp_fun_part_var_aa::recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
				       unsigned base, unsigned wid, unsigned vwid,
                                       vvp_context_t context)
{
      if (context) {
            vvp_fun_part_var_state_s*state = static_cast<vvp_fun_part_var_state_s*>
                  (vvp_get_context_item(context, context_idx_));

            assert(bit.size() == wid);

            vvp_vector4_t tmp = state->source;
            if (tmp.size() == 0)
                  tmp = vvp_vector4_t(vwid);

            assert(tmp.size() == vwid);
            tmp.set_vec(base, bit);
            recv_vec4(port, tmp, context);
      } else {
            context = context_scope_->live_contexts;
            while (context) {
                  recv_vec4(port, bit, context);
                  context = vvp_get_next_context(context);
            }
      }
}

/*
 * Given a node functor, create a network node and link it into the
 * netlist. This form assumes nodes with a single input.
 */
void link_node_1(char*label, char*source, vvp_net_fun_t*fun)
{
      vvp_net_t*net = new vvp_net_t;
      net->fun = fun;

      define_functor_symbol(label, net);
      free(label);

      input_connect(net, 0, source);
}

void compile_part_select(char*label, char*source,
			 unsigned base, unsigned wid)
{
      vvp_fun_part*fun = 0;
      if (vpip_peek_current_scope()->is_automatic) {
            fun = new vvp_fun_part_aa(base, wid);
      } else {
            fun = new vvp_fun_part_sa(base, wid);
      }
      link_node_1(label, source, fun);
}

void compile_part_select_pv(char*label, char*source,
			    unsigned base, unsigned wid,
			    unsigned vector_wid)
{
      vvp_fun_part_pv*fun = new vvp_fun_part_pv(base, wid, vector_wid);
      link_node_1(label, source, fun);
}

void compile_part_select_var(char*label, char*source, char*var,
			     unsigned wid)
{
      vvp_fun_part_var*fun = 0;
      if (vpip_peek_current_scope()->is_automatic) {
            fun = new vvp_fun_part_var_aa(wid);
      } else {
            fun = new vvp_fun_part_var_sa(wid);
      }
      vvp_net_t*net = new vvp_net_t;
      net->fun = fun;

      define_functor_symbol(label, net);
      free(label);

      input_connect(net, 0, source);
      input_connect(net, 1, var);
}
