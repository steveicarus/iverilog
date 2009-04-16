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

# include  "config.h"
# include  "vvp_net.h"
# include  "vvp_net_sig.h"
# include  "statistics.h"
# include  "vpi_priv.h"
# include  <assert.h>
#ifdef CHECK_WITH_VALGRIND
# include  <valgrind/memcheck.h>
# include  <map>
#endif

template <class T> T coerce_to_width(const T&that, unsigned width)
{
      if (that.size() == width)
	    return that;

      assert(that.size() > width);
      T res (width);
      for (unsigned idx = 0 ;  idx < width ;  idx += 1)
	    res.set_bit(idx, that.value(idx));

      return res;
}

/* **** vvp_fun_signal methods **** */

vvp_fun_signal_base::vvp_fun_signal_base()
{
      needs_init_ = true;
      continuous_assign_active_ = false;
      force_link = 0;
      cassign_link = 0;
      count_functors_sig += 1;
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

/*
 * The signal functor takes commands as long values to port-3. This
 * method interprets those commands.
 */
void vvp_fun_signal_base::recv_long(vvp_net_ptr_t ptr, long bit)
{
      switch (ptr.port()) {
	  case 3: // Command port
	    switch (bit) {
		case 1: // deassign command
		  deassign();
		  break;
		case 2: // release/net
		  release(ptr, true);
		  break;
		case 3: // release/reg
		  release(ptr, false);
		  break;
		default:
		  fprintf(stderr, "Unsupported command %ld.\n", bit);
		  assert(0);
		  break;
	    }
	    break;

	  default: // Other ports are errors.
	    fprintf(stderr, "Unsupported port type %d.\n", ptr.port());
	    assert(0);
	    break;
      }
}

void vvp_fun_signal_base::recv_long_pv(vvp_net_ptr_t ptr, long bit,
                                       unsigned base, unsigned wid)
{
      switch (ptr.port()) {
	  case 3: // Command port
	    switch (bit) {
		case 1: // deassign command
		  deassign_pv(base, wid);
		  break;
		case 2: // release/net
		  release_pv(ptr, true, base, wid);
		  break;
		case 3: // release/reg
		  release_pv(ptr, false, base, wid);
		  break;
		default:
		  fprintf(stderr, "Unsupported command %ld.\n", bit);
		  assert(0);
		  break;
	    }
	    break;

	  default: // Other ports are errors.
	    fprintf(stderr, "Unsupported port type %d.\n", ptr.port());
	    assert(0);
	    break;
      }
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
			bits4_ = bit;
			needs_init_ = false;
			calculate_output_(ptr);
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
			calculate_output_(ptr);
		  }
	    }
	    break;

	  case 1: // Continuous assign value
	    bits4_ = bit;
	    assign_mask_ = vvp_vector2_t(vvp_vector2_t::FILL1, size());
	    calculate_output_(ptr);
	    break;

	  case 2: // Force value

	      // Force from a node may not have been sized completely
	      // by the source, so coerce the size here.
	    if (bit.size() != size())
		  force_ = coerce_to_width(bit, size());
	    else
		  force_ = bit;

	    force_mask_ = vvp_vector2_t(vvp_vector2_t::FILL1, size());
	    calculate_output_(ptr);
	    break;

	  default:
	    fprintf(stderr, "Unsupported port type %d.\n", ptr.port());
	    assert(0);
	    break;
      }
}

void vvp_fun_signal4_sa::recv_vec8(vvp_net_ptr_t ptr, const vvp_vector8_t&bit)
{
      recv_vec4(ptr, reduce4(bit), 0);
}

void vvp_fun_signal4_sa::recv_vec4_pv(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
				      unsigned base, unsigned wid, unsigned vwid,
                                      vvp_context_t)
{
      assert(bit.size() == wid);
      assert(bits4_.size() == vwid);

      switch (ptr.port()) {
	  case 0: // Normal input
	    if (assign_mask_.size() == 0) {
                  for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
			if (base+idx >= bits4_.size()) break;
			bits4_.set_bit(base+idx, bit.value(idx));
		  }
		  needs_init_ = false;
		  calculate_output_(ptr);
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
			calculate_output_(ptr);
		  }
	    }
	    break;

	  case 1: // Continuous assign value
	    if (assign_mask_.size() == 0)
		  assign_mask_ = vvp_vector2_t(vvp_vector2_t::FILL0, size());
	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		  if (base+idx >= bits4_.size())
			break;
		  bits4_.set_bit(base+idx, bit.value(idx));
		  assign_mask_.set_bit(base+idx, 1);
	    }
	    calculate_output_(ptr);
	    break;

	  case 2: // Force value

	    if (force_mask_.size() == 0)
		  force_mask_ = vvp_vector2_t(vvp_vector2_t::FILL0, size());
	    if (force_.size() == 0)
		  force_ = vvp_vector4_t(vwid, BIT4_Z);

	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		  force_mask_.set_bit(base+idx, 1);
		  force_.set_bit(base+idx, bit.value(idx));
	    }

	    calculate_output_(ptr);
	    break;

	  default:
	    fprintf(stderr, "Unsupported port type %d.\n", ptr.port());
	    assert(0);
	    break;
      }
}

void vvp_fun_signal4_sa::recv_vec8_pv(vvp_net_ptr_t ptr, const vvp_vector8_t&bit,
				      unsigned base, unsigned wid, unsigned vwid)
{
      recv_vec4_pv(ptr, reduce4(bit), base, wid, vwid, 0);
}

void vvp_fun_signal4_sa::calculate_output_(vvp_net_ptr_t ptr)
{
      if (force_mask_.size()) {
	    assert(bits4_.size() == force_mask_.size());
	    assert(bits4_.size() == force_.size());
	    vvp_vector4_t bits (bits4_);
	    for (unsigned idx = 0 ;  idx < bits.size() ;  idx += 1) {
		  if (force_mask_.value(idx))
			bits.set_bit(idx, force_.value(idx));
	    }
	    ptr.ptr()->send_vec4(bits, 0);
      } else {
            ptr.ptr()->send_vec4(bits4_, 0);
      }

      run_vpi_callbacks();
}

void vvp_fun_signal4_sa::release(vvp_net_ptr_t ptr, bool net)
{
      force_mask_ = vvp_vector2_t();
      if (net) {
	    ptr.ptr()->send_vec4(bits4_, 0);
	    run_vpi_callbacks();
      } else {
	    bits4_ = force_;
      }
}

void vvp_fun_signal4_sa::release_pv(vvp_net_ptr_t ptr, bool net,
                                    unsigned base, unsigned wid)
{
      assert(bits4_.size() >= base + wid);

      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    force_mask_.set_bit(base+idx, 0);
	    if (!net) bits4_.set_bit(base+idx, force_.value(base+idx));
      }
      if (force_mask_.is_zero()) force_mask_ = vvp_vector2_t();

      if (net) calculate_output_(ptr);
}

unsigned vvp_fun_signal4_sa::size() const
{
      if (force_mask_.size())
	    return force_.size();
      else
	    return bits4_.size();
}

vvp_bit4_t vvp_fun_signal4_sa::value(unsigned idx) const
{
      if (force_mask_.size() && force_mask_.value(idx)) {
	    return force_.value(idx);
      } else {
            return bits4_.value(idx);
      }
}

vvp_scalar_t vvp_fun_signal4_sa::scalar_value(unsigned idx) const
{
      if (force_mask_.size() && force_mask_.value(idx)) {
	    return vvp_scalar_t(force_.value(idx), 6, 6);
      } else {
            return vvp_scalar_t(bits4_.value(idx), 6, 6);
      }
}

vvp_vector4_t vvp_fun_signal4_sa::vec4_value() const
{
      if (force_mask_.size()) {
	    assert(bits4_.size() == force_mask_.size());
	    assert(bits4_.size() == force_.size());
	    vvp_vector4_t bits (bits4_);
	    for (unsigned idx = 0 ;  idx < bits.size() ;  idx += 1) {
		  if (force_mask_.value(idx))
			bits.set_bit(idx, force_.value(idx));
	    }
	    return bits;
      } else {
            return bits4_;
      }
}

vvp_fun_signal4_aa::vvp_fun_signal4_aa(unsigned wid, vvp_bit4_t init)
{
      context_idx_ = vpip_add_item_to_context(this, vpip_peek_context_scope());
      size_ = wid;
}

void vvp_fun_signal4_aa::alloc_instance(vvp_context_t context)
{
      vvp_set_context_item(context, context_idx_, new vvp_vector4_t(size_));
}

void vvp_fun_signal4_aa::reset_instance(vvp_context_t context)
{
      vvp_vector4_t*bits = static_cast<vvp_vector4_t*>
            (vvp_get_context_item(context, context_idx_));

      bits->set_to_x();
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
				      unsigned base, unsigned wid, unsigned vwid,
                                      vvp_context_t context)
{
      assert(ptr.port() == 0);
      assert(bit.size() == wid);
      assert(size_ == vwid);
      assert(context);

      vvp_vector4_t*bits4 = static_cast<vvp_vector4_t*>
            (vvp_get_context_item(context, context_idx_));

      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
            if (base+idx >= bits4->size()) break;
            bits4->set_bit(base+idx, bit.value(idx));
      }
      ptr.ptr()->send_vec4(*bits4, context);
}

void vvp_fun_signal4_aa::release(vvp_net_ptr_t ptr, bool net)
{
        /* Automatic variables can't be forced. */
      assert(0);
}

void vvp_fun_signal4_aa::release_pv(vvp_net_ptr_t ptr, bool net,
                                    unsigned base, unsigned wid)
{
        /* Automatic variables can't be forced. */
      assert(0);
}

unsigned vvp_fun_signal4_aa::size() const
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

vvp_vector4_t vvp_fun_signal4_aa::vec4_value() const
{
      vvp_vector4_t*bits4 = static_cast<vvp_vector4_t*>
            (vthread_get_rd_context_item(context_idx_));

      return *bits4;
}

vvp_fun_signal8::vvp_fun_signal8(unsigned wid)
: bits8_(wid)
{
}

void vvp_fun_signal8::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                                vvp_context_t)
{
      recv_vec8(ptr, vvp_vector8_t(bit,6,6));
}

void vvp_fun_signal8::recv_vec8(vvp_net_ptr_t ptr, const vvp_vector8_t&bit)
{
      switch (ptr.port()) {
	  case 0: // Normal input (feed from net, or set from process)
	    if (needs_init_ || !bits8_.eeq(bit)) {
		  bits8_ = bit;
		  needs_init_ = false;
		  calculate_output_(ptr);
	    }
	    break;

	  case 1: // Continuous assign value
	    /* This is a procedural continuous assign and it can
	     * only be used on a register and a register is never
	     * strength aware. */
	    assert(0);
	    break;

	  case 2: // Force value

	      // Force from a node may not have been sized completely
	      // by the source, so coerce the size here.
	    if (bit.size() != size())
		  force_ = coerce_to_width(bit, size());
	    else
		  force_ = bit;

	    force_mask_ = vvp_vector2_t(vvp_vector2_t::FILL1, size());
	    calculate_output_(ptr);
	    break;

	  default:
	    fprintf(stderr, "Unsupported port type %d.\n", ptr.port());
	    assert(0);
	    break;
      }
}

void vvp_fun_signal8::recv_vec4_pv(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                                   unsigned base, unsigned wid, unsigned vwid,
                                   vvp_context_t)
{
      recv_vec8_pv(ptr, vvp_vector8_t(bit,6,6), base, wid, vwid);
}

void vvp_fun_signal8::recv_vec8_pv(vvp_net_ptr_t ptr, const vvp_vector8_t&bit,
				   unsigned base, unsigned wid, unsigned vwid)
{
      assert(bit.size() == wid);
      assert(bits8_.size() == vwid);

      switch (ptr.port()) {
	  case 0: // Normal input
	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		  if (base+idx >= bits8_.size()) break;
		  bits8_.set_bit(base+idx, bit.value(idx));
	    }
	    needs_init_ = false;
	    calculate_output_(ptr);
	    break;

	  case 1: // Continuous assign value
	    /* This is a procedural continuous assign and it can
	     * only be used on a register and a register is never
	     * strength aware. */
	    assert(0);
	    break;

	  case 2: // Force value

	    if (force_mask_.size() == 0)
		  force_mask_ = vvp_vector2_t(vvp_vector2_t::FILL0, size());
	    if (force_.size() == 0)
		  force_ = vvp_vector8_t(vvp_vector4_t(vwid, BIT4_Z),6,6);

	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		  force_mask_.set_bit(base+idx, 1);
		  force_.set_bit(base+idx, bit.value(idx));
	    }

	    calculate_output_(ptr);
	    break;

	  default:
	    fprintf(stderr, "Unsupported port type %d.\n", ptr.port());
	    assert(0);
	    break;
      }
}

void vvp_fun_signal8::calculate_output_(vvp_net_ptr_t ptr)
{
      if (force_mask_.size()) {
	    assert(bits8_.size() == force_mask_.size());
	    assert(bits8_.size() == force_.size());
	    vvp_vector8_t bits (bits8_);
	    for (unsigned idx = 0 ;  idx < bits.size() ;  idx += 1) {
		  if (force_mask_.value(idx))
			bits.set_bit(idx, force_.value(idx));
	    }
	    ptr.ptr()->send_vec8(bits);

      } else {
	    ptr.ptr()->send_vec8(bits8_);
      }

      run_vpi_callbacks();
}

void vvp_fun_signal8::release(vvp_net_ptr_t ptr, bool net)
{
      force_mask_ = vvp_vector2_t();
      if (net) {
	    ptr.ptr()->send_vec8(bits8_);
	    run_vpi_callbacks();
      } else {
	    bits8_ = force_;
      }
}

void vvp_fun_signal8::release_pv(vvp_net_ptr_t ptr, bool net,
                                 unsigned base, unsigned wid)
{
      assert(bits8_.size() >= base + wid);

      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    force_mask_.set_bit(base+idx, 0);
	    if (!net) bits8_.set_bit(base+idx, force_.value(base+idx));
      }
      if (force_mask_.is_zero()) force_mask_ = vvp_vector2_t();

      if (net) calculate_output_(ptr);
}

unsigned vvp_fun_signal8::size() const
{
      if (force_mask_.size())
	    return force_.size();
      else
	    return bits8_.size();
}

vvp_bit4_t vvp_fun_signal8::value(unsigned idx) const
{
      if (force_mask_.size() && force_mask_.value(idx))
	    return force_.value(idx).value();
      else
	    return bits8_.value(idx).value();
}

vvp_vector4_t vvp_fun_signal8::vec4_value() const
{
      if (force_mask_.size()) {
	    vvp_vector8_t bits (bits8_);
	    for (unsigned idx = 0 ;  idx < bits.size() ;  idx += 1) {
		  if (force_mask_.value(idx))
			bits.set_bit(idx, force_.value(idx));
	    }
	    return reduce4(bits);
      } else
	    return reduce4(bits8_);
}

vvp_scalar_t vvp_fun_signal8::scalar_value(unsigned idx) const
{
      if (force_mask_.size() && force_mask_.value(idx))
	    return force_.value(idx);
      else
	    return bits8_.value(idx);
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

double vvp_fun_signal_real_sa::real_value() const
{
      if (force_mask_.size())
	    return force_;
      else
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
			run_vpi_callbacks();
		  }
	    }
	    break;

	  case 1: // Continuous assign value
	    continuous_assign_active_ = true;
	    bits_ = bit;
	    ptr.ptr()->send_real(bit, 0);
	    run_vpi_callbacks();
	    break;

	  case 2: // Force value
	    force_mask_ = vvp_vector2_t(1, 1);
	    force_ = bit;
	    ptr.ptr()->send_real(bit, 0);
	    run_vpi_callbacks();
	    break;

	  default:
	    fprintf(stderr, "Unsupported port type %d.\n", ptr.port());
	    assert(0);
	    break;
      }
}

void vvp_fun_signal_real_sa::release(vvp_net_ptr_t ptr, bool net)
{
      force_mask_ = vvp_vector2_t();
      if (net) {
	    ptr.ptr()->send_real(bits_, 0);
	    run_vpi_callbacks();
      } else {
	    bits_ = force_;
      }
}

void vvp_fun_signal_real_sa::release_pv(vvp_net_ptr_t ptr, bool net,
                                        unsigned base, unsigned wid)
{
      fprintf(stderr, "Error: cannot take bit/part select of a real value!\n");
      assert(0);
}

vvp_fun_signal_real_aa::vvp_fun_signal_real_aa()
{
      context_idx_ = vpip_add_item_to_context(this, vpip_peek_context_scope());
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

double vvp_fun_signal_real_aa::real_value() const
{
      double*bits = static_cast<double*>
            (vthread_get_rd_context_item(context_idx_));

      return *bits;
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

void vvp_fun_signal_real_aa::release(vvp_net_ptr_t ptr, bool net)
{
        /* Automatic variables can't be forced. */
      assert(0);
}

void vvp_fun_signal_real_aa::release_pv(vvp_net_ptr_t ptr, bool net,
                                        unsigned base, unsigned wid)
{
        /* Automatic variables can't be forced. */
      assert(0);
}
