/*
 * Copyright (c) 2004-2005 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vvp_net.cc,v 1.43 2005/08/27 03:28:16 steve Exp $"

# include  "config.h"
# include  "vvp_net.h"
# include  "schedule.h"
# include  <stdio.h>
# include  <iostream>
# include  <typeinfo>
# include  <limits.h>
# include  <assert.h>

/* *** BIT operations *** */
vvp_bit4_t add_with_carry(vvp_bit4_t a, vvp_bit4_t b, vvp_bit4_t&c)
{
      if (bit4_is_xz(a) || bit4_is_xz(b) || bit4_is_xz(c)) {
	    c = BIT4_X;
	    return BIT4_X;
      }

	// NOTE: This relies on the facts that XZ values have been
	// weeded out, and that BIT4_1 is 1 and BIT4_0 is 0.
      int sum = (int)a + (int)b + (int)c;

      switch (sum) {
	  case 0:
	    return BIT4_0;
	  case 1:
	    c = BIT4_0;
	    return BIT4_1;
	  case 2:
	    c = BIT4_1;
	    return BIT4_0;
	  case 3:
	    c = BIT4_1;
	    return BIT4_1;
	  default:
	    assert(0);
      }
}

vvp_bit4_t operator & (vvp_bit4_t a, vvp_bit4_t b)
{
      if (a == BIT4_0)
	    return BIT4_0;
      if (b == BIT4_0)
	    return BIT4_0;
      if (bit4_is_xz(a))
	    return BIT4_X;
      if (bit4_is_xz(b))
	    return BIT4_X;
      return BIT4_1;
}

vvp_bit4_t operator | (vvp_bit4_t a, vvp_bit4_t b)
{
      if (a == BIT4_1)
	    return BIT4_1;
      if (b == BIT4_1)
	    return BIT4_1;
      if (bit4_is_xz(a))
	    return BIT4_X;
      if (bit4_is_xz(b))
	    return BIT4_X;
      return BIT4_0;
}

vvp_bit4_t operator ^ (vvp_bit4_t a, vvp_bit4_t b)
{
      if (bit4_is_xz(a))
	    return BIT4_X;
      if (bit4_is_xz(b))
	    return BIT4_X;
      if (a == BIT4_0)
	    return b;
      if (b == BIT4_0)
	    return a;
      return BIT4_0;
}

vvp_bit4_t operator ~ (vvp_bit4_t a)
{
      switch (a) {
	  case BIT4_0:
	    return BIT4_1;
	  case BIT4_1:
	    return BIT4_0;
	  default:
	    return  BIT4_X;
      }
}

ostream& operator<<(ostream&out, vvp_bit4_t bit)
{
      switch (bit) {
	  case BIT4_0:
	    out << "0";
	    break;
	  case BIT4_1:
	    out << "1";
	    break;
	  case BIT4_X:
	    out << "X";
	    break;
	  case BIT4_Z:
	    out << "Z";
	    break;
	  default:
	    out << "?";
	    break;
      }
      return out;
}

void vvp_send_vec8(vvp_net_ptr_t ptr, vvp_vector8_t val)
{
      while (struct vvp_net_t*cur = ptr.ptr()) {
	    vvp_net_ptr_t next = cur->port[ptr.port()];

	    if (cur->fun)
		  cur->fun->recv_vec8(ptr, val);

	    ptr = next;
      }
}

void vvp_send_real(vvp_net_ptr_t ptr, double val)
{
      while (struct vvp_net_t*cur = ptr.ptr()) {
	    vvp_net_ptr_t next = cur->port[ptr.port()];

	    if (cur->fun)
		  cur->fun->recv_real(ptr, val);

	    ptr = next;
      }
}

void vvp_send_long(vvp_net_ptr_t ptr, long val)
{
      while (struct vvp_net_t*cur = ptr.ptr()) {
	    vvp_net_ptr_t next = cur->port[ptr.port()];

	    if (cur->fun)
		  cur->fun->recv_long(ptr, val);

	    ptr = next;
      }
}

void vvp_vector4_t::copy_from_(const vvp_vector4_t&that)
{
      size_ = that.size_;
      if (size_ > BITS_PER_WORD) {
	    unsigned words = (size_+BITS_PER_WORD-1) / BITS_PER_WORD;
	    bits_ptr_ = new unsigned long[words];

	    for (unsigned idx = 0 ;  idx < words ;  idx += 1)
		  bits_ptr_[idx] = that.bits_ptr_[idx];

      } else {
	    bits_val_ = that.bits_val_;
      }
}

void vvp_vector4_t::allocate_words_(unsigned wid, unsigned long init)
{
      if (size_ > BITS_PER_WORD) {
	    unsigned cnt = (size_ + BITS_PER_WORD - 1) / BITS_PER_WORD;
	    bits_ptr_ = new unsigned long[cnt];
	    for (unsigned idx = 0 ;  idx < cnt ;  idx += 1)
		  bits_ptr_[idx] = init;

      } else {
	    bits_val_ = init;
      }
}

vvp_vector4_t::vvp_vector4_t(const vvp_vector4_t&that,
			    unsigned adr, unsigned wid)
{
      size_ = wid;
      assert((adr + wid) <= that.size_);

      allocate_words_(wid, WORD_X_BITS);

      if (wid > BITS_PER_WORD) {
	      /* In this case, the subvector and the source vector are
		 long. Do the transfer reasonably efficiently. */
	    unsigned ptr = adr / BITS_PER_WORD;
	    unsigned off = adr % BITS_PER_WORD;
	    unsigned noff = BITS_PER_WORD - off;
	    unsigned long lmask = (1 << 2*off) - 1;
	    unsigned trans = 0;
	    unsigned dst = 0;
	    while (trans < wid) {
		    // The low bits of the result.
		  bits_ptr_[dst] = (that.bits_ptr_[ptr] & ~lmask) >> 2*off;
		  trans += noff;

		  if (trans >= wid)
			break;

		  ptr += 1;
		  bits_ptr_[dst] |= (that.bits_ptr_[ptr] & lmask) << 2*noff;
		  trans += off;
		  dst += 1;
	    }

      } else {
	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		  set_bit(idx, that.value(adr+idx));
	    }
      }
}

/*
 * Change the size of the vvp_vector4_t vector to the new size. Copy
 * the old values, as many as well fit, into the new vector.
 */
void vvp_vector4_t::resize(unsigned newsize)
{
      if (size_ == newsize)
	    return;

      unsigned cnt = (size_ + BITS_PER_WORD - 1) / BITS_PER_WORD;

      if (newsize > BITS_PER_WORD) {
	    unsigned newcnt = (newsize + BITS_PER_WORD - 1) / BITS_PER_WORD;
	    unsigned long*newbits = new unsigned long[newcnt];

	    if (cnt > 1) {
		  unsigned trans = cnt;
		  if (trans > newcnt)
			trans = newcnt;

		  for (unsigned idx = 0 ;  idx < trans ;  idx += 1)
			newbits[idx] = bits_ptr_[idx];

		  delete[]bits_ptr_;

	    } else {
		  newbits[0] = bits_val_;
	    }

	    for (unsigned idx = cnt ;  idx < newcnt ;  idx += 1)
		  newbits[idx] = WORD_X_BITS;

	    size_ = newsize;
	    bits_ptr_ = newbits;

      } else {
	    unsigned long newval;
	    if (cnt > 1) {
		  newval = bits_ptr_[0];
		  delete[]bits_ptr_;
		  bits_val_ = newval;
	    }
      }
}


unsigned long* vvp_vector4_t::subarray(unsigned adr, unsigned wid) const
{
      const unsigned BIT2_PER_WORD = 8*sizeof(unsigned long);
      unsigned awid = (wid + BIT2_PER_WORD - 1) / (BIT2_PER_WORD);
      unsigned long*val = new unsigned long[awid];

      for (unsigned idx = 0 ;  idx < awid ;  idx += 1)
	    val[idx] = 0;

      if (size_ <= BITS_PER_WORD) {
	      /* Handle the special case that the array is small. The
		 entire value of the vector4 is within the bits_val_
		 so we know that the result is a single word, the
		 source is a single word, and we just have to loop
		 through that word. */
	    unsigned long tmp = bits_val_ >> 2UL*adr;
	    tmp &= (1UL << 2*wid) - 1;
	    if (tmp & WORD_X_BITS)
		  goto x_out;

	    unsigned long mask1 = 1;
	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		  if (tmp & 1) val[0] |= mask1;
		  mask1 <<= 1UL;
		  tmp >>= 2UL;
	    }
	    return val;

      } else {

	      /* Get the first word we are scanning. We may in fact be
		 somewhere in the middle of that word. */
	    unsigned long tmp = bits_ptr_[adr/BITS_PER_WORD];
	    tmp >>= 2UL * (adr%BITS_PER_WORD);

	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
		    /* Starting a new word? */
		  if (adr%BITS_PER_WORD == 0)
			tmp = bits_ptr_[adr/BITS_PER_WORD];

		  if (tmp&2)
			goto x_out;
		  if (tmp&1)
			val[idx/BIT2_PER_WORD] |= 1UL << (idx % BIT2_PER_WORD);

		  adr += 1;
		  tmp >>= 2UL;
	    }
      }

      return val;

 x_out:
      delete[]val;
      return 0;
}

void vvp_vector4_t::set_vec(unsigned adr, const vvp_vector4_t&that)
{
      assert(adr+that.size_  <= size_);

      if (size_ <= BITS_PER_WORD) {

	      /* The destination vector (me!) is within a bits_val_
		 word, so the subvector is certainly within a
		 bits_val_ word. Therefore, the entire operation is a
		 matter of writing the bits of that into the addressed
		 bits of this. The mask below is calculated to be 1
		 for all the bits that are to come from that. Do the
		 job by some shifting, masking and OR. */

	    unsigned long lmask = (1UL << 2UL*adr) - 1;
	    unsigned long hmask = (1 << 2UL*(adr+that.size_)) - 1;
	    unsigned long mask = hmask & ~lmask;

	    bits_val_ =
		  (bits_val_ & ~mask)
		  | ((that.bits_val_<<2UL*adr) & mask);

      } else if (that.size_ <= BITS_PER_WORD) {

	    unsigned long dptr = adr / BITS_PER_WORD;
	    unsigned long doff = adr % BITS_PER_WORD;

	    unsigned long lmask = (1UL<<2UL*doff) - 1;
	    unsigned long hmask = (1UL << 2UL*(doff+that.size_)) - 1;
	    unsigned long mask = hmask & ~lmask;

	    bits_ptr_[dptr] =
		  (bits_ptr_[dptr] & ~mask)
		  | ((that.bits_val_ << 2UL*doff) & mask);

	    if (doff + that.size_ > BITS_PER_WORD) {
		  unsigned tail = doff + that.size_ - BITS_PER_WORD;
		  mask = (1UL << 2UL*tail) - 1;

		  dptr += 1;
		  bits_ptr_[dptr] =
			(bits_ptr_[dptr] & ~mask)
			| ((that.bits_val_ >> 2UL*(that.size_-tail)) & mask);
	    }

      } else if (adr%BITS_PER_WORD == 0) {

	      /* In this case, both vectors are long, but the
		 destination is neatly aligned. That means all but the
		 last word can be simply copied with no masking. */

	    unsigned remain = that.size_;
	    unsigned sptr = 0;
	    unsigned dptr = adr / BITS_PER_WORD;
	    while (remain >= BITS_PER_WORD) {
		  bits_ptr_[dptr++] = that.bits_ptr_[sptr++];
		  remain -= BITS_PER_WORD;
	    }

	    if (remain > 0) {
		  unsigned long mask = (1UL << 2UL*remain) - 1;
		  bits_ptr_[dptr] =
			(bits_ptr_[dptr] & ~mask)
			| (that.bits_ptr_[sptr] & mask);
	    }

      } else {

	      /* We know that there are two long vectors, and we know
		 that the destination is definitely NOT aligned. */
	    unsigned remain = that.size_;
	    unsigned sptr = 0;
	    unsigned dptr = adr / BITS_PER_WORD;
	    unsigned doff = adr % BITS_PER_WORD;
	    unsigned long lmask = (1UL << 2UL*doff) - 1;
	    unsigned ndoff = BITS_PER_WORD - doff;
	    while (remain >= BITS_PER_WORD) {
		  bits_ptr_[dptr] =
			(bits_ptr_[dptr] & lmask)
			| ((that.bits_ptr_[sptr] << 2UL*doff) & ~lmask);
		  dptr += 1;

		  bits_ptr_[dptr] =
			(bits_ptr_[dptr] & ~lmask)
			| ((that.bits_ptr_[sptr] >> 2UL*ndoff) & lmask);

		  remain -= BITS_PER_WORD;
		  sptr += 1;
	    }

	    unsigned long hmask = (1UL << 2UL*(doff+remain)) - 1;
	    unsigned long mask = hmask & ~lmask;

	    bits_ptr_[dptr] =
		  (bits_ptr_[dptr] & ~mask)
		  | ((that.bits_ptr_[sptr] << 2UL*doff) & mask);

	    if (doff + remain > BITS_PER_WORD) {
		  unsigned tail = doff + remain - BITS_PER_WORD;
		  mask = (1UL << 2UL*tail) - 1;

		  dptr += 1;
		  bits_ptr_[dptr] =
			(bits_ptr_[dptr] & ~mask)
			| ((that.bits_ptr_[sptr] >> 2UL*(remain-tail))&mask);
	    }


      }
}

bool vvp_vector4_t::eeq(const vvp_vector4_t&that) const
{
      if (size_ != that.size_)
	    return false;

      if (size_ <= BITS_PER_WORD) {
	    unsigned long mask = (1UL << 2UL * size_) - 1;
	    return (bits_val_&mask) == (that.bits_val_&mask);
      }

      unsigned words = size_ / BITS_PER_WORD;
      for (unsigned idx = 0 ;  idx < words ;  idx += 1) {
	    if (bits_ptr_[idx] != that.bits_ptr_[idx])
		  return false;
      }

      unsigned long mask = size_%BITS_PER_WORD;
      if (mask > 0) {
	    mask = (1UL << 2UL*mask) - 1;
	    return (bits_ptr_[words]&mask) == (that.bits_ptr_[words]&mask);
      }

      return true;
}


void vvp_vector4_t::change_z2x()
{
      assert(BIT4_Z == 3 && BIT4_X == 2);
# define Z2X(val) do{ (val) = (val) & ~(((val)&WORD_X_BITS) >> 1UL); }while(0)

      if (size_ <= BITS_PER_WORD) {
	    Z2X(bits_val_);
      } else {
	    unsigned words = (size_+BITS_PER_WORD-1) / BITS_PER_WORD;
	    for (unsigned idx = 0 ;  idx < words ;  idx += 1)
		  Z2X(bits_ptr_[idx]);
      }
}

char* vvp_vector4_t::as_string(char*buf, size_t buf_len)
{
      char*res = buf;
      *buf++ = 'C';
      *buf++ = '4';
      *buf++ = '<';
      buf_len -= 3;

      for (unsigned idx = 0 ;  idx < size() && buf_len >= 2 ;  idx += 1) {
	    switch (value(size()-idx-1)) {
		case BIT4_0:
		  *buf++ = '0';
		  break;
		case BIT4_1:
		  *buf++ = '1';
		  break;
		case BIT4_X:
		  *buf++ = 'x';
		  break;
		case BIT4_Z:
		  *buf++ = 'z';
	    }
	    buf_len -= 1;
      }

      *buf++ = '>';
      *buf++ = 0;
      return res;
}

ostream& operator<< (ostream&out, const vvp_vector4_t&that)
{
      out << that.size() << "'b";
      for (unsigned idx = 0 ;  idx < that.size() ;  idx += 1)
	    out << that.value(that.size()-idx-1);
      return out;
}

bool vector4_to_value(const vvp_vector4_t&vec, unsigned long&val)
{
      unsigned long res = 0;
      unsigned long msk = 1;

      for (unsigned idx = 0 ;  idx < vec.size() ;  idx += 1) {
	    switch (vec.value(idx)) {
		case BIT4_0:
		  break;
		case BIT4_1:
		  res |= msk;
		  break;
		default:
		  return false;
	    }

	    msk <<= 1UL;
      }

      val = res;
      return true;
}

vvp_vector4_t coerce_to_width(const vvp_vector4_t&that, unsigned width)
{
      if (that.size() == width)
	    return that;

      assert(that.size() > width);
      vvp_vector4_t res (width);
      for (unsigned idx = 0 ;  idx < width ;  idx += 1)
	    res.set_bit(idx, that.value(idx));

      return res;
}


vvp_vector2_t::vvp_vector2_t()
{
      vec_ = 0;
      wid_ = 0;
}

vvp_vector2_t::vvp_vector2_t(unsigned long v, unsigned wid)
{
      wid_ = wid;
      const unsigned bits_per_word = 8 * sizeof(vec_[0]);
      const unsigned words = (wid_ + bits_per_word-1) / bits_per_word;

      vec_ = new unsigned long[words];
      for (unsigned idx = 0 ;  idx < words ;  idx += 1)
	    vec_[idx] = 0;
}

vvp_vector2_t::vvp_vector2_t(const vvp_vector4_t&that)
{
      wid_ = that.size();
      const unsigned bits_per_word = 8 * sizeof(vec_[0]);
      const unsigned words = (that.size() + bits_per_word-1) / bits_per_word;

      if (words == 0) {
	    vec_ = 0;
	    wid_ = 0;
	    return;
      }

      vec_ = new unsigned long[words];
      for (unsigned idx = 0 ;  idx < words ;  idx += 1)
	    vec_[idx] = 0;

      for (unsigned idx = 0 ;  idx < that.size() ;  idx += 1) {
	    unsigned addr = idx / bits_per_word;
	    unsigned shift = idx % bits_per_word;

	    switch (that.value(idx)) {
		case BIT4_0:
		  break;
		case BIT4_1:
		  vec_[addr] |= 1UL << shift;
		  break;
		default:
		  delete[]vec_;
		  vec_ = 0;
		  wid_ = 0;
		  return;
	    }
      }
}

vvp_vector2_t::~vvp_vector2_t()
{
      if (vec_) delete[]vec_;
}

unsigned vvp_vector2_t::size() const
{
      return wid_;
}

int vvp_vector2_t::value(unsigned idx) const
{
      if (idx >= wid_)
	    return 0;

      const unsigned bits_per_word = 8 * sizeof(vec_[0]);
      unsigned addr = idx/bits_per_word;
      unsigned mask = idx%bits_per_word;

      if (vec_[addr] & (1UL<<mask))
	    return 1;
      else
	    return 0;
}

bool vvp_vector2_t::is_NaN() const
{
      return wid_ == 0;
}

static unsigned long add_carry(unsigned long a, unsigned long b,
			       unsigned long&carry)
{
      unsigned long out = carry;
      carry = 0;

      if ((ULONG_MAX - out) < a)
	    carry += 1;
      out += a;

      if ((ULONG_MAX - out) < b)
	    carry += 1;
      out += b;

      return out;
}

static void multiply_long(unsigned long a, unsigned long b,
			  unsigned long&low, unsigned long&high)
{
      assert(sizeof(unsigned long) %2 == 0);

      const unsigned long word_mask = (1UL << 4UL*sizeof(a)) - 1UL;
      unsigned long tmpa;
      unsigned long tmpb;
      unsigned long res[4];

      tmpa = a & word_mask;
      tmpb = b & word_mask;
      res[0] = tmpa * tmpb;
      res[1] = res[0] >> 4UL*sizeof(unsigned long);
      res[0] &= word_mask;

      tmpa = (a >> 4UL*sizeof(unsigned long)) & word_mask;
      tmpb = b & word_mask;
      res[1] += tmpa * tmpb;
      res[2] = res[1] >> 4UL*sizeof(unsigned long);
      res[1] &= word_mask;

      tmpa = a & word_mask;
      tmpb = (b >> 4UL*sizeof(unsigned long)) & word_mask;
      res[1] += tmpa * tmpb;
      res[2] += res[1] >> 4UL*sizeof(unsigned long);
      res[3]  = res[2] >> 4UL*sizeof(unsigned long);
      res[1] &= word_mask;
      res[2] &= word_mask;

      tmpa = (a >> 4UL*sizeof(unsigned long)) & word_mask;
      tmpb = (b >> 4UL*sizeof(unsigned long)) & word_mask;
      res[2] += tmpa * tmpb;
      res[3] += res[2] >> 4UL*sizeof(unsigned long);
      res[2] &= word_mask;

      high = (res[3] << 4UL*sizeof(unsigned long)) | res[2];
      low  = (res[1] << 4UL*sizeof(unsigned long)) | res[0];
}

/*
 * Multiplication of two vector2 vectors returns a product as wide as
 * the sum of the widths of the input vectors.
 */
vvp_vector2_t operator * (const vvp_vector2_t&a, const vvp_vector2_t&b)
{
      const unsigned bits_per_word = 8 * sizeof(a.vec_[0]);
      vvp_vector2_t r (0, a.size() + b.size());

      unsigned awords = (a.wid_ + bits_per_word - 1) / bits_per_word;
      unsigned bwords = (b.wid_ + bits_per_word - 1) / bits_per_word;
      unsigned rwords = (r.wid_ + bits_per_word - 1) / bits_per_word;

      for (unsigned bdx = 0 ;  bdx < bwords ;  bdx += 1) {
	    unsigned long tmpb = b.vec_[bdx];
	    if (tmpb == 0)
		  continue;

	    for (unsigned adx = 0 ;  adx < awords ;  adx += 1) {
		  unsigned long tmpa = a.vec_[adx];
		  if (tmpa == 0)
			continue;

		  unsigned long low, hig;
		  multiply_long(tmpa, tmpb, low, hig);

		  unsigned long carry = 0;
		  for (unsigned sdx = 0
			     ; (adx+bdx+sdx) < rwords
			     ;  sdx += 1) {

			r.vec_[adx+bdx+sdx] = add_carry(r.vec_[adx+bdx+sdx],
							low, carry);
			low = hig;
			hig = 0;
		  }
	    }
      }


      return r;
}

vvp_vector4_t vector2_to_vector4(const vvp_vector2_t&that, unsigned wid)
{
      vvp_vector4_t res (wid);

      for (unsigned idx = 0 ;  idx < res.size() ;  idx += 1) {
	    vvp_bit4_t bit = BIT4_0;

	    if (that.value(idx))
		  bit = BIT4_1;

	    res.set_bit(idx, bit);
      }

      return res;
}

vvp_vector8_t::vvp_vector8_t(const vvp_vector8_t&that)
{
      size_ = that.size_;

      bits_ = new vvp_scalar_t[size_];

      for (unsigned idx = 0 ;  idx < size_ ;  idx += 1)
	    bits_[idx] = that.bits_[idx];

}

vvp_vector8_t::vvp_vector8_t(unsigned size)
: size_(size)
{
      if (size_ == 0) {
	    bits_ = 0;
	    return;
      }

      bits_ = new vvp_scalar_t[size_];
}

vvp_vector8_t::vvp_vector8_t(const vvp_vector4_t&that, unsigned str)
: size_(that.size())
{
      if (size_ == 0) {
	    bits_ = 0;
	    return;
      }

      bits_ = new vvp_scalar_t[size_];

      for (unsigned idx = 0 ;  idx < size_ ;  idx += 1)
	    bits_[idx] = vvp_scalar_t (that.value(idx), str);

}

vvp_vector8_t::vvp_vector8_t(const vvp_vector4_t&that,
			     unsigned str0, unsigned str1)
: size_(that.size())
{
      if (size_ == 0) {
	    bits_ = 0;
	    return;
      }

      bits_ = new vvp_scalar_t[size_];
      for (unsigned idx = 0 ;  idx < size_ ;  idx += 1)
	    bits_[idx] = vvp_scalar_t (that.value(idx), str0, str1);

}

vvp_vector8_t& vvp_vector8_t::operator= (const vvp_vector8_t&that)
{
      if (size_ != that.size_) {
	    if (size_ > 0)
		  delete[]bits_;
	    size_ = 0;
      }

      if (that.size_ == 0) {
	    assert(size_ == 0);
	    return *this;
      }

      if (size_ == 0) {
	    size_ = that.size_;
	    bits_ = new vvp_scalar_t[size_];
      }

      for (unsigned idx = 0 ;  idx < size_ ;  idx += 1)
	    bits_[idx] = that.bits_[idx];

      return *this;
}

bool vvp_vector8_t::eeq(const vvp_vector8_t&that) const
{
      if (size_ != that.size_)
	    return false;

      if (size_ == 0)
	    return true;

      for (unsigned idx = 0 ;  idx < size_ ;  idx += 1) {
	    if (! bits_[idx] .eeq( that.bits_[idx] ))
		return false;
      }

      return true;
}

ostream& operator<<(ostream&out, const vvp_vector8_t&that)
{
      out << "C8<";
      for (unsigned idx = 0 ;  idx < that.size() ; idx += 1)
	    out << that.value(that.size()-idx-1);

      out << ">";
      return out;
}

vvp_net_fun_t::vvp_net_fun_t()
{
}

vvp_net_fun_t::~vvp_net_fun_t()
{
}

void vvp_net_fun_t::recv_vec4(vvp_net_ptr_t, const vvp_vector4_t&)
{
      fprintf(stderr, "internal error: %s: recv_vec4 not implemented\n",
	      typeid(*this).name());
      assert(0);
}

void vvp_net_fun_t::recv_vec4_pv(vvp_net_ptr_t, const vvp_vector4_t&,
				 unsigned, unsigned, unsigned)
{
      fprintf(stderr, "internal error: %s: recv_vec4_pv not implemented\n",
	      typeid(*this).name());
      assert(0);
}

void vvp_net_fun_t::recv_vec8(vvp_net_ptr_t port, vvp_vector8_t bit)
{
      recv_vec4(port, reduce4(bit));
}

void vvp_net_fun_t::recv_real(vvp_net_ptr_t, double)
{
      fprintf(stderr, "internal error: %s: recv_real not implemented\n",
	      typeid(*this).name());
      assert(0);
}

void vvp_net_fun_t::recv_long(vvp_net_ptr_t, long)
{
      fprintf(stderr, "internal error: %s: recv_long not implemented\n",
	      typeid(*this).name());
      assert(0);
}

/* **** vvp_fun_drive methods **** */

vvp_fun_drive::vvp_fun_drive(vvp_bit4_t init, unsigned str0, unsigned str1)
{
      assert(str0 < 8);
      assert(str1 < 8);

      drive0_ = str0;
      drive1_ = str1;
}

vvp_fun_drive::~vvp_fun_drive()
{
}

void vvp_fun_drive::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
      assert(port.port() == 0);
      vvp_send_vec8(port.ptr()->out, vvp_vector8_t(bit, drive0_, drive1_));
}


/* **** vvp_fun_signal methods **** */

vvp_fun_signal_base::vvp_fun_signal_base()
{
      needs_init_ = true;
      continuous_assign_active_ = false;
      force_active_ = false;
      force_link = 0;
}

void vvp_fun_signal_base::deassign()
{
      continuous_assign_active_ = false;
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
		  assert(0);
		  break;
	    }
	    break;

	  default: // Other ports ar errors.
	    assert(0);
	    break;
      }
}

vvp_fun_signal::vvp_fun_signal(unsigned wid)
: bits4_(wid)
{
}

/*
 * Nets simply reflect their input to their output.
 *
 * NOTE: It is a quirk of vvp_fun_signal that it has an initial value
 * that needs to be propagated, but after that it only needs to
 * propagate if the value changes. Elimitating duplicate propagations
 * should improve performance, but has the quirk that an input that
 * matches the initial value might not be propagated. The hack used
 * herein is to keep a "needs_init_" flag that is turned false after
 * the first propagation, and forces the first propagation to happen
 * even if it matches the initial value.
 */
void vvp_fun_signal::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit)
{
      switch (ptr.port()) {
	  case 0: // Normal input (feed from net, or set from process)
	    if (!continuous_assign_active_) {
		  if (needs_init_ || !bits4_.eeq(bit)) {
			bits4_ = bit;
			needs_init_ = false;
			vvp_send_vec4(ptr.ptr()->out, bit);
			run_vpi_callbacks();
		  }
	    }
	    break;

	  case 1: // Continuous assign value
	    continuous_assign_active_ = true;
	    if (type_is_vector8_()) {
		  bits8_ = vvp_vector8_t(bit,6);
		  vvp_send_vec8(ptr.ptr()->out, bits8_);
	    } else {
		  bits4_ = bit;
		  vvp_send_vec4(ptr.ptr()->out, bits4_);
	    }
	    run_vpi_callbacks();
	    break;

	  case 2: // Force value

	      // Force from a node may not have been sized completely
	      // by the source, so coerce the size here.
	    if (bit.size() != size())
		  force_ = coerce_to_width(bit, size());
	    else
		  force_ = bit;

	    force_active_ = true;
	    vvp_send_vec4(ptr.ptr()->out, force_);
	    run_vpi_callbacks();
	    break;

	  default:
	    assert(0);
	    break;
      }
}

void vvp_fun_signal::recv_vec4_pv(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
				  unsigned base, unsigned wid, unsigned vwid)
{
      assert(bit.size() == wid);
      assert(bits4_.size() == vwid);

      switch (ptr.port()) {
	  case 0: // Normal input
	    if (! continuous_assign_active_) {
		  for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
			if (base+idx >= bits4_.size())
			      break;
			bits4_.set_bit(base+idx, bit.value(idx));
		  }
		  needs_init_ = false;
		  vvp_send_vec4(ptr.ptr()->out, bits4_);
		  run_vpi_callbacks();
	    }
	    break;

	  default:
	    assert(0);
	    break;
      }
}

void vvp_fun_signal::recv_vec8(vvp_net_ptr_t ptr, vvp_vector8_t bit)
{
	// Only port-0 supports vector8_t inputs.
      assert(ptr.port() == 0);

      if (! continuous_assign_active_) {
	    bits8_ = bit;
	    needs_init_ = false;
	    vvp_send_vec8(ptr.ptr()->out, bit);
	    run_vpi_callbacks();
      }
}

void vvp_fun_signal::release(vvp_net_ptr_t ptr, bool net)
{
      force_active_ = false;
      if (net) {
	    vvp_send_vec4(ptr.ptr()->out, bits4_);
	    run_vpi_callbacks();
      } else {
	    bits4_ = force_;
      }
}

unsigned vvp_fun_signal::size() const
{
      if (force_active_)
	    return force_.size();
      else if (type_is_vector8_())
	    return bits8_.size();
      else
	    return bits4_.size();
}

vvp_bit4_t vvp_fun_signal::value(unsigned idx) const
{
      if (force_active_)
	    return force_.value(idx);
      else if (type_is_vector8_())
	    return bits8_.value(idx).value();
      else
	    return bits4_.value(idx);
}

vvp_scalar_t vvp_fun_signal::scalar_value(unsigned idx) const
{
      if (force_active_)
	    return vvp_scalar_t(force_.value(idx), 6, 6);
      else if (type_is_vector8_())
	    return bits8_.value(idx);
      else
	    return vvp_scalar_t(bits4_.value(idx), 6, 6);
}

vvp_vector4_t vvp_fun_signal::vec4_value() const
{
      if (force_active_)
	    return force_;
      else if (type_is_vector8_())
	    return reduce4(bits8_);
      else
	    return bits4_;
}

vvp_fun_signal_real::vvp_fun_signal_real()
{
}

double vvp_fun_signal_real::real_value() const
{
      if (force_active_)
	    return force_;
      else
	    return bits_;
}

void vvp_fun_signal_real::recv_real(vvp_net_ptr_t ptr, double bit)
{
      switch (ptr.port()) {
	  case 0:
	    if (!continuous_assign_active_) {
		  if (needs_init_ || (bits_ != bit)) {
			bits_ = bit;
			needs_init_ = false;
			vvp_send_real(ptr.ptr()->out, bit);
			run_vpi_callbacks();
		  }
	    }
	    break;

	  case 1: // Continuous assign value
	    continuous_assign_active_ = true;
	    bits_ = bit;
	    vvp_send_real(ptr.ptr()->out, bit);
	    run_vpi_callbacks();
	    break;

	  case 2: // Force value
	    force_active_ = true;
	    force_ = bit;
	    vvp_send_real(ptr.ptr()->out, bit);
	    run_vpi_callbacks();
	    break;

	  default:
	    assert(0);
	    break;
      }
}

void vvp_fun_signal_real::release(vvp_net_ptr_t ptr, bool net)
{
      force_active_ = false;
      if (net) {
	    vvp_send_real(ptr.ptr()->out, bits_);
	    run_vpi_callbacks();
      } else {
	    bits_ = force_;
      }
}

/* **** vvp_wide_fun_* methods **** */

vvp_wide_fun_core::vvp_wide_fun_core(vvp_net_t*net, unsigned nports)
{
      ptr_ = net;
      nports_ = nports;
      port_values_ = new vvp_vector4_t [nports_];
}

vvp_wide_fun_core::~vvp_wide_fun_core()
{
      delete[]port_values_;
}

void vvp_wide_fun_core::propagate_vec4(const vvp_vector4_t&bit,
				       vvp_time64_t delay)
{
      if (delay)
	    schedule_assign_vector(ptr_->out, bit, delay);
      else
	    vvp_send_vec4(ptr_->out, bit);
}


unsigned vvp_wide_fun_core::port_count() const
{
      return nports_;
}

vvp_vector4_t& vvp_wide_fun_core::value(unsigned idx)
{
      assert(idx < nports_);
      return port_values_[idx];
}

void vvp_wide_fun_core::dispatch_vec4_from_input_(unsigned port,
						   vvp_vector4_t bit)
{
      assert(port < nports_);
      port_values_[port] = bit;
      recv_vec4_from_inputs(port);
}

vvp_wide_fun_t::vvp_wide_fun_t(vvp_wide_fun_core*c, unsigned base)
: core_(c), port_base_(base)
{
}

vvp_wide_fun_t::~vvp_wide_fun_t()
{
}

void vvp_wide_fun_t::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
      unsigned pidx = port_base_ + port.port();
      core_->dispatch_vec4_from_input_(pidx, bit);
}


/* **** vvp_scalar_t methods **** */

/*
 * DRIVE STRENGTHS:
 *
 * The normal functor is not aware of strengths. It generates strength
 * simply by virtue of having strength specifications. The drive
 * strength specification includes a drive0 and drive1 strength, each
 * with 8 possible values (that can be represented in 3 bits) as given
 * in this table:
 *
 *    HiZ    = 0,
 *    SMALL  = 1,
 *    MEDIUM = 2,
 *    WEAK   = 3,
 *    LARGE  = 4,
 *    PULL   = 5,
 *    STRONG = 6,
 *    SUPPLY = 7
 *
 * The vvp_scalar_t value, however, is a combination of value and
 * strength, used in strength-aware contexts.
 *
 * OUTPUT STRENGTHS:
 *
 * The strength-aware values are specified as an 8 bit value, that is
 * two 4 bit numbers. The value is encoded with two drive strengths (0-7)
 * and two drive values (0 or 1). Each nibble contains three bits of
 * strength and one bit of value, like so: VSSS. The high nibble has
 * the strength-value closest to supply1, and the low nibble has the
 * strength-value closest to supply0.
 */

/*
 * A signal value is unambiguous if the top 4 bits and the bottom 4
 * bits are identical. This means that the VSSSvsss bits of the 8bit
 * value have V==v and SSS==sss.
 */
# define UNAMBIG(v)  (((v) & 0x0f) == (((v) >> 4) & 0x0f))

#if 0
# define STREN1(v) ( ((v)&0x80)? ((v)&0xf0) : (0x70 - ((v)&0xf0)) )
# define STREN0(v) ( ((v)&0x08)? ((v)&0x0f) : (0x07 - ((v)&0x0f)) )
#else
# define STREN1(v) (((v)&0x70) >> 4)
# define STREN0(v) ((v)&0x07)
#endif

vvp_scalar_t::vvp_scalar_t(vvp_bit4_t val, unsigned str0, unsigned str1)
{
      assert(str0 <= 7);
      assert(str1 <= 7);

      if (str0 == 0 && str1 == 0) {
	    value_ = 0x00;
      } else switch (val) {
	  case BIT4_0:
	    value_ = str0 | (str0<<4);
	    break;
	  case BIT4_1:
	    value_ = str1 | (str1<<4) | 0x88;
	    break;
	  case BIT4_X:
	    value_ = str0 | (str1<<4) | 0x80;
	    break;
	  case BIT4_Z:
	    value_ = 0x00;
	    break;
      }
}

vvp_bit4_t vvp_scalar_t::value() const
{
      if (value_ == 0) {
	    return BIT4_Z;

      } else switch (value_ & 0x88) {
	  case 0x00:
	    return BIT4_0;
	  case 0x88:
	    return BIT4_1;
	  default:
	    return BIT4_X;
      }
}

unsigned vvp_scalar_t::strength0() const
{
      return STREN0(value_);
}

unsigned vvp_scalar_t::strength1() const
{
      return STREN1(value_);
}

ostream& operator <<(ostream&out, vvp_scalar_t a)
{
      out << a.strength0() << a.strength1();
      switch (a.value()) {
	  case BIT4_0:
	    out << "0";
	    break;
	  case BIT4_1:
	    out << "1";
	    break;
	  case BIT4_X:
	    out << "X";
	    break;
	  case BIT4_Z:
	    out << "Z";
	    break;
      }
      return out;
}

vvp_scalar_t resolve(vvp_scalar_t a, vvp_scalar_t b)
{
	// If the value is 0, that is the same as HiZ. In that case,
	// resolution is simply a matter of returning the *other* value.
      if (a.value_ == 0)
	    return b;
      if (b.value_ == 0)
	    return a;

      vvp_scalar_t res = a;

      if (UNAMBIG(a.value_) && UNAMBIG(b.value_)) {

	      /* If both signals are unambiguous, simply choose
		 the stronger. If they have the same strength
		 but different values, then this becomes
		 ambiguous. */

	    if (a.value_ == b.value_) {

		    /* values are equal. do nothing. */

	    } else if ((b.value_&0x07) > (res.value_&0x07)) {

		    /* New value is stronger. Take it. */
		  res.value_ = b.value_;

	    } else if ((b.value_&0x77) == (res.value_&0x77)) {

		    /* Strengths are the same. Make value ambiguous. */
		  res.value_ = (res.value_&0x70) | (b.value_&0x07) | 0x80;

	    } else {

		    /* Must be res is the stronger one. */
	    }

      } else if (UNAMBIG(res.value_)) {
	    unsigned tmp = 0;

	    if ((res.value_&0x70) > (b.value_&0x70))
		  tmp |= res.value_&0xf0;
	    else
		  tmp |= b.value_&0xf0;

	    if ((res.value_&0x07) > (b.value_&0x07))
		  tmp |= res.value_&0x0f;
	    else
		  tmp |= b.value_&0x0f;

	    res.value_ = tmp;

      } else if (UNAMBIG(b.value_)) {

	      /* If one of the signals is unambiguous, then it
		 will sweep up the weaker parts of the ambiguous
		 signal. The result may be ambiguous, or maybe not. */

	    unsigned tmp = 0;

	    if ((b.value_&0x70) > (res.value_&0x70))
		  tmp |= b.value_&0xf0;
	    else
		  tmp |= res.value_&0xf0;

	    if ((b.value_&0x07) > (res.value_&0x07))
		  tmp |= b.value_&0x0f;
	    else
		  tmp |= res.value_&0x0f;

	    res.value_ = tmp;

      } else {

	      /* If both signals are ambiguous, then the result
		 has an even wider ambiguity. */

	    unsigned tmp = 0;
	    int sv1a = a.value_&0x80 ? STREN1(a.value_) : - STREN1(a.value_);
	    int sv0a = a.value_&0x08 ? STREN0(a.value_) : - STREN0(a.value_);
	    int sv1b = b.value_&0x80 ? STREN1(b.value_) : - STREN1(b.value_);
	    int sv0b = b.value_&0x08 ? STREN0(b.value_) : - STREN0(b.value_);

	    int sv1 = sv1a;
	    int sv0 = sv0a;

	    if (sv0a > sv1)
		  sv1 = sv0a;
	    if (sv1b > sv1)
		  sv1 = sv1b;
	    if (sv0b > sv1)
		  sv1 = sv0b;

	    if (sv1a < sv0)
		  sv0 = sv1a;
	    if (sv1b < sv0)
		  sv0 = sv1b;
	    if (sv0b < sv0)
		  sv0 = sv0b;

	    if (sv1 > 0) {
		  tmp |= 0x80;
		  tmp |= sv1 << 4;
	    } else {
		  tmp |= (-sv1) << 4;
	    }

	    if (sv0 > 0) {
		  tmp |= 0x08;
		  tmp |= sv0;
	    } else {
		  tmp |= (-sv0);
	    }

	    res.value_ = tmp;
      }

	/* Canonicalize the HiZ value. */
      if ((res.value_&0x77) == 0)
	    res.value_ = 0;

      return res;
}

vvp_vector8_t resolve(const vvp_vector8_t&a, const vvp_vector8_t&b)
{
      assert(a.size() == b.size());

      vvp_vector8_t out (a.size());

      for (unsigned idx = 0 ;  idx < out.size() ;  idx += 1) {
	    out.set_bit(idx, resolve(a.value(idx), b.value(idx)));
      }

      return out;
}

vvp_vector8_t resistive_reduction(const vvp_vector8_t&that)
{
      static unsigned rstr[8] = {
	    0, /* Hi-Z --> Hi-Z */
	    1, /* Small capacitance  --> Small capacitance */
	    1, /* Medium capacitance --> Small capacitance */
	    2, /* Weak drive         --> Medium capacitance */
	    2, /* Large capacitance  --> Medium capacitance */
	    3, /* Pull drive         --> Weak drive */
	    5, /* Strong drive       --> Pull drive */
	    5  /* Supply drive       --> Pull drive */
      };

      vvp_vector8_t res (that.size());

      for (unsigned idx = 0 ;  idx < res.size() ;  idx += 1) {
	    vvp_scalar_t bit = that.value(idx);
	    bit = vvp_scalar_t(bit.value(),
			       rstr[bit.strength0()],
			       rstr[bit.strength1()]);
	    res.set_bit(idx, bit);
      }

      return res;
}

vvp_vector4_t reduce4(const vvp_vector8_t&that)
{
      vvp_vector4_t out (that.size());
      for (unsigned idx = 0 ;  idx < out.size() ;  idx += 1)
	    out.set_bit(idx, that.value(idx).value());

      return out;
}

vvp_bit4_t compare_gtge(const vvp_vector4_t&lef, const vvp_vector4_t&rig,
			vvp_bit4_t out_if_equal)
{
      unsigned min_size = lef.size();
      if (rig.size() < min_size)
	    min_size = rig.size();

	// If one of the inputs is nil, treat is as all X values, and
	// that makes the result BIT4_X.
      if (min_size == 0)
	    return BIT4_X;

	// As per the IEEE1364 definition of >, >=, < and <=, if there
	// are any X or Z values in either of the operand vectors,
	// then the result of the compare is BIT4_X.

	// Check for X/Z in the left operand
      for (unsigned idx = 0 ;  idx < lef.size() ;  idx += 1) {
	    vvp_bit4_t bit = lef.value(idx);
	    if (bit == BIT4_X)
		  return BIT4_X;
	    if (bit == BIT4_Z)
		  return BIT4_X;
      }

	// Check for X/Z in the right operand
      for (unsigned idx = 0 ;  idx < rig.size() ;  idx += 1) {
	    vvp_bit4_t bit = rig.value(idx);
	    if (bit == BIT4_X)
		  return BIT4_X;
	    if (bit == BIT4_Z)
		  return BIT4_X;
      }

      for (unsigned idx = lef.size() ; idx > rig.size() ;  idx -= 1) {
	    if (lef.value(idx-1) == BIT4_1)
		  return BIT4_1;
      }

      for (unsigned idx = rig.size() ; idx > lef.size() ;  idx -= 1) {
	    if (rig.value(idx-1) == BIT4_1)
		  return BIT4_0;
      }

      for (unsigned idx = min_size ; idx > 0 ;  idx -= 1) {
	    vvp_bit4_t lv = lef.value(idx-1);
	    vvp_bit4_t rv = rig.value(idx-1);

	    if (lv == rv)
		  continue;

	    if (lv == BIT4_1)
		  return BIT4_1;
	    else
		  return BIT4_0;
      }

      return out_if_equal;
}

vvp_vector4_t operator ~ (const vvp_vector4_t&that)
{
      vvp_vector4_t res (that.size());
      for (unsigned idx = 0 ;  idx < res.size() ;  idx += 1)
	    res.set_bit(idx, ~ that.value(idx));

      return res;
}

vvp_bit4_t compare_gtge_signed(const vvp_vector4_t&a,
			       const vvp_vector4_t&b,
			       vvp_bit4_t out_if_equal)
{
      assert(a.size() == b.size());

      unsigned sign_idx = a.size()-1;
      vvp_bit4_t a_sign = a.value(sign_idx);
      vvp_bit4_t b_sign = b.value(sign_idx);

      if (a_sign == BIT4_X)
	    return BIT4_X;
      if (a_sign == BIT4_Z)
	    return BIT4_X;
      if (b_sign == BIT4_X)
	    return BIT4_X;
      if (b_sign == BIT4_Z)
	    return BIT4_Z;

      if (a_sign == b_sign)
	    return compare_gtge(a, b, out_if_equal);

      for (unsigned idx = 0 ;  idx < sign_idx ;  idx += 1) {
	    vvp_bit4_t a_bit = a.value(idx);
	    vvp_bit4_t b_bit = a.value(idx);

	    if (a_bit == BIT4_X)
		  return BIT4_X;
	    if (a_bit == BIT4_Z)
		  return BIT4_X;
	    if (b_bit == BIT4_X)
		  return BIT4_X;
	    if (b_bit == BIT4_Z)
		  return BIT4_Z;
      }

      if(a_sign == BIT4_0)
	    return BIT4_1;
      else
	    return BIT4_0;
}

/*
 * $Log: vvp_net.cc,v $
 * Revision 1.43  2005/08/27 03:28:16  steve
 *  Be more cautios about accessing out-of-range bits.
 *
 * Revision 1.42  2005/08/27 02:34:43  steve
 *  Bring threads into the vvp_vector4_t structure.
 *
 * Revision 1.41  2005/07/06 04:29:25  steve
 *  Implement real valued signals and arith nodes.
 *
 * Revision 1.40  2005/06/27 21:13:14  steve
 *  Make vector2 multiply more portable.
 *
 * Revision 1.39  2005/06/26 01:57:22  steve
 *  Make bit masks of vector4_t 64bit aware.
 *
 * Revision 1.38  2005/06/24 02:16:42  steve
 *  inline the vvp_send_vec4_pv function.
 *
 * Revision 1.37  2005/06/22 18:30:12  steve
 *  Inline more simple stuff, and more vector4_t by const reference for performance.
 *
 * Revision 1.36  2005/06/22 00:04:49  steve
 *  Reduce vvp_vector4 copies by using const references.
 *
 * Revision 1.35  2005/06/21 22:48:23  steve
 *  Optimize vvp_scalar_t handling, and fun_buf Z handling.
 *
 * Revision 1.34  2005/06/20 01:28:14  steve
 *  Inline some commonly called vvp_vector4_t methods.
 *
 * Revision 1.33  2005/06/19 18:42:00  steve
 *  Optimize the LOAD_VEC implementation.
 *
 * Revision 1.32  2005/06/15 00:47:15  steve
 *  Resolv do not propogate inputs that do not change.
 *
 * Revision 1.31  2005/06/13 00:54:04  steve
 *  More unified vec4 to hex string functions.
 *
 * Revision 1.30  2005/06/12 15:13:37  steve
 *  Support resistive mos devices.
 *
 * Revision 1.29  2005/06/02 16:02:11  steve
 *  Add support for notif0/1 gates.
 *  Make delay nodes support inertial delay.
 *  Add the %force/link instruction.
 *
 * Revision 1.28  2005/05/17 20:54:56  steve
 *  Clean up definition of vvp_vector4_t insertion into ostream.
 *
 * Revision 1.27  2005/05/07 03:14:50  steve
 *  ostream insert for vvp_vector4_t objects.
 *
 * Revision 1.26  2005/04/28 04:59:53  steve
 *  Remove dead functor code.
 *
 * Revision 1.25  2005/04/25 04:42:17  steve
 *  vvp_fun_signal eliminates duplicate propagations.
 *
 * Revision 1.24  2005/04/13 06:34:20  steve
 *  Add vvp driver functor for logic outputs,
 *  Add ostream output operators for debugging.
 *
 * Revision 1.23  2005/04/09 06:00:58  steve
 *  scalars with 0-drivers are hiZ by definition.
 *
 * Revision 1.22  2005/04/09 05:30:38  steve
 *  Default behavior for recv_vec8 methods.
 *
 * Revision 1.21  2005/04/03 05:45:51  steve
 *  Rework the vvp_delay_t class.
 *
 * Revision 1.20  2005/04/01 06:02:45  steve
 *  Reimplement combinational UDPs.
 *
 * Revision 1.19  2005/03/18 02:56:04  steve
 *  Add support for LPM_UFUNC user defined functions.
 *
 * Revision 1.18  2005/03/12 04:27:43  steve
 *  Implement VPI access to signal strengths,
 *  Fix resolution of ambiguous drive pairs,
 *  Fix spelling of scalar.
 *
 * Revision 1.17  2005/02/14 01:50:23  steve
 *  Signals may receive part vectors from %set/x0
 *  instructions. Re-implement the %set/x0 to do
 *  just that. Remove the useless %set/x0/x instruction.
 *
 * Revision 1.16  2005/02/12 06:13:22  steve
 *  Add debug dumps for vectors, and fix vvp_scaler_t make from BIT4_X values.
 *
 * Revision 1.15  2005/02/10 04:54:41  steve
 *  Simplify vvp_scaler strength representation.
 *
 * Revision 1.14  2005/02/07 22:42:42  steve
 *  Add .repeat functor and BIFIF functors.
 *
 * Revision 1.13  2005/02/04 05:13:02  steve
 *  Add wide .arith/mult, and vvp_vector2_t vectors.
 *
 * Revision 1.12  2005/02/03 04:55:13  steve
 *  Add support for reduction logic gates.
 *
 * Revision 1.11  2005/01/30 05:06:49  steve
 *  Get .arith/sub working.
 *
 * Revision 1.10  2005/01/29 17:52:06  steve
 *  move AND to buitin instead of table.
 *
 * Revision 1.9  2005/01/28 05:34:25  steve
 *  Add vector4 implementation of .arith/mult.
 *
 * Revision 1.8  2005/01/22 17:36:15  steve
 *  .cmp/x supports signed magnitude compare.
 *
 * Revision 1.7  2005/01/22 00:58:22  steve
 *  Implement the %load/x instruction.
 *
 * Revision 1.6  2005/01/16 04:19:08  steve
 *  Reimplement comparators as vvp_vector4_t nodes.
 *
 * Revision 1.5  2005/01/09 20:11:16  steve
 *  Add the .part/pv node and related functionality.
 *
 * Revision 1.4  2005/01/01 02:12:34  steve
 *  vvp_fun_signal propagates vvp_vector8_t vectors when appropriate
 *
 * Revision 1.3  2004/12/31 06:00:06  steve
 *  Implement .resolv functors, and stub signals recv_vec8 method.
 *
 * Revision 1.2  2004/12/15 17:16:08  steve
 *  Add basic force/release capabilities.
 *
 * Revision 1.1  2004/12/11 02:31:30  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 */

