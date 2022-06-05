/*
 * Copyright (c) 2004-2020 Stephen Williams (steve@icarus.com)
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
# include  "vvp_net.h"
# include  "vvp_net_sig.h"
# include  "statistics.h"
# include  "vpi_priv.h"
# include  <vector>
# include  <cassert>
#ifdef CHECK_WITH_VALGRIND
# include  <valgrind/memcheck.h>
# include  <map>
#endif

# include  <iostream>

using namespace std;

/*
 * The filter_mask_ method takes as an input the value to propagate,
 * the mask of what is being forced, and returns a propagation
 * mode. In the process, it may update the filtered output value.
 *
 * The input value is the subvector "val" that is placed as "base" in
 * the output. The val may be shorter then the target vector.
 *
 * The "force" vector in the value being force, with the force_mask_
 * member a bit mask of which parts of the force vector really apply.
 */
template <class T> vvp_net_fil_t::prop_t vvp_net_fil_t::filter_mask_(const T&val, const T&force, T&filter, unsigned base)
{
      if (!test_force_mask_is_zero()) {
	      // Some bits are being forced. Go through the
	      // force_mask_ and force value to see which bits are
	      // propagated and which are kept from the forced
	      // value. Update the filter with the filtered result and
	      // return REPL to indicate that some bits have changed,
	      // or STOP if no bits change.
	    bool propagate_flag = force_propagate_;
	    force_propagate_ = false;
	    assert(force_mask_.size() == force.size());
	    assert((base+val.size()) <= force_mask_.size());

	    filter = val;
	    for (unsigned idx = 0 ; idx < val.size() ; idx += 1) {
		  if (force_mask_.value(base+idx))
			filter.set_bit(idx, force.value(base+idx));
		  else
			propagate_flag = true;
	    }

	    if (propagate_flag) {
		  run_vpi_callbacks();
		  return REPL;
	    } else {
		  return STOP;
	    }

      } else {
	    run_vpi_callbacks();
	    return PROP;
      }
}

template <class T> vvp_net_fil_t::prop_t vvp_net_fil_t::filter_mask_(T&val, T force)
{

      if (test_force_mask(0)) {
	    val = force;
	    run_vpi_callbacks();
	    return REPL;
      }
      run_vpi_callbacks();
      return PROP;
}

template <class T> vvp_net_fil_t::prop_t vvp_net_fil_t::filter_input_mask_(const T&val, const T&force, T&rep) const
{
      if (test_force_mask_is_zero())
	    return PROP;

      assert(force_mask_.size() == force.size());

      rep = val;
      for (unsigned idx = 0 ; idx < val.size() ; idx += 1) {
	    if (force_mask_.value(idx))
		  rep.set_bit(idx, force.value(idx));
      }

      return REPL;
}

vvp_signal_value::~vvp_signal_value()
{
}

double vvp_signal_value::real_value() const
{
      assert(0);
      return 0;
}

void vvp_net_t::force_vec4(const vvp_vector4_t&val, const vvp_vector2_t&mask)
{
      assert(fil);
      fil->force_fil_vec4(val, mask);
      fun->force_flag(false);
      vvp_send_vec4(out_, val, 0);
}

void vvp_net_t::force_vec8(const vvp_vector8_t&val, const vvp_vector2_t&mask)
{
      assert(fil);
      fil->force_fil_vec8(val, mask);
      fun->force_flag(false);
      vvp_send_vec8(out_, val);
}

void vvp_net_t::force_real(double val, const vvp_vector2_t&mask)
{
      assert(fil);
      fil->force_fil_real(val, mask);
      fun->force_flag(false);
      vvp_send_real(out_, val, 0);
}

/* **** vvp_fun_signal methods **** */

vvp_fun_signal_base::vvp_fun_signal_base()
{
      continuous_assign_active_ = false;
      needs_init_ = true;
      cassign_link = 0;
      count_functors_sig += 1;
}

vvp_fun_signal4_sa::vvp_fun_signal4_sa(unsigned wid, vvp_bit4_t init)
: bits4_(wid, init)
{
}

/*
 * Nets simply reflect their input to their output.
 *
 * NOTE: It is a quirk of vvp_fun_signal that it has an initial value
 * that needs to be propagated, but after that it only needs to
 * propagate if the value changes. Eliminating duplicate propagations
 * should improve performance, but has the quirk that an input that
 * matches the initial value might not be propagated. The hack used
 * herein is to keep a "needs_init_" flag that is turned false after
 * the first propagation, and forces the first propagation to happen
 * even if it matches the initial value.
 */
void vvp_fun_signal4_sa::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                                   vvp_context_t)
{
      switch (ptr.port()) {
	  case 0: // Normal input (feed from net, or set from process)
	      /* If we don't have a continuous assign mask then just
		 copy the bits, otherwise we need to see if there are
		 any holes in the mask so we can set those bits. */
	    if (assign_mask_.size() == 0) {
                  if (needs_init_ || !bits4_.eeq(bit)) {
			assert(bit.size() == bits4_.size());
			bits4_ = bit;
			needs_init_ = false;
			ptr.ptr()->send_vec4(bits4_, 0);
		  }
	    } else {
		  bool changed = false;
		  assert(bits4_.size() == assign_mask_.size());
		  for (unsigned idx = 0 ;  idx < bit.size() ;  idx += 1) {
			if (idx >= bits4_.size()) break;
			if (assign_mask_.value(idx)) continue;
			bits4_.set_bit(idx, bit.value(idx));
			changed = true;
		  }
		  if (changed) {
			needs_init_ = false;
			ptr.ptr()->send_vec4(bits4_, 0);
		  }
	    }
	    break;

	  case 1: // Continuous assign value
	      // Handle the simple case of the linked source being wider
	      // than this signal. Note we don't yet support the case of
	      // the linked source being narrower than this signal, or
	      // the case of an expression being assigned.
	    bits4_ = coerce_to_width(bit, bits4_.size());
	    assign_mask_ = vvp_vector2_t(vvp_vector2_t::FILL1, bits4_.size());
	    ptr.ptr()->send_vec4(bits4_, 0);
	    break;

	  default:
	    fprintf(stderr, "Unsupported port type %u.\n", ptr.port());
	    assert(0);
	    break;
      }
}

void vvp_fun_signal4_sa::recv_vec8(vvp_net_ptr_t ptr, const vvp_vector8_t&bit)
{
      recv_vec4(ptr, reduce4(bit), 0);
}

void vvp_fun_signal4_sa::recv_vec4_pv(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
				      unsigned base, unsigned vwid, vvp_context_t)
{
      assert(bits4_.size() == vwid);
      unsigned wid = bit.size();

      switch (ptr.port()) {
	  case 0: // Normal input
	    if (assign_mask_.size() == 0) {
                  for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
			if (base+idx >= bits4_.size()) break;
			bits4_.set_bit(base+idx, bit.value(idx));
		  }
		  needs_init_ = false;
		  ptr.ptr()->send_vec4(bits4_,0);
	    } else {
		  bool changed = false;
		  assert(bits4_.size() == assign_mask_.size());
		  for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
			if (base+idx >= bits4_.size()) break;
			if (assign_mask_.value(base+idx)) continue;
			bits4_.set_bit(base+idx, bit.value(idx));
			changed = true;
		  }
		  if (changed) {
			needs_init_ = false;
			ptr.ptr()->send_vec4(bits4_,0);
		  }
	    }
	    break;

	  case 1: // Continuous assign value
	    if (assign_mask_.size() == 0)
		  assign_mask_ = vvp_vector2_t(vvp_vector2_t::FILL0, bits4_.size());
	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		  if (base+idx >= bits4_.size())
			break;
		  bits4_.set_bit(base+idx, bit.value(idx));
		  assign_mask_.set_bit(base+idx, 1);
	    }
	    ptr.ptr()->send_vec4(bits4_,0);
	    break;

	  default:
	    fprintf(stderr, "Unsupported port type %u.\n", ptr.port());
	    assert(0);
	    break;
      }
}

void vvp_fun_signal4_sa::recv_vec8_pv(vvp_net_ptr_t ptr, const vvp_vector8_t&bit,
				      unsigned base, unsigned vwid)
{
      recv_vec4_pv(ptr, reduce4(bit), base, vwid, 0);
}

void vvp_fun_signal_base::deassign()
{
      continuous_assign_active_ = false;
      assign_mask_ = vvp_vector2_t();
}

void vvp_fun_signal_base::deassign_pv(unsigned base, unsigned wid)
{
      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    assign_mask_.set_bit(base+idx, 0);
      }

      if (assign_mask_.is_zero()) {
	    assign_mask_ = vvp_vector2_t();
      }
}

void automatic_signal_base::release(vvp_net_ptr_t,bool)
{
      assert(0);
}

void automatic_signal_base::release_pv(vvp_net_ptr_t,unsigned,unsigned,bool)
{
      assert(0);
}

unsigned automatic_signal_base::filter_size() const
{
      assert(0);
      return(0);
}
void automatic_signal_base::force_fil_vec4(const vvp_vector4_t&, const vvp_vector2_t&)
{
      assert(0);
}
void automatic_signal_base::force_fil_vec8(const vvp_vector8_t&, const vvp_vector2_t&)
{
      assert(0);
}
void automatic_signal_base::force_fil_real(double, const vvp_vector2_t&)
{
      assert(0);
}
void automatic_signal_base::get_value(struct t_vpi_value*)
{
      assert(0);
}

const vvp_vector4_t& vvp_fun_signal4_sa::vec4_unfiltered_value() const
{
      return bits4_;
}

vvp_fun_signal4_aa::vvp_fun_signal4_aa(unsigned wid, vvp_bit4_t init)
{
      context_idx_ = vpip_add_item_to_context(this, vpip_peek_context_scope());
      size_ = wid;
      init_ = init;
}

vvp_fun_signal4_aa::~vvp_fun_signal4_aa()
{
      assert(0);
}

void vvp_fun_signal4_aa::alloc_instance(vvp_context_t context)
{
      vvp_set_context_item(context, context_idx_, new vvp_vector4_t(size_, init_));
}

void vvp_fun_signal4_aa::reset_instance(vvp_context_t context)
{
      vvp_vector4_t*bits = static_cast<vvp_vector4_t*>
            (vvp_get_context_item(context, context_idx_));

      bits->fill_bits(init_);
}

#ifdef CHECK_WITH_VALGRIND
void vvp_fun_signal4_aa::free_instance(vvp_context_t context)
{
      vvp_vector4_t*bits = static_cast<vvp_vector4_t*>
            (vvp_get_context_item(context, context_idx_));
      delete bits;
}
#endif

/*
 * Continuous and forced assignments are not permitted on automatic
 * variables. So we only expect to receive on port 0.
 */
void vvp_fun_signal4_aa::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                                   vvp_context_t context)
{
      assert(ptr.port() == 0);
      assert(context);

      vvp_vector4_t*bits4 = static_cast<vvp_vector4_t*>
            (vvp_get_context_item(context, context_idx_));

      if (!bits4->eeq(bit)) {
            *bits4 = bit;
            ptr.ptr()->send_vec4(*bits4, context);
      }
}

void vvp_fun_signal4_aa::recv_vec4_pv(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
				      unsigned base, unsigned vwid, vvp_context_t context)
{
      assert(ptr.port() == 0);
      assert(size_ == vwid);
      assert(context);

      vvp_vector4_t*bits4 = static_cast<vvp_vector4_t*>
            (vvp_get_context_item(context, context_idx_));

      unsigned wid = bit.size();
      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
            if (base+idx >= bits4->size()) break;
            bits4->set_bit(base+idx, bit.value(idx));
      }
      ptr.ptr()->send_vec4(*bits4, context);
}

unsigned vvp_fun_signal4_aa::value_size() const
{
      return size_;
}

vvp_bit4_t vvp_fun_signal4_aa::value(unsigned idx) const
{
      vvp_vector4_t*bits4 = static_cast<vvp_vector4_t*>
            (vthread_get_rd_context_item(context_idx_));

      return bits4->value(idx);
}

vvp_scalar_t vvp_fun_signal4_aa::scalar_value(unsigned idx) const
{
      vvp_vector4_t*bits4 = static_cast<vvp_vector4_t*>
            (vthread_get_rd_context_item(context_idx_));

      return vvp_scalar_t(bits4->value(idx), 6, 6);
}

void vvp_fun_signal4_aa::vec4_value(vvp_vector4_t&val) const
{
      vvp_vector4_t*bits4 = static_cast<vvp_vector4_t*>
            (vthread_get_rd_context_item(context_idx_));

      val = *bits4;
}

const vvp_vector4_t&vvp_fun_signal4_aa::vec4_unfiltered_value() const
{
      vvp_vector4_t*bits4 = static_cast<vvp_vector4_t*>
            (vthread_get_rd_context_item(context_idx_));

      return *bits4;
}

void vvp_fun_signal4_aa::operator delete(void*)
{
      assert(0);
}

/*
 * Testing for equality, we want a bitwise test instead of an
 * arithmetic test because we want to treat for example -0 different
 * from +0.
 */
bool bits_equal(double a, double b)
{
      return memcmp(&a, &b, sizeof a) == 0;
}

vvp_fun_signal_real_sa::vvp_fun_signal_real_sa()
{
      bits_ = 0.0;
}

double vvp_fun_signal_real_sa::real_unfiltered_value() const
{
      return bits_;
}

void vvp_fun_signal_real_sa::recv_real(vvp_net_ptr_t ptr, double bit,
                                       vvp_context_t)
{
      switch (ptr.port()) {
	  case 0:
	    if (!continuous_assign_active_) {
                  if (needs_init_ || !bits_equal(bits_, bit)) {
			bits_ = bit;
			needs_init_ = false;
			ptr.ptr()->send_real(bit, 0);
		  }
	    }
	    break;

	  case 1: // Continuous assign value
	    continuous_assign_active_ = true;
	    bits_ = bit;
	    ptr.ptr()->send_real(bit, 0);
	    break;

	  default:
	    fprintf(stderr, "Unsupported port type %u.\n", ptr.port());
	    assert(0);
	    break;
      }
}

vvp_fun_signal_real_aa::vvp_fun_signal_real_aa()
{
      context_idx_ = vpip_add_item_to_context(this, vpip_peek_context_scope());
}

vvp_fun_signal_real_aa::~vvp_fun_signal_real_aa()
{
      assert(0);
}

void vvp_fun_signal_real_aa::alloc_instance(vvp_context_t context)
{
      double*bits = new double;
      vvp_set_context_item(context, context_idx_, bits);

      *bits = 0.0;
}

void vvp_fun_signal_real_aa::reset_instance(vvp_context_t context)
{
      double*bits = static_cast<double*>
            (vvp_get_context_item(context, context_idx_));

      *bits = 0.0;
}

#ifdef CHECK_WITH_VALGRIND
void vvp_fun_signal_real_aa::free_instance(vvp_context_t context)
{
      double*bits = static_cast<double*>
            (vvp_get_context_item(context, context_idx_));
      delete bits;
}
#endif

double vvp_fun_signal_real_aa::real_unfiltered_value() const
{
      double*bits = static_cast<double*>
            (vthread_get_rd_context_item(context_idx_));

      return *bits;
}

double vvp_fun_signal_real_aa::real_value() const
{
      return real_unfiltered_value();
}

void vvp_fun_signal_real_aa::recv_real(vvp_net_ptr_t ptr, double bit,
                                       vvp_context_t context)
{
      assert(ptr.port() == 0);
      assert(context);

      double*bits = static_cast<double*>
            (vvp_get_context_item(context, context_idx_));

      if (!bits_equal(*bits,bit)) {
            *bits = bit;
            ptr.ptr()->send_real(bit, context);
      }
}

unsigned vvp_fun_signal_real_aa::value_size() const
{
      assert(0);
      return 1;
}

vvp_bit4_t vvp_fun_signal_real_aa::value(unsigned) const
{
      assert(0);
      return BIT4_X;
}

vvp_scalar_t vvp_fun_signal_real_aa::scalar_value(unsigned) const
{
      assert(0);
      return vvp_scalar_t();
}

void vvp_fun_signal_real_aa::vec4_value(vvp_vector4_t&) const
{
      assert(0);
}

void* vvp_fun_signal_real_aa::operator new(std::size_t size)
{
      return vvp_net_fun_t::heap_.alloc(size);
}

void vvp_fun_signal_real_aa::operator delete(void*)
{
      assert(0);
}


vvp_fun_signal_string_sa::vvp_fun_signal_string_sa()
{
}

void vvp_fun_signal_string_sa::recv_string(vvp_net_ptr_t ptr, const std::string&bit,
					   vvp_context_t)
{
      assert(ptr.port() == 0);

      if (needs_init_ || value_ != bit) {
	    value_ = bit;
	    needs_init_ = false;

	    ptr.ptr()->send_string(bit, 0);
      }
}

const string& vvp_fun_signal_string_sa::get_string() const
{
      return value_;
}

vvp_fun_signal_string_aa::vvp_fun_signal_string_aa()
{
      context_idx_ = vpip_add_item_to_context(this, vpip_peek_context_scope());
}

vvp_fun_signal_string_aa::~vvp_fun_signal_string_aa()
{
      assert(0);
}

void vvp_fun_signal_string_aa::alloc_instance(vvp_context_t context)
{
      string*bits = new std::string;
      vvp_set_context_item(context, context_idx_, bits);
      *bits = "";
}

void vvp_fun_signal_string_aa::reset_instance(vvp_context_t context)
{
      string*bits = static_cast<std::string*>
	    (vvp_get_context_item(context, context_idx_));
      *bits = "";
}

#ifdef CHECK_WITH_VALGRIND
void vvp_fun_signal_string_aa::free_instance(vvp_context_t context)
{
      string*bits = static_cast<std::string*>
            (vvp_get_context_item(context, context_idx_));
      delete bits;
}
#endif

void vvp_fun_signal_string_aa::recv_string(vvp_net_ptr_t ptr, const std::string&bit, vvp_context_t context)
{
      assert(ptr.port() == 0);
      assert(context);

      string*bits = static_cast<std::string*>
	    (vvp_get_context_item(context, context_idx_));

      if (*bits != bit) {
	    *bits = bit;
	    ptr.ptr()->send_string(bit, context);
      }
}

unsigned vvp_fun_signal_string_aa::value_size() const
{
      assert(0);
      return 1;
}

vvp_bit4_t vvp_fun_signal_string_aa::value(unsigned) const
{
      assert(0);
      return BIT4_X;
}

vvp_scalar_t vvp_fun_signal_string_aa::scalar_value(unsigned) const
{
      assert(0);
      return vvp_scalar_t();
}

void vvp_fun_signal_string_aa::vec4_value(vvp_vector4_t&) const
{
      assert(0);
}

double vvp_fun_signal_string_aa::real_value() const
{
      assert(0);
      return 0.0;
}

const std::string& vvp_fun_signal_string_aa::get_string() const
{
      string*bits = static_cast<std::string*>
            (vthread_get_rd_context_item(context_idx_));

      return *bits;
}

void* vvp_fun_signal_string_aa::operator new(std::size_t size)
{
      return vvp_net_fun_t::heap_.alloc(size);
}

void vvp_fun_signal_string_aa::operator delete(void*)
{
      assert(0);
}

  /* OBJECT signals */

vvp_fun_signal_object_sa::vvp_fun_signal_object_sa(unsigned size)
: vvp_fun_signal_object(size)
{
}

#ifdef CHECK_WITH_VALGRIND
void vvp_fun_signal_object_aa::free_instance(vvp_context_t context)
{
      vvp_object_t*bits = static_cast<vvp_object_t*>
            (vvp_get_context_item(context, context_idx_));
      delete bits;
}
#endif

void vvp_fun_signal_object_sa::recv_object(vvp_net_ptr_t ptr, vvp_object_t bit,
					   vvp_context_t)
{
      assert(ptr.port() == 0);

      if (needs_init_ || value_ != bit) {
	    value_ = bit;
	    needs_init_ = false;

	    ptr.ptr()->send_object(bit, 0);
      }
}

vvp_object_t vvp_fun_signal_object_sa::get_object() const
{
      return value_;
}

vvp_fun_signal_object_aa::vvp_fun_signal_object_aa(unsigned size)
: vvp_fun_signal_object(size)
{
      context_idx_ = vpip_add_item_to_context(this, vpip_peek_context_scope());
}

vvp_fun_signal_object_aa::~vvp_fun_signal_object_aa()
{
      assert(0);
}

void vvp_fun_signal_object_aa::alloc_instance(vvp_context_t context)
{
      vvp_object_t*bits = new vvp_object_t;
      vvp_set_context_item(context, context_idx_, bits);
      bits->reset();
}

void vvp_fun_signal_object_aa::reset_instance(vvp_context_t context)
{
      vvp_object_t*bits = static_cast<vvp_object_t*>
	    (vvp_get_context_item(context, context_idx_));
      bits->reset();
}

vvp_object_t vvp_fun_signal_object_aa::get_object() const
{
      vvp_object_t*bits = static_cast<vvp_object_t*>
	    (vthread_get_rd_context_item(context_idx_));
      return *bits;
}

void vvp_fun_signal_object_aa::recv_object(vvp_net_ptr_t ptr, vvp_object_t bit,
					   vvp_context_t context)
{
      assert(ptr.port() == 0);
      assert(context);

      vvp_object_t*bits = static_cast<vvp_object_t*>
	    (vvp_get_context_item(context, context_idx_));

      if (*bits != bit) {
	    *bits = bit;
	    ptr.ptr()->send_object(bit, context);
      }
}

unsigned vvp_fun_signal_object_aa::value_size() const
{
      return 1;
}

vvp_bit4_t vvp_fun_signal_object_aa::value(unsigned) const
{
      assert(0);
      return BIT4_X;
}

vvp_scalar_t vvp_fun_signal_object_aa::scalar_value(unsigned) const
{
      assert(0);
      return vvp_scalar_t();
}

void vvp_fun_signal_object_aa::vec4_value(vvp_vector4_t&) const
{
      assert(0);
}

void* vvp_fun_signal_object_aa::operator new(std::size_t size)
{
      return vvp_net_fun_t::heap_.alloc(size);
}

void vvp_fun_signal_object_aa::operator delete(void*)
{
      assert(0);
}

  /* **** */

vvp_fun_force::vvp_fun_force()
{
}

vvp_fun_force::~vvp_fun_force()
{
}

void vvp_fun_force::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
			      vvp_context_t)
{
      assert(ptr.port() == 0);
      vvp_net_t*net = ptr.ptr();

      vvp_net_t*dst = net->port[3].ptr();
      assert(dst->fil);

      dst->force_vec4(coerce_to_width(bit, dst->fil->filter_size()), vvp_vector2_t(vvp_vector2_t::FILL1, dst->fil->filter_size()));
}

void vvp_fun_force::recv_real(vvp_net_ptr_t ptr, double bit, vvp_context_t)
{
      assert(ptr.port() == 0);
      vvp_net_t*net = ptr.ptr();
      vvp_net_t*dst = net->port[3].ptr();
      dst->force_real(bit, vvp_vector2_t(vvp_vector2_t::FILL1, 1));
}

vvp_wire_base::vvp_wire_base()
{
}

vvp_wire_base::~vvp_wire_base()
{
}

vvp_bit4_t vvp_wire_base::driven_value(unsigned) const
{
      assert(0);
      return BIT4_X;
}

bool vvp_wire_base::is_forced(unsigned) const
{
      assert(0);
      return false;
}

vvp_wire_vec4::vvp_wire_vec4(unsigned wid, vvp_bit4_t init)
: bits4_(wid, init)
{
      needs_init_ = true;
}

vvp_net_fil_t::prop_t vvp_wire_vec4::filter_vec4(const vvp_vector4_t&bit, vvp_vector4_t&rep,
						 unsigned base, unsigned vwid)
{
	// Special case! the input bit is 0 wid. Interpret this as a
	// vector of BIT4_X to match the width of the bits4_ vector.
	// FIXME! This is a hack to work around some buggy gate
	// implementations! This should be removed!
      if (base==0 && vwid==0) {
	    vvp_vector4_t tmp (bits4_.size(), BIT4_X);
	    if (bits4_ .eeq(tmp) && !needs_init_) return STOP;
	    bits4_ = tmp;
	    needs_init_ = false;
	    return filter_mask_(tmp, force4_, rep, 0);
      }

      if (vwid != bits4_.size()) {
	    cerr << "Internal error: Input vector expected width="
		 << bits4_.size() << ", got "
		 << "bit=" << bit << ", base=" << base << ", vwid=" << vwid
		 << endl;
      }
      assert(bits4_.size() == vwid);

	// Keep track of the value being driven from this net, even if
	// it is not ultimately what survives the force filter.
      if (base==0 && bit.size()==vwid) {
	    if (bits4_ .eeq( bit ) && !needs_init_) return STOP;
	    bits4_ = bit;
      } else {
	    bool rc = bits4_.set_vec(base, bit);
	    if (rc == false && !needs_init_) return STOP;
      }

      needs_init_ = false;
      return filter_mask_(bit, force4_, rep, base);
}

vvp_net_fil_t::prop_t vvp_wire_vec4::filter_vec8(const vvp_vector8_t&bit,
                                                 vvp_vector8_t&rep,
                                                 unsigned base,
                                                 unsigned vwid)
{
      assert(bits4_.size() == vwid);

	// Keep track of the value being driven from this net, even if
	// it is not ultimately what survives the force filter.
      vvp_vector4_t bit4 (reduce4(bit));
      if (base==0 && bit4.size()==vwid) {
	    if (bits4_ .eeq( bit4 ) && !needs_init_) return STOP;
	    bits4_ = bit4;
      } else {
	    bool rc = bits4_.set_vec(base, bit4);
	    if (rc == false && !needs_init_) return STOP;
      }

      needs_init_ = false;
      return filter_mask_(bit, vvp_vector8_t(force4_,6,6), rep, base);
}

unsigned vvp_wire_vec4::filter_size() const
{
      return bits4_.size();
}

void vvp_wire_vec4::force_fil_vec4(const vvp_vector4_t&val, const vvp_vector2_t&mask)
{
      force_mask(mask);

      if (force4_.size() == 0) {
	    force4_ = val;
      } else {
	    for (unsigned idx = 0; idx < mask.size() ; idx += 1) {
		  if (mask.value(idx) == 0)
			continue;

		  force4_.set_bit(idx, val.value(idx));
	    }
      }
      run_vpi_callbacks();
}

void vvp_wire_vec4::force_fil_vec8(const vvp_vector8_t&, const vvp_vector2_t&)
{
      assert(0);
}

void vvp_wire_vec4::force_fil_real(double, const vvp_vector2_t&)
{
      assert(0);
}

void vvp_wire_vec4::release(vvp_net_ptr_t ptr, bool net_flag)
{
      vvp_vector2_t mask (vvp_vector2_t::FILL1, bits4_.size());
      if (net_flag) {
	      // Wires revert to their unforced value after release.
            release_mask(mask);
	    needs_init_ = ! force4_ .eeq(bits4_);
	    ptr.ptr()->send_vec4(bits4_, 0);
	    run_vpi_callbacks();
      } else {
	      // Variables keep the current value.
	    vvp_vector4_t res (bits4_.size());
	    for (unsigned idx=0; idx<bits4_.size(); idx += 1)
		  res.set_bit(idx,value(idx));
            release_mask(mask);
	    ptr.ptr()->fun->recv_vec4(ptr, res, 0);
      }
}

void vvp_wire_vec4::release_pv(vvp_net_ptr_t ptr, unsigned base, unsigned wid, bool net_flag)
{
      assert(bits4_.size() >= base + wid);

      vvp_vector2_t mask (vvp_vector2_t::FILL0, bits4_.size());
      for (unsigned idx = 0 ; idx < wid ; idx += 1)
	    mask.set_bit(base+idx, 1);

      if (net_flag) {
	      // Wires revert to their unforced value after release.
	    release_mask(mask);
	    needs_init_ = ! force4_.subvalue(base,wid) .eeq(bits4_.subvalue(base,wid));
	    ptr.ptr()->send_vec4_pv(bits4_.subvalue(base,wid),
				    base, bits4_.size(), 0);
	    run_vpi_callbacks();
      } else {
	      // Variables keep the current value.
	    vvp_vector4_t res (wid);
	    for (unsigned idx=0; idx<wid; idx += 1)
		  res.set_bit(idx,value(base+idx));
	    release_mask(mask);
	    ptr.ptr()->fun->recv_vec4_pv(ptr, res, base, bits4_.size(), 0);
      }
}

unsigned vvp_wire_vec4::value_size() const
{
      return bits4_.size();
}

vvp_bit4_t vvp_wire_vec4::filtered_value_(unsigned idx) const
{
      if (test_force_mask(idx))
	    return force4_.value(idx);
      else
	    return bits4_.value(idx);
}

vvp_bit4_t vvp_wire_vec4::value(unsigned idx) const
{
      return filtered_value_(idx);
}

vvp_scalar_t vvp_wire_vec4::scalar_value(unsigned idx) const
{
      return vvp_scalar_t(value(idx),6,6);
}

void vvp_wire_vec4::vec4_value(vvp_vector4_t&val) const
{
      val = bits4_;
      if (test_force_mask_is_zero())
	    return;

      for (unsigned idx = 0 ; idx < bits4_.size() ; idx += 1)
	    val.set_bit(idx, filtered_value_(idx));
}

vvp_bit4_t vvp_wire_vec4::driven_value(unsigned idx) const
{
      return bits4_.value(idx);
}

bool vvp_wire_vec4::is_forced(unsigned idx) const
{
      return test_force_mask(idx);
}

vvp_wire_vec8::vvp_wire_vec8(unsigned wid)
: bits8_(wid)
{
      needs_init_ = true;
}

vvp_net_fil_t::prop_t vvp_wire_vec8::filter_vec4(const vvp_vector4_t&bit,
                                                 vvp_vector4_t&rep,
                                                 unsigned base,
                                                 unsigned vwid)
{
	// For now there is no support for a non-zero base.
      assert(0 == base);
      assert(bits8_.size() == vwid);
      assert(bits8_.size() == bit.size());
	// QUESTION: Is it really correct to propagate a vec4 if this
	// is a vec8 node? In fact, it is really possible for a vec4
	// value to get through to a vec8 filter?
      vvp_vector8_t rep8;
      prop_t rc = filter_vec8(vvp_vector8_t(bit,6,6), rep8, 0, vwid);
      if (rc == REPL)
	    rep = reduce4(rep8);

      needs_init_ = false;
      return rc;
}

vvp_net_fil_t::prop_t vvp_wire_vec8::filter_vec8(const vvp_vector8_t&bit, vvp_vector8_t&rep, unsigned base, unsigned vwid)
{
      assert(vwid == bits8_.size());
	// Keep track of the value being driven from this net, even if
	// it is not ultimately what survives the force filter.
      if (base==0 && bit.size()==vwid) {
	    bits8_ = bit;
      } else {
	    if (bits8_.size() == 0)
		  bits8_ = vvp_vector8_t(vwid);
	    assert(bits8_.size() == vwid);
	    bits8_.set_vec(base, bit);
      }
      needs_init_ = false;
      return filter_mask_(bit, force8_, rep, base);
}

vvp_net_fil_t::prop_t vvp_wire_vec8::filter_input_vec8(const vvp_vector8_t&bit, vvp_vector8_t&rep) const
{
      return filter_input_mask_(bit, force8_, rep);
}

unsigned vvp_wire_vec8::filter_size() const
{
      return bits8_.size();
}

void vvp_wire_vec8::force_fil_vec4(const vvp_vector4_t&val, const vvp_vector2_t&mask)
{
      force_fil_vec8(vvp_vector8_t(val,6,6), mask);
}

void vvp_wire_vec8::force_fil_vec8(const vvp_vector8_t&val, const vvp_vector2_t&mask)
{
      force_mask(mask);

      if (force8_.size() == 0) {
	    force8_ = val;
      } else {
	    for (unsigned idx = 0; idx < mask.size() ; idx += 1) {
		  if (mask.value(idx) == 0)
			continue;

		  force8_.set_bit(idx, val.value(idx));
	    }
      }
      run_vpi_callbacks();
}

void vvp_wire_vec8::force_fil_real(double, const vvp_vector2_t&)
{
      assert(0);
}

void vvp_wire_vec8::release(vvp_net_ptr_t ptr, bool net_flag)
{
	// Wires revert to their unforced value after release.
      vvp_vector2_t mask (vvp_vector2_t::FILL1, bits8_.size());
      release_mask(mask);
      if (net_flag) {
	    needs_init_ = !force8_ .eeq(bits8_);
	    ptr.ptr()->send_vec8(bits8_);
      } else {
	// Variable do not know about strength so this should not be able
	// to happen. If for some reason it can then it should not be too
	// hard to fix this code like was done for vvp_wire_vec4 above.
	    assert(0);
//	    ptr.ptr()->fun->recv_vec8(ptr, force8_);
      }
}

void vvp_wire_vec8::release_pv(vvp_net_ptr_t ptr, unsigned base, unsigned wid, bool net_flag)
{
      assert(bits8_.size() >= base + wid);

      vvp_vector2_t mask (vvp_vector2_t::FILL0, bits8_.size());
      for (unsigned idx = 0 ; idx < wid ; idx += 1)
	    mask.set_bit(base+idx, 1);

      release_mask(mask);

      if (net_flag) {
	    needs_init_ = !force8_.subvalue(base,wid) .eeq((bits8_.subvalue(base,wid)));
	    ptr.ptr()->send_vec8_pv(bits8_.subvalue(base,wid),
				    base, bits8_.size());
	    run_vpi_callbacks();
      } else {
	// Variable do not know about strength so this should not be able
	// to happen. If for some reason it can then it should not be too
	// hard to fix this code like was done for vvp_wire_vec4 above.
	    assert(0);
//	    ptr.ptr()->fun->recv_vec8_pv(ptr, force8_.subvalue(base,wid),
//					 base, force8_.size());
      }
}

unsigned vvp_wire_vec8::value_size() const
{
      return bits8_.size();
}

vvp_scalar_t vvp_wire_vec8::filtered_value_(unsigned idx) const
{
      if (test_force_mask(idx))
	    return force8_.value(idx);
      else
	    return bits8_.value(idx);
}

vvp_bit4_t vvp_wire_vec8::value(unsigned idx) const
{
      return filtered_value_(idx).value();
}

vvp_scalar_t vvp_wire_vec8::scalar_value(unsigned idx) const
{
      return filtered_value_(idx);
}

vvp_vector8_t vvp_wire_vec8::vec8_value() const
{
      vvp_vector8_t tmp = bits8_;
      for (unsigned idx = 0 ; idx < bits8_.size() ; idx += 1)
	    tmp.set_bit(idx, filtered_value_(idx));
      return tmp;
}

void vvp_wire_vec8::vec4_value(vvp_vector4_t&val) const
{
      val = reduce4(vec8_value());
}

vvp_bit4_t vvp_wire_vec8::driven_value(unsigned idx) const
{
      return bits8_.value(idx).value();
}

bool vvp_wire_vec8::is_forced(unsigned idx) const
{
      return test_force_mask(idx);
}

vvp_wire_real::vvp_wire_real()
: bit_(0.0), force_(0.0)
{
}

vvp_net_fil_t::prop_t vvp_wire_real::filter_real(double&bit)
{
      bit_ = bit;
      return filter_mask_(bit, force_);
}

unsigned vvp_wire_real::filter_size() const
{
      assert(0);
      return 0;
}

void vvp_wire_real::force_fil_vec4(const vvp_vector4_t&, const vvp_vector2_t&)
{
      assert(0);
}

void vvp_wire_real::force_fil_vec8(const vvp_vector8_t&, const vvp_vector2_t&)
{
      assert(0);
}

void vvp_wire_real::force_fil_real(double val, const vvp_vector2_t&mask)
{
      force_mask(mask);
      if (mask.value(0))
	    force_ = val;

      run_vpi_callbacks();
}

void vvp_wire_real::release(vvp_net_ptr_t ptr, bool net_flag)
{
      vvp_vector2_t mask (vvp_vector2_t::FILL1, 1);
      if (net_flag) {
	      // Wires revert to their unforced value after release.
	    release_mask(mask);
	    ptr.ptr()->send_real(bit_, 0);
      } else {
	      // Variables keep the current value.
	    double res =  real_value();
	    release_mask(mask);
	    ptr.ptr()->fun->recv_real(ptr, res, 0);
      }
}

void vvp_wire_real::release_pv(vvp_net_ptr_t, unsigned, unsigned, bool)
{
      assert(0);
#if 0
	// A real is a single value. If for some reason this part release
	// can happen the following code should work correctly (requires
	// a base of 0 and a width of 1).
      vvp_vector2_t mask (vvp_vector2_t::FILL1, 1);
      assert(base == 0 && wid == 1);

      if (net_flag) {
	      // Wires revert to their unforced value after release.
	    release_mask(mask);
	    ptr.ptr()->send_real(bit_, 0);
      } else {
	      // Variables keep the current value.
	    double res =  real_value();
	    release_mask(mask);
	    ptr.ptr()->fun->recv_real(ptr, res, 0);
      }
#endif
}

unsigned vvp_wire_real::value_size() const
{
      assert(0);
      return 1;
}

vvp_bit4_t vvp_wire_real::value(unsigned) const
{
      assert(0);
      return BIT4_X;
}

vvp_scalar_t vvp_wire_real::scalar_value(unsigned) const
{
      assert(0);
      return vvp_scalar_t();
}

void vvp_wire_real::vec4_value(vvp_vector4_t&) const
{
      assert(0);
}

double vvp_wire_real::real_value() const
{
      if (test_force_mask(0))
	    return force_;
      else
	    return bit_;
}

#if 0
vvp_wire_string::vvp_wire_string()
{
}

unsigned vvp_wire_string::filter_size() const
{
      assert(0);
      return 0;
}

void vvp_wire_string::force_fil_vec4(const vvp_vector4_t&, const vvp_vector2_t&)
{
      assert(0);
}
void vvp_wire_string::force_fil_vec8(const vvp_vector8_t&, const vvp_vector2_t&)
{
      assert(0);
}
void vvp_wire_string::force_fil_real(double, const vvp_vector2_t&)
{
      assert(0);
}

void vvp_wire_string::release(vvp_net_ptr_t ptr, bool net_flag)
{
      assert(0);
}

void vvp_wire_string::release_pv(vvp_net_ptr_t, unsigned, unsigned, bool)
{
      assert(0);
}

unsigned vvp_wire_string::value_size() const
{
      assert(0);
      return 1;
}

vvp_bit4_t vvp_wire_string::value(unsigned) const
{
      assert(0);
      return BIT4_X;
}

vvp_scalar_t vvp_wire_string::scalar_value(unsigned) const
{
      assert(0);
      return vvp_scalar_t();
}

void vvp_wire_string::vec4_value(vvp_vector4_t&) const
{
      assert(0);
}

double vvp_wire_string::real_value() const
{
      assert(0);
      return 0.0;
}
#endif
