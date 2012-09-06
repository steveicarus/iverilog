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
# include  "vpi_priv.h"
# include  "schedule.h"
# include  "statistics.h"
# include  <cstdio>
# include  <cstring>
# include  <cstdlib>
# include  <iostream>
# include  <typeinfo>
# include  <climits>
# include  <cmath>
# include  <cassert>
#ifdef CHECK_WITH_VALGRIND
# include  <valgrind/memcheck.h>
# include  <map>
# include "sfunc.h"
#endif

// Allocate around 1Megabytes/chunk.
static const size_t VVP_NET_CHUNK = 1024*1024/sizeof(vvp_net_t);
static vvp_net_t*vvp_net_alloc_table = 0;
#ifdef CHECK_WITH_VALGRIND
static vvp_net_t **vvp_net_pool = 0;
static unsigned vvp_net_pool_count = 0;
#endif
static size_t vvp_net_alloc_remaining = 0;
// For statistics, count the vvp_nets allocated and the bytes of alloc
// chunks allocated.
unsigned long count_vvp_nets = 0;
size_t size_vvp_nets = 0;
size_t size_vvp_net_funs = 0;

void* vvp_net_t::operator new (size_t size)
{
      assert(size == sizeof(vvp_net_t));
      if (vvp_net_alloc_remaining == 0) {
	    vvp_net_alloc_table = ::new vvp_net_t[VVP_NET_CHUNK];
	    vvp_net_alloc_remaining = VVP_NET_CHUNK;
	    size_vvp_nets += size*VVP_NET_CHUNK;
#ifdef CHECK_WITH_VALGRIND
	    VALGRIND_MAKE_MEM_NOACCESS(vvp_net_alloc_table, size*VVP_NET_CHUNK);
	    VALGRIND_CREATE_MEMPOOL(vvp_net_alloc_table, 0, 0);
	    vvp_net_pool_count += 1;
	    vvp_net_pool = (vvp_net_t **) realloc(vvp_net_pool,
	                   vvp_net_pool_count*sizeof(vvp_net_t **));
	    vvp_net_pool[vvp_net_pool_count-1] = vvp_net_alloc_table;
#endif
      }

      vvp_net_t*return_this = vvp_net_alloc_table;
#ifdef CHECK_WITH_VALGRIND
      VALGRIND_MEMPOOL_ALLOC(vvp_net_pool[vvp_net_pool_count-1],
                             return_this, size);
      return_this->pool = vvp_net_pool[vvp_net_pool_count-1];
#endif
      vvp_net_alloc_table += 1;
      vvp_net_alloc_remaining -= 1;
      count_vvp_nets += 1;
      return return_this;
}

#ifdef CHECK_WITH_VALGRIND
static map<vvp_net_t*, bool> vvp_net_map;
static map<sfunc_core*, bool> sfunc_map;

void vvp_net_delete(vvp_net_t *item)
{
      vvp_net_map[item] = true;
      if (sfunc_core*tmp = dynamic_cast<sfunc_core*> (item->fun)) {
	    sfunc_map[tmp] = true;
      }
}

void vvp_net_pool_delete()
{
      unsigned long vvp_nets_del = 0;

      map<vvp_net_t*, bool>::iterator iter;
      for (iter = vvp_net_map.begin(); iter != vvp_net_map.end(); iter++) {
	    vvp_nets_del += 1;
	    VALGRIND_MEMPOOL_FREE(iter->first->pool, iter->first);
      }

      map<sfunc_core*, bool>::iterator siter;
      for (siter = sfunc_map.begin(); siter != sfunc_map.end(); siter++) {
	    delete siter->first;
      }

      if (RUNNING_ON_VALGRIND && (vvp_nets_del != count_vvp_nets)) {
	    fflush(NULL);
	    VALGRIND_PRINTF("Error: vvp missed deleting %lu net(s).",
	                    count_vvp_nets - vvp_nets_del);
      }

      for (unsigned idx = 0; idx < vvp_net_pool_count; idx += 1) {
	    VALGRIND_DESTROY_MEMPOOL(vvp_net_pool[idx])
	    ::delete [] vvp_net_pool[idx];
      }
      free(vvp_net_pool);
      vvp_net_pool = 0;
      vvp_net_pool_count = 0;
}
#endif

void vvp_net_t::operator delete(void*)
{
      assert(0);
}

void* vvp_net_fun_t::operator new(size_t size)
{
	// Link in an initial chunk of space for net_fun_t
	// objects. This chunk doesn't need to be the same size as the
	// subsequent chunks, but we do need to make sure it is
	// aligned with pointer alignment. (Hence the union with "align".)
      static union { void*align; char bytes[512*1024]; } initial_chunk;

	// Initialize the pointer to the initial chunk.
      static char*chunk_ptr = initial_chunk.bytes;
      static size_t chunk_remaining = sizeof(initial_chunk);

	// Once the initial chunk fills up, allocate new chunks in
	// fairly large blocks to reduce the system allocator
	// overhead, but not such big chunks that we create our own
	// waste. (Expect the typical waste to be CHUNK_BYTES/2.)
      const size_t CHUNK_BYTES = 256*1024;

      if (size > chunk_remaining) {
	    chunk_ptr = ::new char[CHUNK_BYTES];
	    chunk_remaining = CHUNK_BYTES;
	    size_vvp_net_funs += CHUNK_BYTES;
      }

      assert( (size%sizeof(void*)) == 0 );

      void*res = chunk_ptr;
      chunk_ptr += size;
      chunk_remaining -= size;

      return res;
}

void vvp_net_fun_t::operator delete(void*)
{
      assert(0);
}

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
	      // c must already be 0.
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
      }
      fprintf(stderr, "Incorrect result %d.\n", sum);
      assert(0);
      return BIT4_X;
}

vvp_bit4_t scalar_to_bit4(PLI_INT32 scalar)
{
      switch(scalar) {
	  case vpi0:
	    return BIT4_0;
	  case vpi1:
	    return BIT4_1;
	  case vpiX:
	    return BIT4_X;
	  case vpiZ:
	    return BIT4_Z;
      }
      fprintf(stderr, "Unsupported scalar value %d.\n", (int)scalar);
      assert(0);
      return BIT4_X;
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

typedef unsigned short edge_t;

inline edge_t VVP_EDGE(vvp_bit4_t from, vvp_bit4_t to)
{
      return 1 << ((from << 2) | to);
}

const edge_t vvp_edge_posedge
      = VVP_EDGE(BIT4_0,BIT4_1)
      | VVP_EDGE(BIT4_0,BIT4_X)
      | VVP_EDGE(BIT4_0,BIT4_Z)
      | VVP_EDGE(BIT4_X,BIT4_1)
      | VVP_EDGE(BIT4_Z,BIT4_1)
      ;

const edge_t vvp_edge_negedge
      = VVP_EDGE(BIT4_1,BIT4_0)
      | VVP_EDGE(BIT4_1,BIT4_X)
      | VVP_EDGE(BIT4_1,BIT4_Z)
      | VVP_EDGE(BIT4_X,BIT4_0)
      | VVP_EDGE(BIT4_Z,BIT4_0)
      ;

int edge(vvp_bit4_t from, vvp_bit4_t to)
{
      edge_t mask = VVP_EDGE(from, to);
      if (mask & vvp_edge_posedge)
	    return 1;
      if (mask & vvp_edge_negedge)
	    return -1;
      return 0;
}

void vvp_send_vec8(vvp_net_ptr_t ptr, const vvp_vector8_t&val)
{
      while (struct vvp_net_t*cur = ptr.ptr()) {
	    vvp_net_ptr_t next = cur->port[ptr.port()];

	    if (cur->fun)
		  cur->fun->recv_vec8(ptr, val);

	    ptr = next;
      }
}

void vvp_send_real(vvp_net_ptr_t ptr, double val, vvp_context_t context)
{
      while (struct vvp_net_t*cur = ptr.ptr()) {
	    vvp_net_ptr_t next = cur->port[ptr.port()];

	    if (cur->fun)
		  cur->fun->recv_real(ptr, val, context);

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

void vvp_send_long_pv(vvp_net_ptr_t ptr, long val,
                      unsigned base, unsigned wid)
{
      while (struct vvp_net_t*cur = ptr.ptr()) {
	    vvp_net_ptr_t next = cur->port[ptr.port()];

	    if (cur->fun)
		  cur->fun->recv_long_pv(ptr, val, base, wid);

	    ptr = next;
      }
}

void vvp_vector4_t::copy_bits(const vvp_vector4_t&that)
{

      if (size_ == that.size_) {
	    if (size_ > BITS_PER_WORD) {
		  unsigned words = (size_+BITS_PER_WORD-1) / BITS_PER_WORD;
		  for (unsigned idx = 0 ;  idx < words ;  idx += 1)
			abits_ptr_[idx] = that.abits_ptr_[idx];
		  for (unsigned idx = 0 ;  idx < words ;  idx += 1)
			bbits_ptr_[idx] = that.bbits_ptr_[idx];
	    } else {
		  abits_val_ = that.abits_val_;
		  bbits_val_ = that.bbits_val_;
	    }
	    return;
      }

	/* Now we know that the sizes of this and that are definitely
	   different. We can use that in code below. In any case, we
	   need to copy only the smaller of the sizes. */

	/* If source and destination are both short, then mask/copy
	   the bit values. */
      if (size_ <= BITS_PER_WORD && that.size_ <= BITS_PER_WORD) {
	    unsigned bits_to_copy = (that.size_ < size_) ? that.size_ : size_;
	    unsigned long mask = (1UL << bits_to_copy) - 1UL;
	    abits_val_ &= ~mask;
	    bbits_val_ &= ~mask;
	    abits_val_ |= that.abits_val_&mask;
	    bbits_val_ |= that.bbits_val_&mask;
	    return;
      }

	/* Now we know that either source or destination are long. If
	   the destination is short, then mask/copy from the low word
	   of the long source. */
      if (size_ <= BITS_PER_WORD) {
	    abits_val_ = that.abits_ptr_[0];
	    bbits_val_ = that.bbits_ptr_[0];
	    if (size_ < BITS_PER_WORD) {
		  unsigned long mask = (1UL << size_) - 1UL;
		  abits_val_ &= mask;
		  bbits_val_ &= mask;
	    }
	    return;
      }

	/* Now we know that the destination must be long. If the
	   source is short, then mask/copy from its value. */
      if (that.size_ <= BITS_PER_WORD) {
	    unsigned long mask;
	    if (that.size_ < BITS_PER_WORD) {
		  mask = (1UL << that.size_) - 1UL;
		  abits_ptr_[0] &= ~mask;
		  bbits_ptr_[0] &= ~mask;
	    } else {
		  mask = -1UL;
	    }
	    abits_ptr_[0] |= that.abits_val_&mask;
	    bbits_ptr_[0] |= that.bbits_val_&mask;
	    return;
      }

	/* Finally, we know that source and destination are long. copy
	   words until we get to the last. */
      unsigned bits_to_copy = (that.size_ < size_) ? that.size_ : size_;
      unsigned word = 0;
      while (bits_to_copy >= BITS_PER_WORD) {
	    abits_ptr_[word] = that.abits_ptr_[word];
	    bbits_ptr_[word] = that.bbits_ptr_[word];
	    bits_to_copy -= BITS_PER_WORD;
	    word += 1;
      }
      if (bits_to_copy > 0) {
	    unsigned long mask = (1UL << bits_to_copy) - 1UL;
	    abits_ptr_[word] &= ~mask;
	    bbits_ptr_[word] &= ~mask;
	    abits_ptr_[word] |= that.abits_ptr_[word] & mask;
	    bbits_ptr_[word] |= that.bbits_ptr_[word] & mask;
      }
}

void vvp_vector4_t::copy_from_(const vvp_vector4_t&that)
{
      size_ = that.size_;
      if (size_ > BITS_PER_WORD) {
	    unsigned words = (size_+BITS_PER_WORD-1) / BITS_PER_WORD;
	    abits_ptr_ = new unsigned long[2*words];
	    bbits_ptr_ = abits_ptr_ + words;

	    for (unsigned idx = 0 ;  idx < words ;  idx += 1)
		  abits_ptr_[idx] = that.abits_ptr_[idx];
	    for (unsigned idx = 0 ;  idx < words ;  idx += 1)
		  bbits_ptr_[idx] = that.bbits_ptr_[idx];

      } else {
	    abits_val_ = that.abits_val_;
	    bbits_val_ = that.bbits_val_;
      }
}

void vvp_vector4_t::allocate_words_(unsigned wid, unsigned long inita, unsigned long initb)
{
      if (size_ > BITS_PER_WORD) {
	    unsigned cnt = (size_ + BITS_PER_WORD - 1) / BITS_PER_WORD;
	    abits_ptr_ = new unsigned long[2*cnt];
	    bbits_ptr_ = abits_ptr_ + cnt;
	    for (unsigned idx = 0 ;  idx < cnt ;  idx += 1)
		  abits_ptr_[idx] = inita;
	    for (unsigned idx = 0 ;  idx < cnt ;  idx += 1)
		  bbits_ptr_[idx] = initb;

      } else {
	    abits_val_ = inita;
	    bbits_val_ = initb;
      }
}

vvp_vector4_t::vvp_vector4_t(unsigned size__, double val)
: size_(size__)
{
      bool is_neg = false;
      double fraction;
      int exponent;

	/* We return 'bx for a NaN or +/- infinity. */
      if (val != val || (val && (val == 0.5*val)))  {
	    allocate_words_(size_, WORD_X_ABITS, WORD_X_BBITS);
	    return;
      }

	/* Convert to a positive result. */
      if (val < 0.0) {
	    is_neg = true;
	    val = -val;
      }
      allocate_words_(size_, WORD_0_ABITS, WORD_0_BBITS);

	/* Get the exponent and fractional part of the number. */
      fraction = frexp(val, &exponent);

	/* If the value is small enough just use lround(). */
      if (exponent < BITS_PER_WORD-2) {
	    if (is_neg) this->invert();  // Invert the bits if negative.
	    long sval = lround(val);
	    if (is_neg) sval = -sval;
	      /* This requires that 0 and 1 have the same bbit value. */
	    if (size_ > BITS_PER_WORD) {
		  abits_ptr_[0] = sval;
	    } else {
		  abits_val_ = sval;
	    }
	    return;
      }

      unsigned nwords = (exponent-1) / BITS_PER_WORD;
      unsigned my_words = (size_ + BITS_PER_WORD - 1) / BITS_PER_WORD - 1;

      fraction = ldexp(fraction, (exponent-1) % BITS_PER_WORD + 1);

	/* Skip any leading bits. */
      for (int idx = (signed) nwords; idx > (signed) my_words; idx -=1) {
	    unsigned long bits = (unsigned long) fraction;
	    fraction = fraction - (double) bits;
	    fraction = ldexp(fraction, BITS_PER_WORD);
      }

	/* Convert the remaining bits as appropriate. */
      if (my_words == 0) {
		  unsigned long bits = (unsigned long) fraction;
		  abits_val_ = bits;
		  fraction = fraction - (double) bits;
		    /* Round any fractional part up. */
		  if (fraction >= 0.5) *this += (int64_t) 1;
      } else {
	    if (nwords < my_words) my_words = nwords;
	    for (int idx = (signed)my_words; idx >= 0; idx -= 1) {
		  unsigned long bits = (unsigned long) fraction;
		  abits_ptr_[idx] = bits;
		  fraction = fraction - (double) bits;
		  fraction = ldexp(fraction, BITS_PER_WORD);
	    }
	      /* Round any fractional part up. */
	    if (fraction >= ldexp(0.5, BITS_PER_WORD)) *this += (int64_t) 1;
       }

	/* Convert to a negative number if needed. */
      if (is_neg) {
	    this->invert();
	    *this += (int64_t) 1;
      }
}

vvp_vector4_t::vvp_vector4_t(const vvp_vector4_t&that,
			    unsigned adr, unsigned wid)
{
      size_ = wid;
      assert((adr + wid) <= that.size_);

      allocate_words_(wid, WORD_X_ABITS, WORD_X_BBITS);

      if (wid > BITS_PER_WORD) {
	      /* In this case, the subvector and the source vector are
		 long. Do the transfer reasonably efficiently. */
	    unsigned ptr = adr / BITS_PER_WORD;
	    unsigned long off = adr % BITS_PER_WORD;
	    unsigned long noff = BITS_PER_WORD - off;
	    unsigned long lmask = (1UL << off) - 1UL;
	    unsigned trans = 0;
	    unsigned dst = 0;
	    while (trans < wid) {
		    // The low bits of the result.
		  abits_ptr_[dst] = (that.abits_ptr_[ptr] & ~lmask) >> off;
		  bbits_ptr_[dst] = (that.bbits_ptr_[ptr] & ~lmask) >> off;
		  trans += noff;

		  if (trans >= wid)
			break;

		  ptr += 1;

		    // The high bits of the result. Skip this if the
		    // source and destination are perfectly aligned.
		  if (noff != BITS_PER_WORD) {
			abits_ptr_[dst] |= (that.abits_ptr_[ptr]&lmask) << noff;
			bbits_ptr_[dst] |= (that.bbits_ptr_[ptr]&lmask) << noff;
			trans += off;
		  }

		  dst += 1;
	    }

      } else if (that.size_ > BITS_PER_WORD) {
	      /* In this case, the subvector fits in a single word,
		 but the source is large. */
	    unsigned ptr = adr / BITS_PER_WORD;
	    unsigned long off = adr % BITS_PER_WORD;
	    unsigned trans = BITS_PER_WORD - off;
	    if (trans > wid)
		  trans = wid;

	    if (trans == BITS_PER_WORD) {
		    // Very special case: Copy exactly 1 perfectly
		    // aligned word.
		  abits_val_ = that.abits_ptr_[ptr];
		  bbits_val_ = that.bbits_ptr_[ptr];

	    } else {
		    // lmask is the low bits of the destination,
		    // masked into the source.
		  unsigned long lmask = (1UL<<trans) - 1UL;
		  lmask <<= off;

		    // The low bits of the result.
		  abits_val_ = (that.abits_ptr_[ptr] & lmask) >> off;
		  bbits_val_ = (that.bbits_ptr_[ptr] & lmask) >> off;

		  if (trans < wid) {
			  // If there are more bits, then get them
			  // from the bottom of the next word of the
			  // source.
			unsigned long hmask = (1UL << (wid-trans)) - 1UL;

			  // The high bits of the result.
			abits_val_ |= (that.abits_ptr_[ptr+1]&hmask) << trans;
			bbits_val_ |= (that.bbits_ptr_[ptr+1]&hmask) << trans;
		  }
	    }

      } else if (size_ == BITS_PER_WORD) {
	      /* We know that source and destination are short. If the
		 destination is a full word, then we know the copy is
		 aligned and complete. */
	    abits_val_ = that.abits_val_;
	    bbits_val_ = that.bbits_val_;

      } else {
	      /* Finally, the source and destination vectors are both
		 short, so there is a single mask/shift/copy. */
	    unsigned long mask = (1UL << size_) - 1UL;
	    mask <<= adr;

	    abits_val_ = (that.abits_val_ & mask) >> adr;
	    bbits_val_ = (that.bbits_val_ & mask) >> adr;
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
	    if (newcnt == cnt) {
		    // If the word count doesn't change, then there is
		    // no need for re-allocation so we are done now.
		  size_ = newsize;
		  return;
	    }

	    unsigned long*newbits = new unsigned long[2*newcnt];

	    if (cnt > 1) {
		  unsigned trans = cnt;
		  if (trans > newcnt)
			trans = newcnt;

		  for (unsigned idx = 0 ;  idx < trans ;  idx += 1)
			newbits[idx] = abits_ptr_[idx];
		  for (unsigned idx = 0 ;  idx < trans ;  idx += 1)
			newbits[newcnt+idx] = bbits_ptr_[idx];

		  delete[]abits_ptr_;

	    } else {
		  newbits[0] = abits_val_;
		  newbits[newcnt] = bbits_val_;
	    }

	    for (unsigned idx = cnt ;  idx < newcnt ;  idx += 1)
		  newbits[idx] = WORD_X_ABITS;
	    for (unsigned idx = cnt ;  idx < newcnt ;  idx += 1)
		  newbits[newcnt+idx] = WORD_X_BBITS;

	    size_ = newsize;
	    abits_ptr_ = newbits;
	    bbits_ptr_ = newbits + newcnt;

      } else {
	    if (cnt > 1) {
		  unsigned long newvala = abits_ptr_[0];
		  unsigned long newvalb = bbits_ptr_[0];
		  delete[]abits_ptr_;
		  abits_val_ = newvala;
		  bbits_val_ = newvalb;
	    }

	    size_ = newsize;
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
		 entire value of the vector4 is within the xbits_val_
		 so we know that the result is a single word, the
		 source is a single word, and we just have to loop
		 through that word. */
	    unsigned long atmp = abits_val_ >> adr;
	    unsigned long btmp = bbits_val_ >> adr;
	    if (wid < BIT2_PER_WORD) {
		  atmp &= (1UL << wid) - 1;
		  btmp &= (1UL << wid) - 1;
	    }
	    if (btmp) goto x_out;

	    val[0] = atmp;

      } else {

	    unsigned val_ptr = 0;
	    unsigned val_off = 0;

	      /* Get the first word we are scanning. We may in fact be
		 somewhere in the middle of that word. */
	    while (wid > 0) {
		  unsigned long atmp = abits_ptr_[adr/BITS_PER_WORD];
		  unsigned long btmp = bbits_ptr_[adr/BITS_PER_WORD];
		  unsigned long off = adr%BITS_PER_WORD;
		  atmp >>= off;
		  btmp >>= off;

		  unsigned long trans = BITS_PER_WORD - off;
		  if (trans > (BIT2_PER_WORD - val_off))
			trans = BIT2_PER_WORD - val_off;
		  if (wid < trans)
			trans = wid;
		  if (trans < BIT2_PER_WORD) {
			atmp &= (1UL << trans) - 1;
			btmp &= (1UL << trans) - 1;
		  }
		  if (btmp) goto x_out;

		  val[val_ptr] |= atmp << val_off;
		  adr += trans;
		  wid -= trans;
		  val_off += trans;
		  if (val_off == BIT2_PER_WORD) {
			val_ptr += 1;
			val_off = 0;
		  }
	    }
      }

      return val;

 x_out:
      delete[]val;
      return 0;
}

void vvp_vector4_t::setarray(unsigned adr, unsigned wid, const unsigned long*val)
{
      assert(adr+wid <= size_);

      const unsigned BIT2_PER_WORD = 8*sizeof(unsigned long);

      if (size_ <= BITS_PER_WORD) {
	      // We know here that both the source and the target are
	      // within a single word. Write the bits into the
	      // abits_val_ directly.

	    assert(BIT2_PER_WORD <= BITS_PER_WORD);
	    unsigned long lmask = (1UL << adr) - 1UL;
	    unsigned long hmask = ((adr+wid) < BITS_PER_WORD)
		  ? -1UL << (adr+wid)
		  : 0;
	    unsigned long mask = ~(hmask | lmask);

	    abits_val_ &= ~mask;
	    bbits_val_ &= ~mask;

	    abits_val_ |= mask & (val[0] << adr);

      } else {
	      // The general case, there are multiple words of
	      // destination, and possibly multiple words of source
	      // data. Shift and mask as we go.
	    unsigned off = adr % BITS_PER_WORD;
	    unsigned ptr = adr / BITS_PER_WORD;
	    unsigned val_off = 0;
	    unsigned val_ptr = 0;
	    while (wid > 0) {
		  unsigned trans = wid;
		  if (trans > (BIT2_PER_WORD-val_off))
			trans = BIT2_PER_WORD-val_off;
		  if (trans > (BITS_PER_WORD-off))
			trans = BITS_PER_WORD-off;

		  unsigned long lmask = (1UL << off) - 1UL;
		  unsigned long hmask = ((off+trans) < BITS_PER_WORD)
			? -1UL << (off+trans)
			: 0;
		  unsigned long mask = ~(hmask | lmask);

		  abits_ptr_[ptr] &= ~mask;
		  bbits_ptr_[ptr] &= ~mask;
		  if (val_off >= off)
			abits_ptr_[ptr] |= mask & (val[val_ptr] >> (val_off-off));
		  else
			abits_ptr_[ptr] |= mask & (val[val_ptr] << (off-val_off));

		  wid -= trans;
		  val_off += trans;
		  if (val_off == BIT2_PER_WORD) {
			val_off = 0;
			val_ptr += 1;
		  }
		  off += trans;
		  if (off == BITS_PER_WORD) {
			off = 0;
			ptr += 1;
		  }
	    }
      }
}

/*
 * Set the bits of that vector, which must be a subset of this vector,
 * into the addressed part of this vector. Use bit masking and word
 * copies to go as fast as reasonably possible.
 */
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

	    unsigned long lmask = (1UL << adr) - 1;
	    unsigned long hmask;
	    unsigned long hshift = adr+that.size_;
	    if (hshift >= BITS_PER_WORD)
		  hmask = -1UL;
	    else
		  hmask = (1UL << (adr+that.size_)) - 1;
	    unsigned long mask = hmask & ~lmask;

	    abits_val_ =
		  (abits_val_ & ~mask)
		  | ((that.abits_val_<<adr) & mask);
	    bbits_val_ =
		  (bbits_val_ & ~mask)
		  | ((that.bbits_val_<<adr) & mask);

      } else if (that.size_ <= BITS_PER_WORD) {

	      /* This vector is more than a word, but that vector is
		 still small. Write into the destination, possibly
		 spanning two destination words, depending on whether
		 the source vector spans a word transition. */
	    unsigned long dptr = adr / BITS_PER_WORD;
	    unsigned long doff = adr % BITS_PER_WORD;

	    unsigned long lmask = (1UL << doff) - 1;
	    unsigned long hshift = doff+that.size_;
	    unsigned long hmask;
	    if (hshift >= BITS_PER_WORD)
		  hmask = -1UL;
	    else
		  hmask = (1UL << hshift) - 1UL;

	    unsigned long mask = hmask & ~lmask;

	    abits_ptr_[dptr] =
		  (abits_ptr_[dptr] & ~mask)
		  | ((that.abits_val_ << doff) & mask);
	    bbits_ptr_[dptr] =
		  (bbits_ptr_[dptr] & ~mask)
		  | ((that.bbits_val_ << doff) & mask);

	    if ((doff + that.size_) > BITS_PER_WORD) {
		  unsigned tail = doff + that.size_ - BITS_PER_WORD;
		  mask = (1UL << tail) - 1;

		  dptr += 1;
		  abits_ptr_[dptr] =
			(abits_ptr_[dptr] & ~mask)
			| ((that.abits_val_ >> (that.size_-tail)) & mask);
		  bbits_ptr_[dptr] =
			(bbits_ptr_[dptr] & ~mask)
			| ((that.bbits_val_ >> (that.size_-tail)) & mask);
	    }

      } else if (adr%BITS_PER_WORD == 0) {

	      /* In this case, both vectors are long, but the
		 destination is neatly aligned. That means all but the
		 last word can be simply copied with no masking. */

	    unsigned remain = that.size_;
	    unsigned sptr = 0;
	    unsigned dptr = adr / BITS_PER_WORD;
	    while (remain >= BITS_PER_WORD) {
		  abits_ptr_[dptr] = that.abits_ptr_[sptr];
		  bbits_ptr_[dptr] = that.bbits_ptr_[sptr];
		  dptr += 1;
		  sptr += 1;
		  remain -= BITS_PER_WORD;
	    }

	    if (remain > 0) {
		  unsigned long mask = (1UL << remain) - 1;
		  abits_ptr_[dptr] =
			(abits_ptr_[dptr] & ~mask)
			| (that.abits_ptr_[sptr] & mask);
		  bbits_ptr_[dptr] =
			(bbits_ptr_[dptr] & ~mask)
			| (that.bbits_ptr_[sptr] & mask);
	    }

      } else {

	      /* We know that there are two long vectors, and we know
		 that the destination is definitely NOT aligned. */

	    unsigned remain = that.size_;
	    unsigned sptr = 0;
	    unsigned dptr = adr / BITS_PER_WORD;
	    unsigned doff = adr % BITS_PER_WORD;
	    unsigned long lmask = (1UL << doff) - 1;
	    unsigned ndoff = BITS_PER_WORD - doff;
	    while (remain >= BITS_PER_WORD) {
		  abits_ptr_[dptr] =
			(abits_ptr_[dptr] & lmask)
			| ((that.abits_ptr_[sptr] << doff) & ~lmask);
		  bbits_ptr_[dptr] =
			(bbits_ptr_[dptr] & lmask)
			| ((that.bbits_ptr_[sptr] << doff) & ~lmask);
		  dptr += 1;

		  abits_ptr_[dptr] =
			(abits_ptr_[dptr] & ~lmask)
			| ((that.abits_ptr_[sptr] >> ndoff) & lmask);
		  bbits_ptr_[dptr] =
			(bbits_ptr_[dptr] & ~lmask)
			| ((that.bbits_ptr_[sptr] >> ndoff) & lmask);

		  remain -= BITS_PER_WORD;
		  sptr += 1;
	    }

	    if (remain > 0) {
		  unsigned long hshift = doff+remain;
		  unsigned long hmask;
		  if (hshift >= BITS_PER_WORD)
			hmask = -1UL;
		  else
			hmask = (1UL << (doff+remain)) - 1;

		  unsigned long mask = hmask & ~lmask;

		  abits_ptr_[dptr] = (abits_ptr_[dptr] & ~mask)
		        | ((that.abits_ptr_[sptr] << doff) & mask);
		  bbits_ptr_[dptr] = (bbits_ptr_[dptr] & ~mask)
		        | ((that.bbits_ptr_[sptr] << doff) & mask);

		  if ((doff + remain) > BITS_PER_WORD) {
			unsigned tail = doff + remain - BITS_PER_WORD;
			if (tail >= BITS_PER_WORD)
			      mask = -1UL;
			else
			      mask = (1UL << tail) - 1;

			dptr += 1;
			abits_ptr_[dptr] = (abits_ptr_[dptr] & ~mask) |
			      ((that.abits_ptr_[sptr] >> (remain-tail))&mask);
			bbits_ptr_[dptr] = (bbits_ptr_[dptr] & ~mask) |
			      ((that.bbits_ptr_[sptr] >> (remain-tail))&mask);
		  }
	    }
      }

}

bool vvp_vector4_t::eeq(const vvp_vector4_t&that) const
{
      if (size_ != that.size_)
	    return false;

      if (size_ < BITS_PER_WORD) {
	    unsigned long mask = (1UL << size_) - 1;
	    return (abits_val_&mask) == (that.abits_val_&mask)
		  && (bbits_val_&mask) == (that.bbits_val_&mask);
      }

      if (size_ == BITS_PER_WORD) {
	    return (abits_val_ == that.abits_val_)
		  && (bbits_val_ == that.bbits_val_);
      }

      unsigned words = size_ / BITS_PER_WORD;
      for (unsigned idx = 0 ;  idx < words ;  idx += 1) {
	    if (abits_ptr_[idx] != that.abits_ptr_[idx])
		  return false;
	    if (bbits_ptr_[idx] != that.bbits_ptr_[idx])
		  return false;
      }

      unsigned long mask = size_%BITS_PER_WORD;
      if (mask > 0) {
	    mask = (1UL << mask) - 1;
	    return (abits_ptr_[words]&mask) == (that.abits_ptr_[words]&mask)
		  && (bbits_ptr_[words]&mask) == (that.bbits_ptr_[words]&mask);
      }

      return true;
}

bool vvp_vector4_t::has_xz() const
{
      if (size_ < BITS_PER_WORD) {
	    unsigned long mask = -1UL >> (BITS_PER_WORD - size_);
	    return bbits_val_&mask;
      }

      if (size_ == BITS_PER_WORD) {
	    return bbits_val_;
      }

      unsigned words = size_ / BITS_PER_WORD;
      for (unsigned idx = 0 ; idx < words ; idx += 1) {
	    if (bbits_ptr_[idx])
		  return true;
      }

      unsigned long mask = size_%BITS_PER_WORD;
      if (mask > 0) {
	    mask = -1UL >> (BITS_PER_WORD - mask);
	    return bbits_ptr_[words]&mask;
      }

      return false;
}

void vvp_vector4_t::change_z2x()
{
	// This method relies on the fact that both BIT4_X and BIT4_Z
	// have the bbit set in the vector4 encoding, and also that
	// the BIT4_X has abit set in the vector4 encoding. By simply
	// or-ing the bbit into the abit, BIT4_X and BIT4_Z both
	// become BIT4_X.

      if (size_ <= BITS_PER_WORD) {
	    abits_val_ |= bbits_val_;
      } else {
	    unsigned words = (size_+BITS_PER_WORD-1) / BITS_PER_WORD;
	    for (unsigned idx = 0 ;  idx < words ;  idx += 1)
		  abits_ptr_[idx] |= bbits_ptr_[idx];
      }
}

void vvp_vector4_t::set_to_x()
{
      if (size_ <= BITS_PER_WORD) {
	    abits_val_ = vvp_vector4_t::WORD_X_ABITS;
            bbits_val_ = vvp_vector4_t::WORD_X_BBITS;
      } else {
	    unsigned words = (size_+BITS_PER_WORD-1) / BITS_PER_WORD;
	    for (unsigned idx = 0 ;  idx < words ;  idx += 1) {
		  abits_ptr_[idx] = vvp_vector4_t::WORD_X_ABITS;
                  bbits_ptr_[idx] = vvp_vector4_t::WORD_X_BBITS;
            }
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

void vvp_vector4_t::invert()
{
      if (size_ <= BITS_PER_WORD) {
	    unsigned long mask = (size_<BITS_PER_WORD)? (1UL<<size_)-1UL : -1UL;
	    abits_val_ = mask & ~abits_val_;
	    abits_val_ |= bbits_val_;
      } else {
	    unsigned remaining = size_;
	    unsigned idx = 0;
	    while (remaining >= BITS_PER_WORD) {
		  abits_ptr_[idx] = ~abits_ptr_[idx];
		  abits_ptr_[idx] |= bbits_ptr_[idx];
		  idx += 1;
		  remaining -= BITS_PER_WORD;
	    }
	    if (remaining > 0) {
		  unsigned long mask = (1UL<<remaining) - 1UL;
		  abits_ptr_[idx] = mask & ~abits_ptr_[idx];
		  abits_ptr_[idx] |= bbits_ptr_[idx];
	    }
      }
}

vvp_vector4_t& vvp_vector4_t::operator &= (const vvp_vector4_t&that)
{
	// The truth table is:
	//     00 01 11 10
	//  00 00 00 00 00
	//  01 00 01 11 11
	//  11 00 11 11 11
	//  10 00 11 11 11
      if (size_ <= BITS_PER_WORD) {
	    unsigned long tmp1 = abits_val_ | bbits_val_;
	    unsigned long tmp2 = that.abits_val_ | that.bbits_val_;
	    abits_val_ = tmp1 & tmp2;
	    bbits_val_ = (tmp1 & that.bbits_val_) | (tmp2 & bbits_val_);
      } else {
	    unsigned words = (size_ + BITS_PER_WORD - 1) / BITS_PER_WORD;
	    for (unsigned idx = 0; idx < words ; idx += 1) {
		  unsigned long tmp1 = abits_ptr_[idx] | bbits_ptr_[idx];
		  unsigned long tmp2 = that.abits_ptr_[idx] |
		                       that.bbits_ptr_[idx];
		  abits_ptr_[idx] = tmp1 & tmp2;
		  bbits_ptr_[idx] = (tmp1 & that.bbits_ptr_[idx]) |
		                    (tmp2 & bbits_ptr_[idx]);
	    }
      }

      return *this;
}

vvp_vector4_t& vvp_vector4_t::operator |= (const vvp_vector4_t&that)
{
	// The truth table is:
	//     00 01 11 10
	//  00 00 01 11 11
	//  01 01 01 01 01
	//  11 11 01 11 11
	//  10 11 01 11 11
      if (size_ <= BITS_PER_WORD) {
	    unsigned long tmp = abits_val_ | bbits_val_ |
	                        that.abits_val_ | that.bbits_val_;
	    bbits_val_ = ((~abits_val_ | bbits_val_) & that.bbits_val_) |
	                 ((~that.abits_val_ | that.bbits_val_) & bbits_val_);
	    abits_val_ = tmp;

      } else {
	    unsigned words = (size_ + BITS_PER_WORD - 1) / BITS_PER_WORD;
	    for (unsigned idx = 0; idx < words ; idx += 1) {
		  unsigned long tmp = abits_ptr_[idx] | bbits_ptr_[idx] |
	                        that.abits_ptr_[idx] | that.bbits_ptr_[idx];
		  bbits_ptr_[idx] = ((~abits_ptr_[idx] | bbits_ptr_[idx]) &
		                     that.bbits_ptr_[idx]) |
		                    ((~that.abits_ptr_[idx] |
		                      that.bbits_ptr_[idx]) & bbits_ptr_[idx]);
		  abits_ptr_[idx] = tmp;
	    }
      }

      return *this;
}

/*
* Add an integer to the vvp_vector4_t in place, bit by bit so that
* there is no size limitations.
*/
vvp_vector4_t& vvp_vector4_t::operator += (int64_t that)
{
      vvp_bit4_t carry = BIT4_0;
      unsigned idx;

      if (has_xz()) {
	    vvp_vector4_t xxx (size(), BIT4_X);
	    *this = xxx;
	    return *this;
      }

      for (idx = 0 ; idx < size() ; idx += 1) {
	    if (that == 0 && carry==BIT4_0)
		  break;

	    vvp_bit4_t that_bit = (that&1)? BIT4_1 : BIT4_0;
	    that >>= 1;

	    if (that_bit==BIT4_0 && carry==BIT4_0)
		  continue;

	    vvp_bit4_t bit = value(idx);
	    bit = add_with_carry(bit, that_bit, carry);

	    set_bit(idx, bit);
      }

      return *this;
}

ostream& operator<< (ostream&out, const vvp_vector4_t&that)
{
      out << that.size() << "'b";
      for (unsigned idx = 0 ;  idx < that.size() ;  idx += 1)
	    out << that.value(that.size()-idx-1);
      return out;
}

bool vector4_to_value(const vvp_vector4_t&vec, long&val,
		      bool is_signed, bool is_arithmetic)
{
      long res = 0;
      long msk = 1;
      bool rc_flag = true;

      unsigned size = vec.size();
      if (size > 8*sizeof(val)) size = 8*sizeof(val);
      for (unsigned idx = 0 ;  idx < size ;  idx += 1) {
	    switch (vec.value(idx)) {
		case BIT4_0:
		  break;
		case BIT4_1:
		  res |= msk;
		  break;
		default:
		  if (is_arithmetic)
			return false;
		  else
			rc_flag = false;
	    }

	    msk <<= 1L;
      }

      if (is_signed && vec.value(vec.size()-1) == BIT4_1) {
	    if (vec.size() < 8*sizeof(val))
		  res |= (-1L) << vec.size();
      }

      val = res;
      return rc_flag;
}

bool vector4_to_value(const vvp_vector4_t&vec, unsigned long&val)
{
      unsigned long res = 0;
      unsigned long msk = 1;

      unsigned size = vec.size();
      if (size > 8*sizeof(val)) size = 8*sizeof(val);
      for (unsigned idx = 0 ;  idx < size ;  idx += 1) {
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

#ifndef UL_AND_TIME64_SAME
bool vector4_to_value(const vvp_vector4_t&vec, int64_t&val,
		      bool is_signed, bool is_arithmetic)
{
      long res = 0;
      long msk = 1;
      bool rc_flag = true;

      unsigned size = vec.size();
      if (size > 8*sizeof(val)) size = 8*sizeof(val);
      for (unsigned idx = 0 ;  idx < size ;  idx += 1) {
	    switch (vec.value(idx)) {
		case BIT4_0:
		  break;
		case BIT4_1:
		  res |= msk;
		  break;
		default:
		  if (is_arithmetic)
			return false;
		  else
			rc_flag = false;
	    }

	    msk <<= 1L;
      }

      if (is_signed && vec.value(vec.size()-1) == BIT4_1) {
	    if (vec.size() < 8*sizeof(val))
		  res |= (-1L) << vec.size();
      }

      val = res;
      return rc_flag;
}

bool vector4_to_value(const vvp_vector4_t&vec, vvp_time64_t&val)
{
      vvp_time64_t res = 0;
      vvp_time64_t msk = 1;

      unsigned size = vec.size();
      if (size > 8*sizeof(val)) size = 8*sizeof(val);
      for (unsigned idx = 0 ;  idx < size ;  idx += 1) {
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
#endif

bool vector4_to_value(const vvp_vector4_t&vec, double&val, bool signed_flag)
{

      if (vec.size() == 0) {
	    val = 0.0;
	    return true;
      }

      bool flag = true;

      if (vec.value(vec.size()-1) != BIT4_1) {
	    signed_flag = false;
      }

      double res = 0.0;
      if (signed_flag) {
	    vvp_bit4_t carry = BIT4_1;
	    for (unsigned idx = 0 ;  idx < vec.size() ;  idx += 1) {
		  vvp_bit4_t a = ~vec.value(idx);
		  vvp_bit4_t x = add_with_carry(a, BIT4_0, carry);
		  switch (x) {
		      case BIT4_0:
			break;
		      case BIT4_1:
			res += pow(2.0, (int)idx);
			break;
		      default:
			flag = false;
		  }
	    }
	    res *= -1.0;
      } else {
	    for (unsigned idx = 0 ;  idx < vec.size() ;  idx += 1) {
		  switch (vec.value(idx)) {
		      case BIT4_0:
			break;
		      case BIT4_1:
			res += pow(2.0, (int)idx);
			break;
		      default:
			flag = false;
		  }
	    }
      }
      val = res;
      return flag;
}

vvp_realarray_t::vvp_realarray_t(unsigned wor)
: words_(wor)
{
      array_ = new double[words_];
	// Real array words have a default value of zero.
      for (unsigned idx = 0 ; idx < words_; idx += 1) {
	    array_[idx] = 0.0;
      }
}

vvp_realarray_t::~vvp_realarray_t()
{
      delete[]array_;
}

void vvp_realarray_t::set_word(unsigned word, double value)
{
      if (word >= words_)
	    return;
      array_[word] = value;
}

double vvp_realarray_t::get_word(unsigned word) const
{
      if (word >= words_)
	    return 0.0;
      else
	    return array_[word];
}

vvp_vector4array_t::vvp_vector4array_t(unsigned width__, unsigned words__)
: width_(width__), words_(words__)
{
}

vvp_vector4array_t::~vvp_vector4array_t()
{
}

void vvp_vector4array_t::set_word_(v4cell*cell, const vvp_vector4_t&that)
{
      assert(that.size_ == width_);

      if (width_ <= vvp_vector4_t::BITS_PER_WORD) {
	    cell->abits_val_ = that.abits_val_;
	    cell->bbits_val_ = that.bbits_val_;
	    return;
      }

      unsigned cnt = (width_ + vvp_vector4_t::BITS_PER_WORD-1)/vvp_vector4_t::BITS_PER_WORD;

      if (cell->abits_ptr_ == 0) {
	    cell->abits_ptr_ = new unsigned long[2*cnt];
	    cell->bbits_ptr_ = cell->abits_ptr_ + cnt;
      }

      for (unsigned idx = 0 ; idx < cnt ; idx += 1)
	    cell->abits_ptr_[idx] = that.abits_ptr_[idx];
      for (unsigned idx = 0 ; idx < cnt ; idx += 1)
	    cell->bbits_ptr_[idx] = that.bbits_ptr_[idx];
}

vvp_vector4_t vvp_vector4array_t::get_word_(v4cell*cell) const
{
      if (width_ <= vvp_vector4_t::BITS_PER_WORD) {
	    vvp_vector4_t res;
	    res.size_ = width_;
	    res.abits_val_ = cell->abits_val_;
	    res.bbits_val_ = cell->bbits_val_;
	    return res;
      }

      vvp_vector4_t res (width_, BIT4_X);
      if (cell->abits_ptr_ == 0)
	    return res;

      unsigned cnt = (width_ + vvp_vector4_t::BITS_PER_WORD-1)/vvp_vector4_t::BITS_PER_WORD;

      for (unsigned idx = 0 ; idx < cnt ; idx += 1)
	    res.abits_ptr_[idx] = cell->abits_ptr_[idx];
      for (unsigned idx = 0 ; idx < cnt ; idx += 1)
	    res.bbits_ptr_[idx] = cell->bbits_ptr_[idx];

      return res;
}

vvp_vector4array_sa::vvp_vector4array_sa(unsigned width__, unsigned words__)
: vvp_vector4array_t(width__, words__)
{
      array_ = new v4cell[words_];

      if (width_ <= vvp_vector4_t::BITS_PER_WORD) {
	    for (unsigned idx = 0 ; idx < words_ ; idx += 1) {
		  array_[idx].abits_val_ = vvp_vector4_t::WORD_X_ABITS;
		  array_[idx].bbits_val_ = vvp_vector4_t::WORD_X_BBITS;
	    }
      } else {
	    for (unsigned idx = 0 ; idx < words_ ; idx += 1) {
		  array_[idx].abits_ptr_ = 0;
		  array_[idx].bbits_ptr_ = 0;
	    }
      }
}

vvp_vector4array_sa::~vvp_vector4array_sa()
{
      if (array_) {
	    if (width_ > vvp_vector4_t::BITS_PER_WORD) {
		  for (unsigned idx = 0 ; idx < words_ ; idx += 1)
			if (array_[idx].abits_ptr_)
			      delete[]array_[idx].abits_ptr_;
	    }
	    delete[]array_;
      }
}

void vvp_vector4array_sa::set_word(unsigned index, const vvp_vector4_t&that)
{
      assert(index < words_);

      v4cell*cell = &array_[index];

      set_word_(cell, that);
}

vvp_vector4_t vvp_vector4array_sa::get_word(unsigned index) const
{
      if (index >= words_)
	    return vvp_vector4_t(width_, BIT4_X);

      assert(index < words_);

      v4cell*cell = &array_[index];

      return get_word_(cell);
}

vvp_vector4array_aa::vvp_vector4array_aa(unsigned width__, unsigned words__)
: vvp_vector4array_t(width__, words__)
{
      context_idx_ = vpip_add_item_to_context(this, vpip_peek_context_scope());
}

vvp_vector4array_aa::~vvp_vector4array_aa()
{
}

void vvp_vector4array_aa::alloc_instance(vvp_context_t context)
{
      v4cell*array = new v4cell[words_];

      if (width_ <= vvp_vector4_t::BITS_PER_WORD) {
	    for (unsigned idx = 0 ; idx < words_ ; idx += 1) {
		  array[idx].abits_val_ = vvp_vector4_t::WORD_X_ABITS;
		  array[idx].bbits_val_ = vvp_vector4_t::WORD_X_BBITS;
	    }
      } else {
	    for (unsigned idx = 0 ; idx < words_ ; idx += 1) {
		  array[idx].abits_ptr_ = 0;
		  array[idx].bbits_ptr_ = 0;
	    }
      }

      vvp_set_context_item(context, context_idx_, array);
}

void vvp_vector4array_aa::reset_instance(vvp_context_t context)
{
      v4cell*cell = static_cast<v4cell*>
            (vvp_get_context_item(context, context_idx_));

      if (width_ <= vvp_vector4_t::BITS_PER_WORD) {
	    for (unsigned idx = 0 ; idx < words_ ; idx += 1) {
		  cell->abits_val_ = vvp_vector4_t::WORD_X_ABITS;
		  cell->bbits_val_ = vvp_vector4_t::WORD_X_BBITS;
                  cell++;
	    }
      } else {
            unsigned cnt = (width_ + vvp_vector4_t::BITS_PER_WORD-1)/vvp_vector4_t::BITS_PER_WORD;
	    for (unsigned idx = 0 ; idx < words_ ; idx += 1) {
		  if (cell->abits_ptr_) {
                        for (unsigned n = 0 ; n < cnt ; n += 1) {
			      cell->abits_ptr_[n] = vvp_vector4_t::WORD_X_ABITS;
			      cell->bbits_ptr_[n] = vvp_vector4_t::WORD_X_BBITS;
                        }
                  }
                  cell++;
	    }
      }
}

#ifdef CHECK_WITH_VALGRIND
void vvp_vector4array_aa::free_instance(vvp_context_t context)
{
      v4cell*cell = static_cast<v4cell*>
            (vvp_get_context_item(context, context_idx_));
      delete [] cell;
}
#endif

void vvp_vector4array_aa::set_word(unsigned index, const vvp_vector4_t&that)
{
      assert(index < words_);

      v4cell*cell = static_cast<v4cell*>
            (vthread_get_wt_context_item(context_idx_)) + index;

      set_word_(cell, that);
}

vvp_vector4_t vvp_vector4array_aa::get_word(unsigned index) const
{
      if (index >= words_)
	    return vvp_vector4_t(width_, BIT4_X);

      assert(index < words_);

      v4cell*cell = static_cast<v4cell*>
            (vthread_get_rd_context_item(context_idx_)) + index;

      return get_word_(cell);
}

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
      vec_[0] = v;
      for (unsigned idx = 1 ;  idx < words ;  idx += 1)
	    vec_[idx] = 0;
}

vvp_vector2_t::vvp_vector2_t(vvp_vector2_t::fill_t fill, unsigned wid)
{
      wid_ = wid;
      const unsigned bits_per_word = 8 * sizeof(vec_[0]);
      const unsigned words = (wid_ + bits_per_word-1) / bits_per_word;

      vec_ = new unsigned long[words];
      for (unsigned idx = 0 ;  idx < words ;  idx += 1)
	    vec_[idx] = fill? -1 : 0;
}

vvp_vector2_t::vvp_vector2_t(const vvp_vector4_t&that)
{
      wid_ = that.size();
      const unsigned words = (that.size() + BITS_PER_WORD-1) / BITS_PER_WORD;

      if (words == 0) {
	    vec_ = 0;
	    wid_ = 0;
	    return;
      }

      vec_ = new unsigned long[words];
      for (unsigned idx = 0 ;  idx < words ;  idx += 1)
	    vec_[idx] = 0;

      for (unsigned idx = 0 ;  idx < that.size() ;  idx += 1) {
	    unsigned addr = idx / BITS_PER_WORD;
	    unsigned shift = idx % BITS_PER_WORD;

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

void vvp_vector2_t::copy_from_that_(const vvp_vector2_t&that)
{
      wid_ = that.wid_;
      const unsigned words = (wid_ + BITS_PER_WORD-1) / BITS_PER_WORD;

      if (words == 0) {
	    vec_ = 0;
	    wid_ = 0;
	    return;
      }

      vec_ = new unsigned long[words];
      for (unsigned idx = 0 ;  idx < words ;  idx += 1)
	    vec_[idx] = that.vec_[idx];
}

vvp_vector2_t::vvp_vector2_t(const vvp_vector2_t&that)
{
      copy_from_that_(that);
}

vvp_vector2_t::vvp_vector2_t(const vvp_vector2_t&that, unsigned newsize)
{
      wid_ = newsize;
      if (newsize == 0) {
	    vec_ = 0;
	    return;
      }

      const unsigned words = (wid_ + BITS_PER_WORD-1) / BITS_PER_WORD;
      const unsigned twords = (that.wid_ + BITS_PER_WORD-1) / BITS_PER_WORD;

      vec_ = new unsigned long[words];
      for (unsigned idx = 0 ;  idx < words ;  idx += 1) {
	    if (idx < twords)
		  vec_[idx] = that.vec_[idx];
	    else
		  vec_[idx] = 0;
      }
}

vvp_vector2_t& vvp_vector2_t::operator= (const vvp_vector2_t&that)
{
      if (this == &that)
	    return *this;

      if (vec_) {
	    delete[]vec_;
	    vec_ = 0;
      }

      copy_from_that_(that);
      return *this;
}

vvp_vector2_t& vvp_vector2_t::operator <<= (unsigned int shift)
{
      if (wid_ == 0)
	    return *this;

      const unsigned words = (wid_ + BITS_PER_WORD-1) / BITS_PER_WORD;

	// Number of words to shift
      const unsigned wshift = shift / BITS_PER_WORD;
	// bits to shift within each word.
      const unsigned long oshift = shift % BITS_PER_WORD;

	// If shifting the entire value away, then return zeros.
      if (wshift >= words) {
	    for (unsigned idx = 0 ;  idx < words ;  idx += 1)
		  vec_[idx] = 0;

	    return *this;
      }

	// Do the word shift first.
      if (wshift > 0) {
	    for (unsigned idx = 0 ;  idx < words-wshift ;  idx += 1) {
		  unsigned sel = words - idx - 1;
		  vec_[sel] = vec_[sel-wshift];
	    }

	    for (unsigned idx = 0 ;  idx < wshift ;  idx += 1)
		  vec_[idx] = 0;
      }

	// Do the fine shift.
      if (oshift != 0) {
	    unsigned long pad = 0;
	    for (unsigned idx = 0 ;  idx < words ;  idx += 1) {
		  unsigned long next_pad = vec_[idx] >> (BITS_PER_WORD-oshift);
		  vec_[idx] = (vec_[idx] << oshift) | pad;
		  pad = next_pad;
	    }

	      // Cleanup the tail bits.
	    unsigned long mask = -1UL >> (BITS_PER_WORD - wid_%BITS_PER_WORD);
	    vec_[words-1] &= mask;
      }

      return *this;
}

vvp_vector2_t& vvp_vector2_t::operator >>= (unsigned shift)
{
      if (wid_ == 0)
	    return *this;

      const unsigned words = (wid_ + BITS_PER_WORD-1) / BITS_PER_WORD;

	// Number of words to shift
      const unsigned wshift = shift / BITS_PER_WORD;
	// bits to shift within each word.
      const unsigned long oshift = shift % BITS_PER_WORD;

	// If shifting the entire value away, then return zeros.
      if (wshift >= words) {
	    for (unsigned idx = 0 ;  idx < words ;  idx += 1)
		  vec_[idx] = 0;

	    return *this;
      }

      if (wshift > 0) {
	    for (unsigned idx = 0 ;  idx < words-wshift ;  idx += 1)
		  vec_[idx] = vec_[idx+wshift];

	    for (unsigned idx = words-wshift ;  idx < words ;  idx += 1)
		  vec_[idx] = 0;
      }

      if (oshift > 0) {
	    unsigned long pad = 0;
	    for (unsigned idx = words ;  idx > 0 ;  idx -= 1) {
		  unsigned long new_pad = vec_[idx-1] <<(BITS_PER_WORD-oshift);
		  vec_[idx-1] = pad | (vec_[idx-1] >> oshift);
		  pad = new_pad;
	    }

	      // Cleanup the tail bits.

	    unsigned use_words = words;
	      // Mask_shift is the number of high bits of the top word
	      // that are to be masked off. We start with the number
	      // of bits that are not included even in the original,
	      // then we include the bits of the shift, that are to be
	      // masked to zero.
	    unsigned long mask_shift = BITS_PER_WORD - wid_%BITS_PER_WORD;
	    mask_shift %= BITS_PER_WORD;
	    mask_shift += oshift;
	    while (mask_shift >= BITS_PER_WORD) {
		  vec_[use_words-1] = 0;
		  use_words -= 1;
		  mask_shift -= BITS_PER_WORD;
	    }
	    if (mask_shift > 0) {
		  assert(use_words > 0);
		  unsigned long mask = -1UL >> mask_shift;
		  vec_[use_words-1] &= mask;
	    }
      }

      return *this;
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

vvp_vector2_t& vvp_vector2_t::operator += (const vvp_vector2_t&that)
{
      assert(wid_ == that.wid_);
      if (wid_ == 0)
	    return *this;

      const unsigned words = (wid_ + BITS_PER_WORD-1) / BITS_PER_WORD;

      unsigned long carry = 0;
      for (unsigned idx = 0 ;  idx < words ;  idx += 1) {
	    vec_[idx] = add_carry(vec_[idx], that.vec_[idx], carry);
      }


	// Cleanup the tail bits.
      unsigned long mask = -1UL >> (BITS_PER_WORD - wid_%BITS_PER_WORD);
      vec_[words-1] &= mask;

      return *this;
}

vvp_vector2_t& vvp_vector2_t::operator -= (const vvp_vector2_t&that)
{
      assert(wid_ == that.wid_);
      if (wid_ == 0)
	    return *this;

      const unsigned words = (wid_ + BITS_PER_WORD-1) / BITS_PER_WORD;

      unsigned long carry = 1;
      for (unsigned idx = 0 ;  idx < words ;  idx += 1) {
	    vec_[idx] = add_carry(vec_[idx], ~that.vec_[idx], carry);
      }

      return *this;
}

vvp_vector2_t::~vvp_vector2_t()
{
      if (vec_) delete[]vec_;
}

void vvp_vector2_t::trim()
{
      while (value(wid_-1) == 0 && wid_ > 1) wid_ -= 1;
}

/* This is a special trim that is used on numbers we know represent a
 * negative signed value (they came from a negative real value). */
void vvp_vector2_t::trim_neg()
{
      if (value(wid_-1) == 1 && wid_ > 32) {
	    while (value(wid_-2) == 1 && wid_ > 32) wid_ -= 1;
      }
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

void vvp_vector2_t::set_bit(unsigned idx, int bit)
{
      assert(idx < wid_);

      const unsigned bits_per_word = 8 * sizeof(vec_[0]);
      unsigned addr = idx/bits_per_word;
      unsigned long mask = idx%bits_per_word;

      if (bit)
	    vec_[addr] |= 1UL << mask;
      else
	    vec_[addr] &= ~(1UL << mask);
}

bool vvp_vector2_t::is_NaN() const
{
      return wid_ == 0;
}

bool vvp_vector2_t::is_zero() const
{
      const unsigned words = (wid_ + BITS_PER_WORD-1) / BITS_PER_WORD;

      for (unsigned idx = 0; idx < words; idx += 1) {
	    if (vec_[idx] == 0) continue;
	    return false;
      }

      return true;
}

/*
 * Basic idea from "Introduction to Programming using SML" by
 * Michael R. Hansen and Hans Rischel page 261 and "Seminumerical
 * Algorithms, Third Edition" by Donald E. Knuth section 4.6.3.
 */
vvp_vector2_t pow(const vvp_vector2_t&x, vvp_vector2_t&y)
{
        /* If we have a zero exponent just return a 1 bit wide 1. */
      if (y == vvp_vector2_t(0L, 1)) {
	    return vvp_vector2_t(1L, 1);
      }

        /* Is the value odd? */
      if (y.value(0) == 1) {
	    y.set_bit(0, 0);  // A quick subtract by 1.
	    vvp_vector2_t res = x * pow(x, y);
	    res.trim();  // To keep the size under control trim extra zeros.
	    return res;
      }

      y >>= 1;  // A fast divide by two. We know the LSB is zero.
      vvp_vector2_t z = pow(x, y);
      vvp_vector2_t res = z * z;
      res.trim();  // To keep the size under control trim extra zeros.
      return res;
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

static void div_mod (vvp_vector2_t dividend, vvp_vector2_t divisor,
		     vvp_vector2_t&quotient, vvp_vector2_t&remainder)
{

      quotient = vvp_vector2_t(0, dividend.size());

      if (divisor == quotient) {
	    cerr << "ERROR: division by zero, exiting." << endl;
	    exit(255);
      }

      if (dividend < divisor) {
	    remainder = dividend;
	    return;
      }

      vvp_vector2_t mask (1, dividend.size());

	// Make the dividend 1 bit larger to prevent overflow of
	// divtmp in startup.
      dividend = vvp_vector2_t(dividend, dividend.size()+1);
      vvp_vector2_t divtmp (divisor, dividend.size());

      while (divtmp < dividend) {
	    divtmp <<= 1;
	    mask <<= 1;
      }

      while (dividend >= divisor) {
	    if (divtmp <= dividend) {
		  dividend -= divtmp;
		  quotient += mask;
	    }

	    divtmp >>= 1;
	    mask >>= 1;
      }

      remainder = vvp_vector2_t(dividend, mask.size());
}

vvp_vector2_t operator - (const vvp_vector2_t&that)
{
      vvp_vector2_t neg(that);
      if (neg.wid_ == 0) return neg;

      const unsigned words = (neg.wid_ + neg.BITS_PER_WORD-1) /
                                         neg.BITS_PER_WORD;
      for (unsigned idx = 0 ;  idx < words ;  idx += 1) {
	    neg.vec_[idx] = ~neg.vec_[idx];
      }
      neg += vvp_vector2_t(1, neg.wid_);

      return neg;
}

vvp_vector2_t operator / (const vvp_vector2_t&dividend,
			  const vvp_vector2_t&divisor)
{
      vvp_vector2_t quot, rem;
      div_mod(dividend, divisor, quot, rem);
      return quot;
}

vvp_vector2_t operator % (const vvp_vector2_t&dividend,
			  const vvp_vector2_t&divisor)
{
      vvp_vector2_t quot, rem;
      div_mod(dividend, divisor, quot, rem);
      return rem;
}

bool operator > (const vvp_vector2_t&a, const vvp_vector2_t&b)
{
      const unsigned awords = (a.wid_ + vvp_vector2_t::BITS_PER_WORD-1) / vvp_vector2_t::BITS_PER_WORD;
      const unsigned bwords = (b.wid_ + vvp_vector2_t::BITS_PER_WORD-1) / vvp_vector2_t::BITS_PER_WORD;

      const unsigned words = awords > bwords? awords : bwords;

      for (unsigned idx = words ;  idx > 0 ;  idx -= 1) {
	    unsigned long aw = (idx <= awords)? a.vec_[idx-1] : 0;
	    unsigned long bw = (idx <= bwords)? b.vec_[idx-1] : 0;

	    if (aw > bw)
		  return true;
	    if (aw < bw)
		  return false;
      }

	// If the above loop finishes, then the vectors are equal.
      return false;
}

bool operator >= (const vvp_vector2_t&a, const vvp_vector2_t&b)
{
      const unsigned awords = (a.wid_ + vvp_vector2_t::BITS_PER_WORD-1) / vvp_vector2_t::BITS_PER_WORD;
      const unsigned bwords = (b.wid_ + vvp_vector2_t::BITS_PER_WORD-1) / vvp_vector2_t::BITS_PER_WORD;

      const unsigned words = awords > bwords? awords : bwords;

      for (unsigned idx = words ;  idx > 0 ;  idx -= 1) {
	    unsigned long aw = (idx <= awords)? a.vec_[idx-1] : 0;
	    unsigned long bw = (idx <= bwords)? b.vec_[idx-1] : 0;

	    if (aw > bw)
		  return true;
	    if (aw < bw)
		  return false;
      }

	// If the above loop finishes, then the vectors are equal.
      return true;
}

bool operator < (const vvp_vector2_t&a, const vvp_vector2_t&b)
{
      const unsigned awords = (a.wid_ + vvp_vector2_t::BITS_PER_WORD-1) / vvp_vector2_t::BITS_PER_WORD;
      const unsigned bwords = (b.wid_ + vvp_vector2_t::BITS_PER_WORD-1) / vvp_vector2_t::BITS_PER_WORD;

      unsigned words = awords;
      if (bwords > words)
	    words = bwords;

      for (unsigned idx = words ;  idx > 0 ;  idx -= 1) {
	    unsigned long aw = (idx <= awords)? a.vec_[idx-1] : 0;
	    unsigned long bw = (idx <= bwords)? b.vec_[idx-1] : 0;

	    if (aw < bw)
		  return true;
	    if (aw > bw)
		  return false;
      }

	// If the above loop finishes, then the vectors are equal.
      return false;
}

bool operator <= (const vvp_vector2_t&a, const vvp_vector2_t&b)
{
	// XXXX For now, only support equal width vectors.
      assert(a.wid_ == b.wid_);

      const unsigned awords = (a.wid_ + vvp_vector2_t::BITS_PER_WORD-1) / vvp_vector2_t::BITS_PER_WORD;

      for (unsigned idx = awords ;  idx > 0 ;  idx -= 1) {
	    if (a.vec_[idx-1] < b.vec_[idx-1])
		  return true;
	    if (a.vec_[idx-1] > b.vec_[idx-1])
		  return false;
      }

	// If the above loop finishes, then the vectors are equal.
      return true;
}

bool operator == (const vvp_vector2_t&a, const vvp_vector2_t&b)
{
      const unsigned awords = (a.wid_ + vvp_vector2_t::BITS_PER_WORD-1) / vvp_vector2_t::BITS_PER_WORD;
      const unsigned bwords = (b.wid_ + vvp_vector2_t::BITS_PER_WORD-1) / vvp_vector2_t::BITS_PER_WORD;

      const unsigned words = awords > bwords? awords : bwords;

      for (unsigned idx = words ;  idx > 0 ;  idx -= 1) {
	    unsigned long aw = (idx <= awords)? a.vec_[idx-1] : 0;
	    unsigned long bw = (idx <= bwords)? b.vec_[idx-1] : 0;

	    if (aw > bw)
		  return false;
	    if (aw < bw)
		  return false;
      }

	// If the above loop finishes, then the vectors are equal.
      return true;
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

vvp_vector4_t c4string_to_vector4(const char*str)
{
      assert((str[0]=='C') && (str[1]=='4') && (str[2]=='<'));

      str += 3;
      const char*tp = str + strspn(str,"01xz");
      assert(tp[0] == '>');

      vvp_vector4_t tmp (tp-str);

      for (unsigned idx = 0 ;  idx < tmp.size() ;  idx += 1) {
	    vvp_bit4_t bit;
	    switch (str[idx]) {
		case '0':
		  bit = BIT4_0;
		  break;
		case '1':
		  bit = BIT4_1;
		  break;
		case 'x':
		  bit = BIT4_X;
		  break;
		case 'z':
		  bit = BIT4_Z;
		  break;
		default:
		  fprintf(stderr, "Unsupported bit value %c(%d).\n", str[idx],
		          str[idx]);
		  assert(0);
		  bit = BIT4_0;
		  break;
	    }
	    tmp.set_bit(tmp.size()-idx-1, bit);
      }

      return tmp;
}

ostream& operator<< (ostream&out, const vvp_vector2_t&that)
{
      if (that.is_NaN()) {
	    out << "NaN";

      } else {
	    out << vector2_to_vector4(that, that.size());
      }
      return out;
}

vvp_vector8_t::vvp_vector8_t(const vvp_vector8_t&that)
{
      size_ = that.size_;
      if (size_ <= PTR_THRESH) {
	    memcpy(val_, that.val_, sizeof(val_));
      } else {
	    ptr_ = new vvp_scalar_t[size_];
	    for (unsigned idx = 0 ;  idx < size_ ;  idx += 1)
		  ptr_[idx] = that.ptr_[idx];
      }
}

vvp_vector8_t::vvp_vector8_t(const vvp_vector4_t&that,
			     unsigned str0, unsigned str1)
: size_(that.size())
{
      if (size_ == 0)
	    return;

      vvp_scalar_t*tmp;
      if (size_ <= PTR_THRESH)
	    tmp = new (val_) vvp_scalar_t[PTR_THRESH];
      else
	    tmp = ptr_ = new vvp_scalar_t[size_];

      for (unsigned idx = 0 ;  idx < size_ ;  idx += 1)
	    tmp[idx] = vvp_scalar_t (that.value(idx), str0, str1);

}

vvp_vector8_t& vvp_vector8_t::operator= (const vvp_vector8_t&that)
{
	// Assign to self.
      if (this == &that || (size_ > PTR_THRESH && that.size_ > PTR_THRESH &&
                            ptr_ == that.ptr_))
	    return *this;

      if (size_ != that.size_) {
	    if (size_ > PTR_THRESH)
		  delete[]ptr_;
	    size_ = 0;
      }

      if (that.size_ == 0) {
	    assert(size_ == 0);
	    return *this;
      }

      if (that.size_ <= PTR_THRESH) {
	    size_ = that.size_;
	    memcpy(val_, that.val_, sizeof(val_));
	    return *this;
      }

      if (size_ == 0) {
	    size_ = that.size_;
	    ptr_ = new vvp_scalar_t[size_];
      }

      for (unsigned idx = 0 ;  idx < size_ ;  idx += 1)
	    ptr_[idx] = that.ptr_[idx];

      return *this;
}

vvp_vector8_t vvp_vector8_t::subvalue(unsigned base, unsigned wid) const
{
      vvp_vector8_t tmp (wid);

      vvp_scalar_t* tmp_ptr = tmp.size_<=PTR_THRESH? reinterpret_cast<vvp_scalar_t*>(tmp.val_) : tmp.ptr_;
      const vvp_scalar_t* ptr = size_<=PTR_THRESH? reinterpret_cast<const vvp_scalar_t*>(val_) : ptr_;

      unsigned idx = 0;
      while ((idx < wid) && (base+idx < size_)) {
	    tmp_ptr[idx] = ptr[base+idx];
	    idx += 1;
      }

      return tmp;
}

vvp_vector8_t part_expand(const vvp_vector8_t&that, unsigned wid, unsigned off)
{
      assert(off < wid);
      vvp_vector8_t tmp (wid);

      vvp_scalar_t* tmp_ptr = tmp.size_<=vvp_vector8_t::PTR_THRESH? reinterpret_cast<vvp_scalar_t*>(tmp.val_) : tmp.ptr_;
      const vvp_scalar_t* that_ptr = that.size_<=vvp_vector8_t::PTR_THRESH? reinterpret_cast<const vvp_scalar_t*>(that.val_) : that.ptr_;

      unsigned idx = off;

      while (idx < wid && that.size_ > (idx-off)) {
	    tmp_ptr[idx] = that_ptr[idx-off];
	    idx += 1;
      }

      return tmp;
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
      count_functors += 1;
}

vvp_net_fun_t::~vvp_net_fun_t()
{
}

void vvp_net_fun_t::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                              vvp_context_t)
{
      fprintf(stderr, "internal error: %s: recv_vec4 not implemented\n",
	      typeid(*this).name());
      assert(0);
}

void vvp_net_fun_t::recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
				 unsigned base, unsigned wid, unsigned vwid,
                                 vvp_context_t)
{
      cerr << "internal error: " << typeid(*this).name() << ": "
	   << "recv_vec4_pv(" << bit << ", " << base
	   << ", " << wid << ", " << vwid << ") not implemented" << endl;
      assert(0);
}

void vvp_net_fun_t::recv_vec8(vvp_net_ptr_t port, const vvp_vector8_t&bit)
{
      recv_vec4(port, reduce4(bit), 0);
}

void vvp_net_fun_t::recv_vec8_pv(vvp_net_ptr_t port, const vvp_vector8_t&bit,
				 unsigned base, unsigned wid, unsigned vwid)
{
      recv_vec4_pv(port, reduce4(bit), base, wid, vwid, 0);
}

void vvp_net_fun_t::recv_real(vvp_net_ptr_t port, double bit, vvp_context_t)
{
      fprintf(stderr, "internal error: %s: recv_real(%f) not implemented\n",
	      typeid(*this).name(), bit);
      assert(0);
}

void vvp_net_fun_t::recv_long(vvp_net_ptr_t, long)
{
      fprintf(stderr, "internal error: %s: recv_long not implemented\n",
	      typeid(*this).name());
      assert(0);
}

void vvp_net_fun_t::recv_long_pv(vvp_net_ptr_t, long, unsigned, unsigned)
{
      fprintf(stderr, "internal error: %s: recv_long_pv not implemented\n",
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

void vvp_fun_drive::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                              vvp_context_t)
{
      assert(port.port() == 0);
      vvp_send_vec8(port.ptr()->out, vvp_vector8_t(bit, drive0_, drive1_));
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
	    vvp_send_vec4(ptr.ptr()->out, bits, 0);
      } else {
            vvp_send_vec4(ptr.ptr()->out, bits4_, 0);
      }

      run_vpi_callbacks();
}

void vvp_fun_signal4_sa::release(vvp_net_ptr_t ptr, bool net)
{
	// If not currently forced, do nothing.
      if (force_mask_.size() == 0)
            return;

      force_mask_ = vvp_vector2_t();
      if (net) {
	    vvp_send_vec4(ptr.ptr()->out, bits4_, 0);
	    run_vpi_callbacks();
      } else {
	    bits4_ = force_;
      }
}

void vvp_fun_signal4_sa::release_pv(vvp_net_ptr_t ptr, bool net,
                                    unsigned base, unsigned wid)
{
	// If not currently forced, do nothing.
      if (force_mask_.size() == 0)
            return;

      assert(bits4_.size() >= base + wid);

      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
            if (force_mask_.value(base+idx)) {
                  force_mask_.set_bit(base+idx, 0);
                  if (!net) bits4_.set_bit(base+idx, force_.value(base+idx));
            }
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
            vvp_send_vec4(ptr.ptr()->out, *bits4, context);
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
      vvp_send_vec4(ptr.ptr()->out, *bits4, context);
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
	    vvp_send_vec8(ptr.ptr()->out, bits);

      } else {
	    vvp_send_vec8(ptr.ptr()->out, bits8_);
      }

      run_vpi_callbacks();
}

void vvp_fun_signal8::release(vvp_net_ptr_t ptr, bool net)
{
	// If not currently forced, do nothing.
      if (force_mask_.size() == 0)
            return;

      force_mask_ = vvp_vector2_t();
      if (net) {
	    vvp_send_vec8(ptr.ptr()->out, bits8_);
	    run_vpi_callbacks();
      } else {
	    bits8_ = force_;
      }
}

void vvp_fun_signal8::release_pv(vvp_net_ptr_t ptr, bool net,
                                 unsigned base, unsigned wid)
{
	// If not currently forced, do nothing.
      if (force_mask_.size() == 0)
            return;

      assert(bits8_.size() >= base + wid);

      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
            if (force_mask_.value(base+idx)) {
                  force_mask_.set_bit(base+idx, 0);
                  if (!net) bits8_.set_bit(base+idx, force_.value(base+idx));
            }
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
			vvp_send_real(ptr.ptr()->out, bit, 0);
			run_vpi_callbacks();
		  }
	    }
	    break;

	  case 1: // Continuous assign value
	    continuous_assign_active_ = true;
	    bits_ = bit;
	    vvp_send_real(ptr.ptr()->out, bit, 0);
	    run_vpi_callbacks();
	    break;

	  case 2: // Force value
	    force_mask_ = vvp_vector2_t(1, 1);
	    force_ = bit;
	    vvp_send_real(ptr.ptr()->out, bit, 0);
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
	// If not currently forced, do nothing.
      if (force_mask_.size() == 0)
            return;

      force_mask_ = vvp_vector2_t();
      if (net) {
	    vvp_send_real(ptr.ptr()->out, bits_, 0);
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
            vvp_send_real(ptr.ptr()->out, bit, context);
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

/* **** vvp_wide_fun_* methods **** */

vvp_wide_fun_core::vvp_wide_fun_core(vvp_net_t*net, unsigned nports)
{
      ptr_ = net;
      nports_ = nports;
      port_values_ = 0;
      port_rvalues_ = 0;
}

vvp_wide_fun_core::~vvp_wide_fun_core()
{
      if (port_values_) delete[]port_values_;
      if (port_rvalues_) delete[]port_rvalues_;
}

void vvp_wide_fun_core::propagate_vec4(const vvp_vector4_t&bit,
				       vvp_time64_t delay)
{
      if (delay)
	    schedule_assign_plucked_vector(ptr_->out, delay, bit, 0, bit.size());
      else
	    vvp_send_vec4(ptr_->out, bit, 0);
}

void vvp_wide_fun_core::propagate_real(double bit,
				       vvp_time64_t delay)
{
      if (delay) {
	      // schedule_assign_vector(ptr_->out, bit, delay);
	    assert(0); // Need a real-value version of assign_vector.
      } else {
	    vvp_send_real(ptr_->out, bit, 0);
      }
}


unsigned vvp_wide_fun_core::port_count() const
{
      return nports_;
}

vvp_vector4_t& vvp_wide_fun_core::value(unsigned idx)
{
      assert(idx < nports_);
      assert(port_values_);
      return port_values_[idx];
}

double vvp_wide_fun_core::value_r(unsigned idx)
{
      assert(idx < nports_);
      return port_rvalues_? port_rvalues_[idx] : 0.0;
}

void vvp_wide_fun_core::recv_real_from_inputs(unsigned p)
{
      assert(0);
}

void vvp_wide_fun_core::dispatch_vec4_from_input_(unsigned port,
						   vvp_vector4_t bit)
{
      assert(port < nports_);
      if (port_values_ == 0) port_values_ = new vvp_vector4_t [nports_];
      port_values_[port] = bit;
      recv_vec4_from_inputs(port);
}

void vvp_wide_fun_core::dispatch_real_from_input_(unsigned port,
						  double bit)
{
      assert(port < nports_);
      if (port_rvalues_ == 0) port_rvalues_ = new double[nports_];
      port_rvalues_[port] = bit;
      recv_real_from_inputs(port);
}

vvp_wide_fun_t::vvp_wide_fun_t(vvp_wide_fun_core*c, unsigned base)
: core_(c), port_base_(base)
{
}

vvp_wide_fun_t::~vvp_wide_fun_t()
{
}

void vvp_wide_fun_t::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                               vvp_context_t)
{
      unsigned pidx = port_base_ + port.port();
      core_->dispatch_vec4_from_input_(pidx, bit);
}

void vvp_wide_fun_t::recv_real(vvp_net_ptr_t port, double bit,
                               vvp_context_t)
{
      unsigned pidx = port_base_ + port.port();
      core_->dispatch_real_from_input_(pidx, bit);
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

# define STREN1(v) (((v)&0x70) >> 4)
# define STREN0(v) ((v)&0x07)

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

/*
 * This function is only called if the actual interface function rules
 * out some of the easy cases. If we get here, we can assume that
 * neither of the values is HiZ, and the values are not exactly equal.
 */
vvp_scalar_t fully_featured_resolv_(vvp_scalar_t a, vvp_scalar_t b)
{

      if (UNAMBIG(a.value_) && UNAMBIG(b.value_)) {

	      /* If both signals are unambiguous, simply choose
		 the stronger. If they have the same strength
		 but different values, then this becomes
		 ambiguous. */

	    if ((b.value_&0x07) > (a.value_&0x07)) {

		    /* b value is stronger. Take it. */
		  return b;

	    } else if ((b.value_&0x77) == (a.value_&0x77)) {

		    // Strengths are the same. Since we know already
		    // that the values are not the same, Make value
		    // into "x".
		  vvp_scalar_t tmp (a);
		  tmp.value_ = (tmp.value_&0x77) | 0x80;
		  return tmp;

	    } else {

		    /* Must be "a" is the stronger one. */
		  return a;
	    }

      }

	/* If one of the signals is unambiguous, then it
	   will sweep up the weaker parts of the ambiguous
	   signal. The result may be ambiguous, or maybe not. */

      if (UNAMBIG(a.value_)) {
	    vvp_scalar_t res;

	    if ((a.value_&0x70) > (b.value_&0x70))
		  res.value_ |= a.value_&0xf0;
	    else
		  res.value_ |= b.value_&0xf0;

	    if ((a.value_&0x07) > (b.value_&0x07))
		  res.value_ |= a.value_&0x0f;
	    else
		  res.value_ |= b.value_&0x0f;

	    return res;

      } else if (UNAMBIG(b.value_)) {

	    vvp_scalar_t res;

	    if ((b.value_&0x70) > (a.value_&0x70))
		  res.value_ |= b.value_&0xf0;
	    else
		  res.value_ |= a.value_&0xf0;

	    if ((b.value_&0x07) > (a.value_&0x07))
		  res.value_ |= b.value_&0x0f;
	    else
		  res.value_ |= a.value_&0x0f;

	    return res;

      }


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
	      /* Set the MSB when both arguments MSBs are set. This
		 can only happen if both one strengths are zero. */
	    tmp |= (a.value_&b.value_)&0x80;
	    tmp |= (-sv1) << 4;
      }

      if (sv0 > 0) {
	    tmp |= 0x08;
	    tmp |= sv0;
      } else {
	    tmp |= (-sv0);
      }

      vvp_scalar_t res;
      res.value_ = tmp;

	/* Canonicalize the HiZ value. */
      if ((res.value_&0x77) == 0)
	    res.value_ = 0;

      return res;
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
      if (lef.has_xz())
	    return BIT4_X;

	// Check for X/Z in the right operand
      if (rig.has_xz())
	    return BIT4_X;

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

vvp_bit4_t compare_gtge_signed(const vvp_vector4_t&a,
			       const vvp_vector4_t&b,
			       vvp_bit4_t out_if_equal)
{
      assert(a.size() == b.size());

      unsigned sign_idx = a.size()-1;
      vvp_bit4_t a_sign = a.value(sign_idx);
      vvp_bit4_t b_sign = b.value(sign_idx);

      if (bit4_is_xz(a_sign))
	    return BIT4_X;
      if (bit4_is_xz(b_sign))
	    return BIT4_X;

      if (a_sign == b_sign)
	    return compare_gtge(a, b, out_if_equal);

      if (a.has_xz())
	    return BIT4_X;

      if (b.has_xz())
	    return BIT4_X;

      if(a_sign == BIT4_0)
	    return BIT4_1;
      else
	    return BIT4_0;
}
