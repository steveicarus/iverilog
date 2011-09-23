#ifndef __vvp_net_H
#define __vvp_net_H
/*
 * Copyright (c) 2004-2011 Stephen Williams (steve@icarus.com)
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
# include  "vpi_user.h"
# include  <cstddef>
# include  <cstdlib>
# include  <cstring>
# include  <new>
# include  <cassert>

#ifdef HAVE_IOSFWD
# include  <iosfwd>
#else
class ostream;
#endif

using namespace std;


/* Data types */
class  vvp_scalar_t;

/* Basic netlist types. */
struct vvp_net_t;
class  vvp_net_fun_t;

/* Core net function types. */
class  vvp_fun_concat;
class  vvp_fun_drive;
class  vvp_fun_part;

class  vvp_delay_t;

/*
 * Storage for items declared in automatically allocated scopes (i.e. automatic
 * tasks and functions). The first two slots in each context are reserved for
 * linking to other contexts. The function that adds items to a context knows
 * this, and allocates context indices accordingly.
 */
typedef void**vvp_context_t;

typedef void*vvp_context_item_t;

inline vvp_context_t vvp_allocate_context(unsigned nitem)
{
      return (vvp_context_t)malloc((2 + nitem) * sizeof(void*));
}

inline vvp_context_t vvp_get_next_context(vvp_context_t context)
{
      return (vvp_context_t)context[0];
}

inline void vvp_set_next_context(vvp_context_t context, vvp_context_t next)
{
      context[0] = next;
}

inline vvp_context_t vvp_get_stacked_context(vvp_context_t context)
{
      return (vvp_context_t)context[1];
}

inline void vvp_set_stacked_context(vvp_context_t context, vvp_context_t stack)
{
      context[1] = stack;
}

inline vvp_context_item_t vvp_get_context_item(vvp_context_t context,
                                               unsigned item_idx)
{
      return (vvp_context_item_t)context[item_idx];
}

inline void vvp_set_context_item(vvp_context_t context, unsigned item_idx,
                                 vvp_context_item_t item)
{
      context[item_idx] = item;
}

/*
 * An "automatic" functor is one which may be associated with an automatically
 * allocated scope item. This provides the infrastructure needed to allocate
 * the state information for individual instances of the item.
 */
struct automatic_hooks_s {

      automatic_hooks_s() {}
      virtual ~automatic_hooks_s() {}

      virtual void alloc_instance(vvp_context_t context) = 0;
      virtual void reset_instance(vvp_context_t context) = 0;
#ifdef CHECK_WITH_VALGRIND
      virtual void free_instance(vvp_context_t context) = 0;
#endif
};

/*
 * This is the set of Verilog 4-value bit values. Scalars have this
 * value along with strength, vectors are a collection of these
 * values. The enumeration has fixed numeric values that can be
 * expressed in 2 real bits, so that some of the internal classes can
 * pack them tightly.
 */
enum vvp_bit4_t {
      BIT4_0 = 0,
      BIT4_1 = 1,
      BIT4_X = 3,
      BIT4_Z = 2
};

  /* Return an ASCII character that represents the vvp_bit4_t value. */
inline char vvp_bit4_to_ascii(vvp_bit4_t a) { return "01zx"[a]; }

extern vvp_bit4_t add_with_carry(vvp_bit4_t a, vvp_bit4_t b, vvp_bit4_t&c);

extern vvp_bit4_t scalar_to_bit4(PLI_INT32 scalar);

  /* Return TRUE if the bit is BIT4_X or BIT4_Z. The fast
     implementation here relies on the encoding of vvp_bit4_t values. */
inline bool bit4_is_xz(vvp_bit4_t a) { return a >= 2; }

  /* This function converts BIT4_Z to BIT4_X, but passes other values
     unchanged. This fast implementation relies of the encoding of the
     vvp_bit4_t values. In particular, the BIT4_X==3 and BIT4_Z==2 */
inline vvp_bit4_t bit4_z2x(vvp_bit4_t a)
{ return (vvp_bit4_t) ( (int)a | ((int)a >> 1) ); }

  /* Some common boolean operators. These implement the Verilog rules
     for 4-value bit operations. The fast implementations here rely
     on the encoding of vvp_bit4_t values. */

  // ~BIT4_0 --> BIT4_1
  // ~BIT4_1 --> BIT4_0
  // ~BIT4_X --> BIT4_X
  // ~BIT4_Z --> BIT4_X
inline vvp_bit4_t operator ~ (vvp_bit4_t a)
{ return bit4_z2x((vvp_bit4_t) (((int)a) ^ 1)); }

inline vvp_bit4_t operator | (vvp_bit4_t a, vvp_bit4_t b)
{
      if (a==BIT4_1 || b==BIT4_1)
	    return BIT4_1;
      return bit4_z2x( (vvp_bit4_t) ((int)a | (int)b) );
}

inline vvp_bit4_t operator & (vvp_bit4_t a, vvp_bit4_t b)
{
      if (a==BIT4_0 || b==BIT4_0)
	    return BIT4_0;
      return bit4_z2x( (vvp_bit4_t) ((int)a | (int)b) );
}


extern vvp_bit4_t operator ^ (vvp_bit4_t a, vvp_bit4_t b);
extern ostream& operator<< (ostream&o, vvp_bit4_t a);

  /* Return >0, ==0 or <0 if the from-to transition represents a
     posedge, no edge, or negedge. */
extern int edge(vvp_bit4_t from, vvp_bit4_t to);

/*
 * This class represents scalar values collected into vectors. The
 * vector values can be accessed individually, or treated as a
 * unit. in any case, the elements of the vector are addressed from
 * zero(LSB) to size-1(MSB).
 *
 * No strength values are stored here, if strengths are needed, use a
 * collection of vvp_scalar_t objects instead.
 */
class vvp_vector4_t {

      friend vvp_vector4_t operator ~(const vvp_vector4_t&that);
      friend class vvp_vector4array_t;
      friend class vvp_vector4array_sa;
      friend class vvp_vector4array_aa;

    public:
      explicit vvp_vector4_t(unsigned size =0, vvp_bit4_t bits =BIT4_X);

      explicit vvp_vector4_t(unsigned size, double val);

	// Construct a vector4 from the subvalue of another vector4.
      explicit vvp_vector4_t(const vvp_vector4_t&that,
			     unsigned adr, unsigned wid);

      vvp_vector4_t(const vvp_vector4_t&that);
      vvp_vector4_t& operator= (const vvp_vector4_t&that);

      ~vvp_vector4_t();

      unsigned size() const { return size_; }
      void resize(unsigned new_size);

	// Get the bit at the specified address
      vvp_bit4_t value(unsigned idx) const;
	// Get the vector4 subvector starting at the address
      vvp_vector4_t subvalue(unsigned idx, unsigned size) const;
	// Get the 2-value bits for the subvector. This returns a new
	// array of longs, or a nil pointer if an XZ bit was detected
	// in the array.
      unsigned long*subarray(unsigned idx, unsigned size) const;
      void setarray(unsigned idx, unsigned size, const unsigned long*val);

      void set_bit(unsigned idx, vvp_bit4_t val);
      void set_vec(unsigned idx, const vvp_vector4_t&that);

        // Get the bits from another vector, but keep my size.
      void copy_bits(const vvp_vector4_t&that);

	// Test that the vectors are exactly equal
      bool eeq(const vvp_vector4_t&that) const;

	// Return true if there is an X or Z anywhere in the vector.
      bool has_xz() const;

	// Change all Z bits to X bits.
      void change_z2x();

	// Change all bits to X bits.
      void set_to_x();

	// Display the value into the buf as a string.
      char*as_string(char*buf, size_t buf_len);

      void invert();
      vvp_vector4_t& operator &= (const vvp_vector4_t&that);
      vvp_vector4_t& operator |= (const vvp_vector4_t&that);
      vvp_vector4_t& operator += (int64_t);

    private:
	// Number of vvp_bit4_t bits that can be shoved into a word.
      enum { BITS_PER_WORD = 8*sizeof(unsigned long) };
	// The double value constructor requires that WORD_0_BBITS
	// and WORD_1_BBITS have the same value!
#if SIZEOF_UNSIGNED_LONG == 8
      enum { WORD_0_ABITS = 0x0000000000000000UL,
	     WORD_0_BBITS = 0x0000000000000000UL };
      enum { WORD_1_ABITS = 0xFFFFFFFFFFFFFFFFUL,
	     WORD_1_BBITS = 0x0000000000000000UL };
      enum { WORD_X_ABITS = 0xFFFFFFFFFFFFFFFFUL,
	     WORD_X_BBITS = 0xFFFFFFFFFFFFFFFFUL };
      enum { WORD_Z_ABITS = 0x0000000000000000UL,
             WORD_Z_BBITS = 0xFFFFFFFFFFFFFFFFUL };
#elif SIZEOF_UNSIGNED_LONG == 4
      enum { WORD_0_ABITS = 0x00000000UL, WORD_0_BBITS = 0x00000000UL };
      enum { WORD_1_ABITS = 0xFFFFFFFFUL, WORD_1_BBITS = 0x00000000UL };
      enum { WORD_X_ABITS = 0xFFFFFFFFUL, WORD_X_BBITS = 0xFFFFFFFFUL };
      enum { WORD_Z_ABITS = 0x00000000UL, WORD_Z_BBITS = 0xFFFFFFFFUL };
#else
#error "WORD_X_xBITS not defined for this architecture?"
#endif

	// Initialize and operator= use this private method to copy
	// the data from that object into this object.
      void copy_from_(const vvp_vector4_t&that);

      void allocate_words_(unsigned size, unsigned long inita, unsigned long initb);

	// Values in the vvp_vector4_t are stored split across two
	// arrays. For each bit in the vector, there is an abit and a
	// bbit. the encoding of a vvp_vector4_t is:
	//
	//         abit bbit
	//         ---- ----
	// BIT4_0    0    0   (Note that for BIT4_0 and BIT4_1, the bbit
	// BIT4_1    1    0    value is 0. This makes detecting XZ fast.)
	// BIT4_X    1    1
	// BIT4_Z    0    1

      unsigned size_;
      union {
	    unsigned long abits_val_;
	    unsigned long*abits_ptr_;
      };
      union {
	    unsigned long bbits_val_;
	    unsigned long*bbits_ptr_;
      };
};

inline vvp_vector4_t::vvp_vector4_t(const vvp_vector4_t&that)
{
      copy_from_(that);
}

inline vvp_vector4_t::vvp_vector4_t(unsigned size__, vvp_bit4_t val)
: size_(size__)
{
	/* note: this relies on the bit encoding for the vvp_bit4_t. */
      const static unsigned long init_atable[4] = {
	    WORD_0_ABITS,
	    WORD_1_ABITS,
	    WORD_Z_ABITS,
	    WORD_X_ABITS };
      const static unsigned long init_btable[4] = {
	    WORD_0_BBITS,
	    WORD_1_BBITS,
	    WORD_Z_BBITS,
	    WORD_X_BBITS };

      allocate_words_(size_, init_atable[val], init_btable[val]);
}

inline vvp_vector4_t::~vvp_vector4_t()
{
      if (size_ > BITS_PER_WORD) {
	    delete[] abits_ptr_;
	      // bbits_ptr_ actually points half-way into a
	      // double-length array started at abits_ptr_
      }
}

inline vvp_vector4_t& vvp_vector4_t::operator= (const vvp_vector4_t&that)
{
      if (this == &that)
	    return *this;

      if (size_ > BITS_PER_WORD)
	    delete[] abits_ptr_;

      copy_from_(that);

      return *this;
}


inline vvp_bit4_t vvp_vector4_t::value(unsigned idx) const
{
      if (idx >= size_)
	    return BIT4_X;

      unsigned wdx = idx / BITS_PER_WORD;
      unsigned long off = idx % BITS_PER_WORD;

      unsigned long abits, bbits;
      if (size_ > BITS_PER_WORD) {
	    abits = abits_ptr_[wdx];
	    bbits = bbits_ptr_[wdx];
      } else {
	    abits = abits_val_;
	    bbits = bbits_val_;
      }

      abits >>= off;
      bbits >>= off;
      int tmp = ((bbits&1) << 1) + (abits&1);
      static const vvp_bit4_t bits_bit4_map[4] = {
	    BIT4_0, // bbit==0, abit==0
	    BIT4_1, // bbit==0, abit==1
	    BIT4_Z, // bbit==1, abit==0
	    BIT4_X  // bbit==1, abit==1
      };

	/* Casting is evil, but this cast matches the un-cast done
	   when the vvp_bit4_t value is put into the vector. */
      return bits_bit4_map[tmp];
}

inline vvp_vector4_t vvp_vector4_t::subvalue(unsigned adr, unsigned wid) const
{
      return vvp_vector4_t(*this, adr, wid);
}

inline void vvp_vector4_t::set_bit(unsigned idx, vvp_bit4_t val)
{
      assert(idx < size_);

      unsigned long off = idx % BITS_PER_WORD;
      unsigned long mask = 1UL << off;

      if (size_ > BITS_PER_WORD) {
	    unsigned wdx = idx / BITS_PER_WORD;
	    switch (val) {
		case BIT4_0:
		  abits_ptr_[wdx] &= ~mask;
		  bbits_ptr_[wdx] &= ~mask;
		  break;
		case BIT4_1:
		  abits_ptr_[wdx] |=  mask;
		  bbits_ptr_[wdx] &= ~mask;
		  break;
		case BIT4_X:
		  abits_ptr_[wdx] |=  mask;
		  bbits_ptr_[wdx] |=  mask;
		  break;
		case BIT4_Z:
		  abits_ptr_[wdx] &= ~mask;
		  bbits_ptr_[wdx] |=  mask;
		  break;
	    }
      } else {
	    switch (val) {
		case BIT4_0:
		  abits_val_ &= ~mask;
		  bbits_val_ &= ~mask;
		  break;
		case BIT4_1:
		  abits_val_ |=  mask;
		  bbits_val_ &= ~mask;
		  break;
		case BIT4_X:
		  abits_val_ |=  mask;
		  bbits_val_ |=  mask;
		  break;
		case BIT4_Z:
		  abits_val_ &= ~mask;
		  bbits_val_ |=  mask;
		  break;
	    }
      }
}

inline vvp_vector4_t operator ~ (const vvp_vector4_t&that)
{
      vvp_vector4_t res = that;
      res.invert();
      return res;
}

extern ostream& operator << (ostream&, const vvp_vector4_t&);

extern vvp_bit4_t compare_gtge(const vvp_vector4_t&a,
			       const vvp_vector4_t&b,
			       vvp_bit4_t val_if_equal);
extern vvp_bit4_t compare_gtge_signed(const vvp_vector4_t&a,
				      const vvp_vector4_t&b,
				      vvp_bit4_t val_if_equal);
template <class T> extern T coerce_to_width(const T&that, unsigned width);

/*
 * These functions extract the value of the vector as a native type,
 * if possible, and return true to indicate success. If the vector has
 * any X or Z bits, the resulting value will have 0 bits in their
 * place (this follows the rules of Verilog conversions from vector4
 * to real and integers) and the return value becomes false to
 * indicate an error.
 *
 * The "is_arithmetic" flag true will cause a result to be entirely 0
 * if any bits are X/Z. That is normally what you want if this value
 * is in the midst of an arithmetic expression. If is_arithmetic=false
 * then the X/Z bits will be replaced with 0 bits, and the return
 * value will be "false", but the other bits will be transferred. This
 * is what you want if you are doing "vpi_get_value", for example.
 */
extern bool vector4_to_value(const vvp_vector4_t&a, long&val, bool is_signed, bool is_arithmetic =true);
extern bool vector4_to_value(const vvp_vector4_t&a, unsigned long&val);
#ifndef UL_AND_TIME64_SAME
extern bool vector4_to_value(const vvp_vector4_t&a, int64_t&val, bool is_signed, bool is_arithmetic =true);
extern bool vector4_to_value(const vvp_vector4_t&a, vvp_time64_t&val);
#endif
extern bool vector4_to_value(const vvp_vector4_t&a, double&val, bool is_signed);

/*
 * The __vpiArray handle uses instances of this to keep an array of
 * real valued variables.
 */
class vvp_realarray_t {

    public:
      vvp_realarray_t(unsigned words);
      ~vvp_realarray_t();

      unsigned words() const { return words_; }

      double get_word(unsigned idx) const;
      void set_word(unsigned idx, double val);

    private:
      unsigned words_;
      double*array_;
};

/*
 * vvp_vector4array_t
 */
class vvp_vector4array_t {

    public:
      vvp_vector4array_t(unsigned width, unsigned words);
      virtual ~vvp_vector4array_t();

      unsigned width() const { return width_; }
      unsigned words() const { return words_; }

      virtual vvp_vector4_t get_word(unsigned idx) const = 0;
      virtual void set_word(unsigned idx, const vvp_vector4_t&that) = 0;

    protected:
      struct v4cell {
	    union {
		  unsigned long abits_val_;
		  unsigned long*abits_ptr_;
	    };
	    union {
		  unsigned long bbits_val_;
		  unsigned long*bbits_ptr_;
	    };
      };

      vvp_vector4_t get_word_(v4cell*cell) const;
      void set_word_(v4cell*cell, const vvp_vector4_t&that);

      unsigned width_;
      unsigned words_;

    private: // Not implemented
      vvp_vector4array_t(const vvp_vector4array_t&);
      vvp_vector4array_t& operator = (const vvp_vector4array_t&);
};

/*
 * Statically allocated vvp_vector4array_t
 */
class vvp_vector4array_sa : public vvp_vector4array_t {

    public:
      vvp_vector4array_sa(unsigned width, unsigned words);
      ~vvp_vector4array_sa();

      vvp_vector4_t get_word(unsigned idx) const;
      void set_word(unsigned idx, const vvp_vector4_t&that);

    private:
      v4cell* array_;
};

/*
 * Automatically allocated vvp_vector4array_t
 */
class vvp_vector4array_aa : public vvp_vector4array_t, public automatic_hooks_s {

    public:
      vvp_vector4array_aa(unsigned width, unsigned words);
      ~vvp_vector4array_aa();

      void alloc_instance(vvp_context_t context);
      void reset_instance(vvp_context_t context);
#ifdef CHECK_WITH_VALGRIND
      void free_instance(vvp_context_t context);
#endif

      vvp_vector4_t get_word(unsigned idx) const;
      void set_word(unsigned idx, const vvp_vector4_t&that);

    private:
      unsigned context_idx_;
};

/* vvp_vector2_t
 */
class vvp_vector2_t {

      friend vvp_vector2_t operator - (const vvp_vector2_t&);
      friend vvp_vector2_t operator + (const vvp_vector2_t&,
				       const vvp_vector2_t&);
      friend vvp_vector2_t operator * (const vvp_vector2_t&,
				       const vvp_vector2_t&);
      friend bool operator >  (const vvp_vector2_t&, const vvp_vector2_t&);
      friend bool operator >= (const vvp_vector2_t&, const vvp_vector2_t&);
      friend bool operator <  (const vvp_vector2_t&, const vvp_vector2_t&);
      friend bool operator <= (const vvp_vector2_t&, const vvp_vector2_t&);
      friend bool operator == (const vvp_vector2_t&, const vvp_vector2_t&);

    public:
      vvp_vector2_t();
      vvp_vector2_t(const vvp_vector2_t&);
      vvp_vector2_t(const vvp_vector2_t&, unsigned newsize);
	// Make a vvp_vector2_t from a vvp_vector4_t. If there are X
	// or Z bits, then the result becomes a NaN value.
      explicit vvp_vector2_t(const vvp_vector4_t&that);
	// Make from a native long and a specified width.
      vvp_vector2_t(unsigned long val, unsigned wid);
	// Make with the width, and filled with 1 or 0 bits.
      enum fill_t {FILL0, FILL1};
      vvp_vector2_t(fill_t fill, unsigned wid);
      ~vvp_vector2_t();

      vvp_vector2_t&operator += (const vvp_vector2_t&that);
      vvp_vector2_t&operator -= (const vvp_vector2_t&that);
      vvp_vector2_t&operator <<= (unsigned shift);
      vvp_vector2_t&operator >>= (unsigned shift);
      vvp_vector2_t&operator = (const vvp_vector2_t&);

      bool is_NaN() const;
      bool is_zero() const;
      unsigned size() const;
      int value(unsigned idx) const;
      void set_bit(unsigned idx, int bit);
	// Make the size just big enough to hold the first 1 bit.
      void trim();
	// Trim off extra 1 bit since this is representing a negative value.
	// Always keep at least 32 bits.
      void trim_neg();

    private:
      enum { BITS_PER_WORD = 8 * sizeof(unsigned long) };
      unsigned long*vec_;
      unsigned wid_;

    private:
      void copy_from_that_(const vvp_vector2_t&that);
};

extern bool operator >  (const vvp_vector2_t&, const vvp_vector2_t&);
extern bool operator >= (const vvp_vector2_t&, const vvp_vector2_t&);
extern bool operator <  (const vvp_vector2_t&, const vvp_vector2_t&);
extern bool operator <= (const vvp_vector2_t&, const vvp_vector2_t&);
extern bool operator == (const vvp_vector2_t&, const vvp_vector2_t&);
extern vvp_vector2_t operator - (const vvp_vector2_t&);
extern vvp_vector2_t operator + (const vvp_vector2_t&, const vvp_vector2_t&);
extern vvp_vector2_t operator * (const vvp_vector2_t&, const vvp_vector2_t&);
extern vvp_vector2_t operator / (const vvp_vector2_t&, const vvp_vector2_t&);
extern vvp_vector2_t operator % (const vvp_vector2_t&, const vvp_vector2_t&);

vvp_vector2_t pow(const vvp_vector2_t&, vvp_vector2_t&);
extern vvp_vector4_t vector2_to_vector4(const vvp_vector2_t&, unsigned wid);
/* A c4string is of the form C4<...> where ... are bits. */
extern vvp_vector4_t c4string_to_vector4(const char*str);

extern ostream& operator<< (ostream&, const vvp_vector2_t&);

/* Inline some of the vector2_t methods. */
inline unsigned vvp_vector2_t::size() const
{
      return wid_;
}

/*
 * This class represents a scalar value with strength. These are
 * heavier then the simple vvp_bit4_t, but more information is
 * carried by that weight.
 *
 * The strength values are as defined here:
 *   HiZ    - 0
 *   Small  - 1
 *   Medium - 2
 *   Weak   - 3
 *   Large  - 4
 *   Pull   - 5
 *   Strong - 6
 *   Supply - 7
 *
 * There are two strengths for a value: strength0 and strength1. If
 * the value is Z, then strength0 is the strength of the 0-value, and
 * strength of the 1-value. If the value is 0 or 1, then the strengths
 * are the range for that value.
 */
class vvp_scalar_t {

      friend vvp_scalar_t fully_featured_resolv_(vvp_scalar_t a, vvp_scalar_t b);

    public:
	// Make a HiZ value.
      explicit vvp_scalar_t();

	// Make an unambiguous value.
      explicit vvp_scalar_t(vvp_bit4_t val, unsigned str0, unsigned str1);

	// Get the vvp_bit4_t version of the value
      vvp_bit4_t value() const;
      unsigned strength0() const;
      unsigned strength1() const;

      bool eeq(vvp_scalar_t that) const { return value_ == that.value_; }
      bool is_hiz() const { return value_ == 0; }

    private:
      unsigned char value_;
};

inline vvp_scalar_t::vvp_scalar_t()
{
      value_ = 0;
}

inline vvp_scalar_t::vvp_scalar_t(vvp_bit4_t val, unsigned str0, unsigned str1)
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

inline vvp_bit4_t vvp_scalar_t::value() const
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


inline vvp_scalar_t resolve(vvp_scalar_t a, vvp_scalar_t b)
{
      extern vvp_scalar_t fully_featured_resolv_(vvp_scalar_t, vvp_scalar_t);

	// If the value is HiZ, resolution is simply a matter of
	// returning the *other* value.
      if (a.is_hiz())
	    return b;
      if (b.is_hiz())
	    return a;
	// If the values are the identical, then resolution is simply
	// returning *either* value.
      if (a .eeq( b ))
	    return a;

      return fully_featured_resolv_(a,b);
}

extern ostream& operator<< (ostream&, vvp_scalar_t);

/*
 * This class is a way to carry vectors of strength modeled
 * values. The 8 in the name is the number of possible distinct values
 * a well defined bit may have. When you add in ambiguous values, the
 * number of distinct values span the vvp_scalar_t.
 *
 * A vvp_vector8_t object can be created from a vvp_vector4_t and a
 * strength value. The vvp_vector8_t bits have the values of the input
 * vector, all with the strength specified. If no strength is
 * specified, then the conversion from bit4 to a scalar will use the
 * Verilog convention default of strong (6).
 */
class vvp_vector8_t {

      friend vvp_vector8_t part_expand(const vvp_vector8_t&, unsigned, unsigned);

    public:
      explicit vvp_vector8_t(unsigned size =0);
	// Make a vvp_vector8_t from a vector4 and a specified
	// strength.
      explicit vvp_vector8_t(const vvp_vector4_t&that,
			     unsigned str0,
			     unsigned str1);

      ~vvp_vector8_t();

      unsigned size() const { return size_; }
      vvp_scalar_t value(unsigned idx) const;
      vvp_vector8_t subvalue(unsigned adr, unsigned width) const;
      void set_bit(unsigned idx, vvp_scalar_t val);

	// Test that the vectors are exactly equal
      bool eeq(const vvp_vector8_t&that) const;

      vvp_vector8_t(const vvp_vector8_t&that);
      vvp_vector8_t& operator= (const vvp_vector8_t&that);

    private:
	// This is the number of vvp_scalar_t objects we can keep in
	// the val_ buffer. If the vector8 is bigger then this, then
	// resort to allocations to get a larger buffer.
      enum { PTR_THRESH = 8 };
      unsigned size_;
      union {
	    vvp_scalar_t*ptr_;
	    char val_[PTR_THRESH * sizeof(vvp_scalar_t)];
      };
};

  /* Resolve uses the default Verilog resolver algorithm to resolve
     two drive vectors to a single output. */
inline vvp_vector8_t resolve(const vvp_vector8_t&a, const vvp_vector8_t&b)
{
      assert(a.size() == b.size());
      vvp_vector8_t out (a.size());

      for (unsigned idx = 0 ;  idx < out.size() ;  idx += 1) {
	    out.set_bit(idx, resolve(a.value(idx), b.value(idx)));
      }

      return out;
}

  /* This function implements the strength reduction implied by
     Verilog standard resistive devices. */
extern vvp_vector8_t resistive_reduction(const vvp_vector8_t&a);
  /* The reduce4 function converts a vector8 to a vector4, losing
     strength information in the process. */
extern vvp_vector4_t reduce4(const vvp_vector8_t&that);
extern vvp_vector8_t part_expand(const vvp_vector8_t&a, unsigned wid, unsigned off);
  /* Print a vector8 value to a stream. */
extern ostream& operator<< (ostream&, const vvp_vector8_t&);

inline vvp_vector8_t::vvp_vector8_t(unsigned size__)
: size_(size__)
{
      if (size_ <= PTR_THRESH) {
	    new (reinterpret_cast<void*>(val_)) vvp_scalar_t[PTR_THRESH];
      } else {
	    ptr_ = new vvp_scalar_t[size_];
      }
}

inline vvp_vector8_t::~vvp_vector8_t()
{
      if (size_ > PTR_THRESH)
	    delete[]ptr_;
}

inline vvp_scalar_t vvp_vector8_t::value(unsigned idx) const
{
      assert(idx < size_);
      if (size_ <= PTR_THRESH)
	    return reinterpret_cast<const vvp_scalar_t*>(val_) [idx];
      else
	    return ptr_[idx];
}

inline void vvp_vector8_t::set_bit(unsigned idx, vvp_scalar_t val)
{
      assert(idx < size_);
      if (size_ <= PTR_THRESH)
	    reinterpret_cast<vvp_scalar_t*>(val_) [idx] = val;
      else
	    ptr_[idx] = val;
}

  // Exactly-equal for vvp_vector8_t is common and should be as tight
  // as possible.
inline bool vvp_vector8_t::eeq(const vvp_vector8_t&that) const
{
      if (size_ != that.size_)
	    return false;
      if (size_ == 0)
	    return true;

      if (size_ <= PTR_THRESH)
	    return 0 == memcmp(val_, that.val_, sizeof(val_));

      for (unsigned idx = 0 ;  idx < size_ ;  idx += 1) {
	    if (! ptr_[idx] .eeq( that.ptr_[idx] ))
		return false;
      }

      return true;
}

/*
 * This class implements a pointer that points to an item within a
 * target. The ptr() method returns a pointer to the vvp_net_t, and
 * the port() method returns a 0-3 value that selects the input within
 * the vvp_net_t. Use this pointer to point only to the inputs of
 * vvp_net_t objects. To point to vvp_net_t objects as a whole, use
 * vvp_net_t* pointers.
 *
 * Alert! Ugly details. Protective clothing recommended!
 * The vvp_net_ptr_t encodes the bits of a C pointer, and two bits of
 * port identifier into an unsigned long. This works only if vvp_net_t*
 * values are always aligned on 4-byte boundaries.
 */
template <class T> class vvp_sub_pointer_t {

    public:
      vvp_sub_pointer_t() : bits_(0) { }

      vvp_sub_pointer_t(T*ptr__, unsigned port__)
      {
	    bits_ = reinterpret_cast<unsigned long> (ptr__);
	    assert( (bits_  &  3) == 0 );
	    assert( (port__ & ~3) == 0 );
	    bits_ |= port__;
      }

      ~vvp_sub_pointer_t() { }

      T* ptr()
      { return reinterpret_cast<T*> (bits_ & ~3UL); }

      const T* ptr() const
      { return reinterpret_cast<const T*> (bits_ & ~3UL); }

      unsigned  port() const { return bits_ & 3; }

      bool nil() const { return bits_ == 0; }

      bool operator == (vvp_sub_pointer_t that) const { return bits_ == that.bits_; }
      bool operator != (vvp_sub_pointer_t that) const { return bits_ != that.bits_; }

    private:
      unsigned long bits_;
};

typedef vvp_sub_pointer_t<vvp_net_t> vvp_net_ptr_t;
template <class T> ostream& operator << (ostream&out, vvp_sub_pointer_t<T> val)
{ out << val.ptr() << "[" << val.port() << "]"; return out; }

/*
 * This is the basic unit of netlist connectivity. It is a fan-in of
 * up to 4 inputs, and output pointer, and a pointer to the node's
 * functionality.
 *
 * A net output that is non-nil points to the input of one of its
 * destination nets. If there are multiple destinations, then the
 * referenced input port points to the next input. For example:
 *
 *   +--+--+--+--+---+
 *   |  |  |  |  | . |  Output from this vvp_net_t points to...
 *   +--+--+--+--+-|-+
 *                 |
 *                /
 *               /
 *              /
 *             |
 *             v
 *   +--+--+--+--+---+
 *   |  |  |  |  | . |  ... the fourth input of this vvp_net_t, and...
 *   +--+--+--+--+-|-+
 *             |   |
 *            /    .
 *           /     .
 *          |      .
 *          v
 *   +--+--+--+--+---+
 *   |  |  |  |  | . |  ... the third input of this vvp_net_t.
 *   +--+--+--+--+-|-+
 *
 * Thus, the fan-in of a vvp_net_t node is limited to 4 inputs, but
 * the fan-out is unlimited.
 *
 * The vvp_send_*() functions take as input a vvp_net_ptr_t and follow
 * all the fan-out chain, delivering the specified value.
 */
struct vvp_net_t {
#ifdef CHECK_WITH_VALGRIND
      vvp_net_t *pool;
#endif
      vvp_net_ptr_t port[4];
      vvp_net_ptr_t out;
      vvp_net_fun_t*fun;

    public: // Need a better new for these objects.
      static void* operator new(std::size_t size);
      static void operator delete(void*); // not implemented
    private: // not implemented
      static void* operator new[](std::size_t size);
      static void operator delete[](void*);
};

/*
 * Instances of this class represent the functionality of a
 * node. vvp_net_t objects hold pointers to the vvp_net_fun_t
 * associated with it. Objects of this type take inputs that arrive at
 * a port and perform some sort of action in response.
 *
 * Whenever a bit is delivered to a vvp_net_t object, the associated
 * vvp_net_fun_t::recv_*() method is invoked with the port pointer and
 * the bit value. The port pointer is used to figure out which exact
 * input receives the bit.
 *
 * In this context, a "bit" is the thing that arrives at a single
 * input. The bit may be a single data bit, a bit vector, various
 * sorts of numbers or aggregate objects.
 *
 * recv_vec4 is the most common way for a datum to arrive at a
 * port. The value is a vvp_vector4_t.
 *
 * Most nodes do not care about the specific strengths of bits, so the
 * default behavior for recv_vec8 and recv_vec8_pv is to reduce the
 * operand to a vvp_vector4_t and pass it on to the recv_vec4 or
 * recv_vec4_pv method.
 *
 * The recv_vec4, recv_vec4_pv, and recv_real methods are also
 * passed a context pointer. When the received bit has propagated
 * from a statically allocated node, this will be a null pointer.
 * When the received bit has propagated from an automatically
 * allocated node, this will be a pointer to the context that
 * contains the instance of that bit that has just been modified.
 * When the received bit was from a procedural assignment or from
 * a VPI set_value() operation, this will be a pointer to the
 * writable context associated with the currently running thread.
 */
class vvp_net_fun_t {

    public:
      vvp_net_fun_t();
      virtual ~vvp_net_fun_t();

      virtual void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                             vvp_context_t context);
      virtual void recv_vec8(vvp_net_ptr_t port, const vvp_vector8_t&bit);
      virtual void recv_real(vvp_net_ptr_t port, double bit,
                             vvp_context_t context);
      virtual void recv_long(vvp_net_ptr_t port, long bit);

	// Part select variants of above
      virtual void recv_vec4_pv(vvp_net_ptr_t p, const vvp_vector4_t&bit,
				unsigned base, unsigned wid, unsigned vwid,
                                vvp_context_t context);
      virtual void recv_vec8_pv(vvp_net_ptr_t p, const vvp_vector8_t&bit,
				unsigned base, unsigned wid, unsigned vwid);
      virtual void recv_long_pv(vvp_net_ptr_t port, long bit,
                                unsigned base, unsigned wid);

    public: // These objects are only permallocated.
      static void* operator new(std::size_t size);
      static void operator delete(void*); // not implemented
    private: // not implemented
      vvp_net_fun_t(const vvp_net_fun_t&);
      vvp_net_fun_t& operator= (const vvp_net_fun_t&);
      static void* operator new[](std::size_t size);
      static void operator delete[](void*);
};

/* **** Some core net functions **** */

/* vvp_fun_concat
 * This node function creates vectors (vvp_vector4_t) from the
 * concatenation of the inputs. The inputs (4) may be vector or
 * vector8 objects, but they are reduced to vector4 values and
 * strength information lost.
 *
 * The expected widths of the input vectors must be given up front so
 * that the positions in the output vector (and also the size of the
 * output vector) can be worked out. The input vectors must match the
 * expected width.
 */
class vvp_fun_concat  : public vvp_net_fun_t {

    public:
      vvp_fun_concat(unsigned w0, unsigned w1,
		     unsigned w2, unsigned w3);
      ~vvp_fun_concat();

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t context);

    private:
      unsigned wid_[4];
      vvp_vector4_t val_;
};

/* vvp_fun_repeat
 * This node function create vectors by repeating the input. The width
 * is the width of the output vector, and the repeat is the number of
 * times to repeat the input. The width of the input vector is
 * implicit from these values.
 */
class vvp_fun_repeat  : public vvp_net_fun_t {

    public:
      vvp_fun_repeat(unsigned width, unsigned repeat);
      ~vvp_fun_repeat();

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t context);

    private:
      unsigned wid_;
      unsigned rep_;
};

/* vvp_fun_drive
 * This node function takes an input vvp_vector4_t as input, and
 * repeats that value as a vvp_vector8_t with all the bits given the
 * strength of the drive. This is the way vvp_scalar8_t objects are
 * created. Input 0 is the value to be driven (vvp_vector4_t) and
 * inputs 1 and two are the strength0 and strength1 values to use. The
 * strengths may be taken as constant values, or adjusted as longs
 * from the network.
 *
 * This functor only propagates vvp_vector8_t values.
 */
class vvp_fun_drive  : public vvp_net_fun_t {

    public:
      vvp_fun_drive(vvp_bit4_t init, unsigned str0 =6, unsigned str1 =6);
      ~vvp_fun_drive();

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t context);
	//void recv_long(vvp_net_ptr_t port, long bit);

    private:
      unsigned char drive0_;
      unsigned char drive1_;
};

/*
 * EXTEND functors expand an input vector to the desired output
 * width. The extend_signed functor sign extends the input. If the
 * input is already wider then the desired output, then it is passed
 * unmodified.
 */
class vvp_fun_extend_signed  : public vvp_net_fun_t {

    public:
      explicit vvp_fun_extend_signed(unsigned wid);
      ~vvp_fun_extend_signed();

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t context);

    private:
      unsigned width_;
};

/* vvp_fun_signal
 * This node is the place holder in a vvp network for signals,
 * including nets of various sort. The output from a signal follows
 * the type of its port-0 input. If vvp_vector4_t values come in
 * through port-0, then vvp_vector4_t values are propagated. If
 * vvp_vector8_t values come in through port-0, then vvp_vector8_t
 * values are propagated. Thus, this node is slightly polymorphic.
 *
 * If the signal is a net (i.e. a wire or tri) then this node will
 * have an input that is the data source. The data source will connect
 * through port-0.
 *
 * If the signal is a reg, then there will be no netlist input, the
 * values will be written by behavioral statements. The %set and
 * %assign statements will write through port-0.
 *
 * In any case, behavioral code is able to read the value that this
 * node last propagated, by using the value() method. That is important
 * functionality of this node.
 *
 * Continuous assignments are made through port-1. When a value is
 * written here, continuous assign mode is activated, and input
 * through port-0 is ignored until continuous assign mode is turned
 * off again. Writing into this port can be done in behavioral code
 * using the %cassign/v instruction, or can be done by the network by
 * hooking the output of a vvp_net_t to this port.
 *
 * Force assignments are made through port-2. When a value is written
 * here, force mode is activated. In force mode, port-0 data (or
 * port-1 data if in continuous assign mode) is tracked but not
 * propagated. The force value is propagated and is what is readable
 * through the value method.
 *
 * Port-3 is a command port, intended for use by procedural
 * instructions. The client must write long values to this port to
 * invoke the command of interest. The command values are:
 *
 *  1  -- deassign
 *          The deassign command takes the node out of continuous
 *          assignment mode. The output value is unchanged, and force
 *          mode, if active, remains in effect.
 *
 *  2  -- release/net
 *          The release/net command takes the node out of force mode,
 *          and propagates the tracked port-0 value to the signal
 *          output. This acts like a release of a net signal.
 *
 *  3  -- release/reg
 *          The release/reg command is similar to the release/net
 *          command, but the port-0 value is not propagated. Changes
 *          to port-0 (or port-1 if continuous assign is active) will
 *          propagate starting at the next input change.
 */

/*
* Things derived from vvp_vpi_callback may also be array'ed, so it
* includes some members that arrays use.
*/
class vvp_vpi_callback {

    public:
      vvp_vpi_callback();
      virtual ~vvp_vpi_callback();

      virtual void run_vpi_callbacks();
      void add_vpi_callback(struct __vpiCallback*);
#ifdef CHECK_WITH_VALGRIND
	/* This has only been tested at EOS. */
      void clear_all_callbacks(void);
#endif

      virtual void get_value(struct t_vpi_value*value) =0;

    private:
      struct __vpiCallback*vpi_callbacks_;
};

class vvp_vpi_callback_wordable : public vvp_vpi_callback {
    public:
      vvp_vpi_callback_wordable();
      ~vvp_vpi_callback_wordable();

      void run_vpi_callbacks();
      void attach_as_word(struct __vpiArray* arr, unsigned long addr);

    private:
      struct __vpiArray* array_;
      unsigned long array_word_;
};

class vvp_fun_signal_base : public vvp_net_fun_t, public vvp_vpi_callback_wordable {

    public:
      vvp_fun_signal_base();
      void recv_long(vvp_net_ptr_t port, long bit);
      void recv_long_pv(vvp_net_ptr_t port, long bit,
                        unsigned base, unsigned wid);

    public:

	/* The %force/link instruction needs a place to write the
	   source node of the force, so that subsequent %force and
	   %release instructions can undo the link as needed. */
      struct vvp_net_t*force_link;
      struct vvp_net_t*cassign_link;

    protected:

	// This is true until at least one propagation happens.
      bool needs_init_;
      bool continuous_assign_active_;
      vvp_vector2_t force_mask_;
      vvp_vector2_t assign_mask_;

      void deassign();
      void deassign_pv(unsigned base, unsigned wid);
      virtual void release(vvp_net_ptr_t ptr, bool net) =0;
      virtual void release_pv(vvp_net_ptr_t ptr, bool net,
                              unsigned base, unsigned wid) =0;
};

/*
 * This abstract class is a little more specific than the signal_base
 * class, in that it adds vector access methods.
 */
class vvp_fun_signal_vec : public vvp_fun_signal_base {

    public:
	// For vector signal types, this returns the vector count.
      virtual unsigned size() const =0;
      virtual vvp_bit4_t value(unsigned idx) const =0;
      virtual vvp_scalar_t scalar_value(unsigned idx) const =0;
      virtual vvp_vector4_t vec4_value() const =0;
};

class vvp_fun_signal4 : public vvp_fun_signal_vec {

    public:
      explicit vvp_fun_signal4() {};

      void get_value(struct t_vpi_value*value);

};

/*
 * Statically allocated vvp_fun_signal4.
 */
class vvp_fun_signal4_sa : public vvp_fun_signal4 {

    public:
      explicit vvp_fun_signal4_sa(unsigned wid, vvp_bit4_t init=BIT4_X);

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t);
      void recv_vec8(vvp_net_ptr_t port, const vvp_vector8_t&bit);

	// Part select variants of above
      void recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
			unsigned base, unsigned wid, unsigned vwid,
                        vvp_context_t);
      void recv_vec8_pv(vvp_net_ptr_t port, const vvp_vector8_t&bit,
			unsigned base, unsigned wid, unsigned vwid);

	// Get information about the vector value.
      unsigned   size() const;
      vvp_bit4_t value(unsigned idx) const;
      vvp_scalar_t scalar_value(unsigned idx) const;
      vvp_vector4_t vec4_value() const;

	// Commands
      void release(vvp_net_ptr_t port, bool net);
      void release_pv(vvp_net_ptr_t port, bool net,
                      unsigned base, unsigned wid);

    private:
      void calculate_output_(vvp_net_ptr_t ptr);

      vvp_vector4_t bits4_;
      vvp_vector4_t force_;
};

/*
 * Automatically allocated vvp_fun_signal4.
 */
class vvp_fun_signal4_aa : public vvp_fun_signal4, public automatic_hooks_s {

    public:
      explicit vvp_fun_signal4_aa(unsigned wid, vvp_bit4_t init=BIT4_X);

      void alloc_instance(vvp_context_t context);
      void reset_instance(vvp_context_t context);
#ifdef CHECK_WITH_VALGRIND
      void free_instance(vvp_context_t context);
#endif

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t context);

	// Part select variants of above
      void recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
			unsigned base, unsigned wid, unsigned vwid,
                        vvp_context_t);

	// Get information about the vector value.
      unsigned   size() const;
      vvp_bit4_t value(unsigned idx) const;
      vvp_scalar_t scalar_value(unsigned idx) const;
      vvp_vector4_t vec4_value() const;

	// Commands
      void release(vvp_net_ptr_t port, bool net);
      void release_pv(vvp_net_ptr_t port, bool net,
                      unsigned base, unsigned wid);

    private:
      unsigned context_idx_;
      unsigned size_;
};

class vvp_fun_signal8  : public vvp_fun_signal_vec {

    public:
      explicit vvp_fun_signal8(unsigned wid);

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t context);
      void recv_vec8(vvp_net_ptr_t port, const vvp_vector8_t&bit);

	// Part select variants of above
      void recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                        unsigned base, unsigned wid, unsigned vwid,
                        vvp_context_t context);
      void recv_vec8_pv(vvp_net_ptr_t port, const vvp_vector8_t&bit,
                        unsigned base, unsigned wid, unsigned vwid);

	// Get information about the vector value.
      unsigned   size() const;
      vvp_bit4_t value(unsigned idx) const;
      vvp_scalar_t scalar_value(unsigned idx) const;
      vvp_vector4_t vec4_value() const;

	// Commands
      void release(vvp_net_ptr_t port, bool net);
      void release_pv(vvp_net_ptr_t port, bool net,
                      unsigned base, unsigned wid);

      void get_value(struct t_vpi_value*value);

    private:
      void calculate_output_(vvp_net_ptr_t ptr);

      vvp_vector8_t bits8_;
      vvp_vector8_t force_;
};

class vvp_fun_signal_real : public vvp_fun_signal_base {

    public:
      explicit vvp_fun_signal_real() {};

	// Get information about the vector value.
      virtual double real_value() const = 0;

      void get_value(struct t_vpi_value*value);
};

/*
 * Statically allocated vvp_fun_signal_real.
 */
class vvp_fun_signal_real_sa : public vvp_fun_signal_real {

    public:
      explicit vvp_fun_signal_real_sa();

      void recv_real(vvp_net_ptr_t port, double bit,
                     vvp_context_t);

	// Get information about the vector value.
      double real_value() const;

	// Commands
      void release(vvp_net_ptr_t port, bool net);
      void release_pv(vvp_net_ptr_t port, bool net,
                      unsigned base, unsigned wid);

    private:
      double bits_;
      double force_;
};

/*
 * Automatically allocated vvp_fun_signal_real.
 */
class vvp_fun_signal_real_aa : public vvp_fun_signal_real, public automatic_hooks_s {

    public:
      explicit vvp_fun_signal_real_aa();

      void alloc_instance(vvp_context_t context);
      void reset_instance(vvp_context_t context);
#ifdef CHECK_WITH_VALGRIND
      void free_instance(vvp_context_t context);
#endif

      void recv_real(vvp_net_ptr_t port, double bit,
                     vvp_context_t context);

	// Get information about the vector value.
      double real_value() const;

	// Commands
      void release(vvp_net_ptr_t port, bool net);
      void release_pv(vvp_net_ptr_t port, bool net,
                      unsigned base, unsigned wid);

    private:
      unsigned context_idx_;
};

/*
 * Wide Functors:
 * Wide functors represent special devices that may have more than 4
 * input ports. These devices need a set of N/4 actual functors to
 * catch the inputs, and use another to deliver the output.
 *
 *  vvp_wide_fun_t --+--> vvp_wide_fun_core --> ...
 *                   |
 *  vvp_wide_fun_t --+
 *                   |
 *  vvp_wide_fun_t --+
 *
 * There are enough input functors to take all the functor inputs, 4
 * per functor. These inputs deliver the changed input value to the
 * wide_fun_core, which carries the infrastructure for the thread. The
 * wide_fun_core is also a functor whose output is connected to the rest
 * of the netlist. This is where the result is delivered back to the
 * netlist.
 *
 * The vvp_wide_fun_core keeps a store of the inputs from all the
 * input functors, and makes them available to the derived class that
 * does the processing.
 */

class vvp_wide_fun_core : public vvp_net_fun_t {

    public:
      vvp_wide_fun_core(vvp_net_t*net, unsigned nports);
      virtual ~vvp_wide_fun_core();
	// These objects are not perm allocated.
      void* operator new(std::size_t size) { return ::new char[size]; }
      void operator delete(void* ptr) { ::delete[]((char*)ptr); }

    protected:
      void propagate_vec4(const vvp_vector4_t&bit, vvp_time64_t delay =0);
      void propagate_real(double bit, vvp_time64_t delay =0);
      unsigned port_count() const;

      vvp_vector4_t& value(unsigned);
      double value_r(unsigned);

    private:
	// the derived class implements this to receive an indication
	// that one of the port input values changed.
      virtual void recv_vec4_from_inputs(unsigned port) =0;
      virtual void recv_real_from_inputs(unsigned port);

      friend class vvp_wide_fun_t;
      void dispatch_vec4_from_input_(unsigned port, vvp_vector4_t bit);
      void dispatch_real_from_input_(unsigned port, double bit);

    private:
	// Back-point to the vvp_net_t that points to me.
      vvp_net_t*ptr_;
	// Structure to track the input values from the input functors.
      unsigned nports_;
      vvp_vector4_t*port_values_;
      double*port_rvalues_;
};

/*
 * The job of the input functor is only to monitor inputs to the
 * function and pass them to the ufunc_core object. Each functor takes
 * up to 4 inputs, with the base the port number for the first
 * function input that this functor represents.
 */
class vvp_wide_fun_t : public vvp_net_fun_t {

    public:
      vvp_wide_fun_t(vvp_wide_fun_core*c, unsigned base);
      ~vvp_wide_fun_t();

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t context);
      void recv_real(vvp_net_ptr_t port, double bit,
                     vvp_context_t context);

    private:
      vvp_wide_fun_core*core_;
      unsigned port_base_;
};


inline void vvp_send_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&val,
                          vvp_context_t context)
{
      while (struct vvp_net_t*cur = ptr.ptr()) {
	    vvp_net_ptr_t next = cur->port[ptr.port()];

	    if (cur->fun)
		  cur->fun->recv_vec4(ptr, val, context);

	    ptr = next;
      }
}

extern void vvp_send_vec8(vvp_net_ptr_t ptr, const vvp_vector8_t&val);
extern void vvp_send_real(vvp_net_ptr_t ptr, double val,
                          vvp_context_t context);
extern void vvp_send_long(vvp_net_ptr_t ptr, long val);
extern void vvp_send_long_pv(vvp_net_ptr_t ptr, long val,
                             unsigned base, unsigned width);

/*
 * Part-vector versions of above functions. This function uses the
 * corresponding recv_vec4_pv method in the vvp_net_fun_t functor to
 * deliver parts of a vector.
 *
 * The ptr is the destination input port to write to.
 *
 * <val> is the vector to be written. The width of this vector must
 * exactly match the <wid> vector.
 *
 * The <base> is where in the receiver the bit vector is to be
 * written. This address is given in canonical units; 0 is the LSB, 1
 * is the next bit, and so on.
 *
 * The <vwid> is the width of the destination vector that this part is
 * part of. This is used by intermediate nodes, i.e. resolvers, to
 * know how wide to pad with Z, if it needs to transform the part to a
 * mirror of the destination vector.
 */
inline void vvp_send_vec4_pv(vvp_net_ptr_t ptr, const vvp_vector4_t&val,
		             unsigned base, unsigned wid, unsigned vwid,
                             vvp_context_t context)
{
      while (struct vvp_net_t*cur = ptr.ptr()) {
	    vvp_net_ptr_t next = cur->port[ptr.port()];

	    if (cur->fun)
		  cur->fun->recv_vec4_pv(ptr, val, base, wid, vwid, context);

	    ptr = next;
      }
}

inline void vvp_send_vec8_pv(vvp_net_ptr_t ptr, const vvp_vector8_t&val,
		             unsigned base, unsigned wid, unsigned vwid)
{
      while (struct vvp_net_t*cur = ptr.ptr()) {
	    vvp_net_ptr_t next = cur->port[ptr.port()];

	    if (cur->fun)
		  cur->fun->recv_vec8_pv(ptr, val, base, wid, vwid);

	    ptr = next;
      }
}

#endif
