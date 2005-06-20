#ifndef __vvp_net_H
#define __vvp_net_H
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
#ident "$Id: vvp_net.h,v 1.36 2005/06/20 01:28:14 steve Exp $"

# include  "config.h"
# include  <stddef.h>
# include  <assert.h>

#ifdef HAVE_IOSFWD
# include  <iosfwd>
#else
class ostream;
#endif



/* Data types */
class  vvp_scalar_t;

/* Basic netlist types. */
class  vvp_net_t;
class  vvp_net_ptr_t;
class  vvp_net_fun_t;

/* Core net function types. */
class  vvp_fun_concat;
class  vvp_fun_drive;
class  vvp_fun_part;

class  vvp_delay_t;

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
      BIT4_X = 2,
      BIT4_Z = 3
};

extern vvp_bit4_t add_with_carry(vvp_bit4_t a, vvp_bit4_t b, vvp_bit4_t&c);
  /* Return TRUE if the bit is BIT4_X or BIT4_Z */
extern bool bit4_is_xz(vvp_bit4_t a);
  /* Some common boolean operators. These implement the Verilog rules
     for 4-value bit operations. */
extern vvp_bit4_t operator ~ (vvp_bit4_t a);
extern vvp_bit4_t operator & (vvp_bit4_t a, vvp_bit4_t b);
extern vvp_bit4_t operator | (vvp_bit4_t a, vvp_bit4_t b);
extern vvp_bit4_t operator ^ (vvp_bit4_t a, vvp_bit4_t b);
extern ostream& operator<< (ostream&o, vvp_bit4_t a);

/*
 * This class represents scaler values collected into vectors. The
 * vector values can be accessed individually, or treated as a
 * unit. in any case, the elements of the vector are addressed from
 * zero(LSB) to size-1(MSB).
 *
 * No strength values are stored here, if strengths are needed, use a
 * collection of vvp_scalar_t objects instead.
 */
class vvp_vector4_t {

    public:
      explicit vvp_vector4_t(unsigned size =0);

      vvp_vector4_t(const vvp_vector4_t&that);
      vvp_vector4_t& operator= (const vvp_vector4_t&that);

      ~vvp_vector4_t();

      unsigned size() const { return size_; }
      vvp_bit4_t value(unsigned idx) const;
      void set_bit(unsigned idx, vvp_bit4_t val);

	// Test that the vectors are exactly equal
      bool eeq(const vvp_vector4_t&that) const;

	// Display the value into the buf as a string.
      char*as_string(char*buf, size_t buf_len);

    private:
	// Number of vvp_bit4_t bits that can be shoved into a word.
      enum { BITS_PER_WORD = 8*sizeof(unsigned long)/2 };

	// Initialize and operator= use this private method to copy
	// the data from that object into this object.
      void copy_from_(const vvp_vector4_t&that);

      unsigned size_;
      union {
	    unsigned long bits_val_;
	    unsigned long*bits_ptr_;
      };
};

inline vvp_vector4_t::vvp_vector4_t(const vvp_vector4_t&that)
{
      copy_from_(that);
}

inline vvp_vector4_t::~vvp_vector4_t()
{
      if (size_ > BITS_PER_WORD) {
	    delete[] bits_ptr_;
      }
}

inline vvp_vector4_t& vvp_vector4_t::operator= (const vvp_vector4_t&that)
{
      if (this == &that)
	    return *this;

      if (size_ > BITS_PER_WORD)
	    delete[] bits_ptr_;

      copy_from_(that);

      return *this;
}


inline vvp_bit4_t vvp_vector4_t::value(unsigned idx) const
{
      if (idx >= size_)
	    return BIT4_X;

      unsigned wdx = idx / BITS_PER_WORD;
      unsigned off = idx % BITS_PER_WORD;

      unsigned long bits;
      if (size_ > BITS_PER_WORD) {
	    bits = bits_ptr_[wdx];
      } else {
	    bits = bits_val_;
      }

      bits >>= (off * 2);

	/* Casting is evil, but this cast matches the un-cast done
	   when the vvp_bit4_t value is put into the vector. */
      return (vvp_bit4_t) (bits & 3);
}

inline void vvp_vector4_t::set_bit(unsigned idx, vvp_bit4_t val)
{
      assert(idx < size_);

      unsigned wdx = idx / BITS_PER_WORD;
      unsigned off = idx % BITS_PER_WORD;
      unsigned long mask = 3UL << (2*off);

      if (size_ > BITS_PER_WORD) {
	    bits_ptr_[wdx] &= ~mask;
	    bits_ptr_[wdx] |= val << (2*off);
      } else {
	    bits_val_ &= ~mask;
	    bits_val_ |= val << (2*off);
      }
}


extern vvp_vector4_t operator ~ (const vvp_vector4_t&that);
extern ostream& operator << (ostream&, const vvp_vector4_t&);

extern vvp_bit4_t compare_gtge(const vvp_vector4_t&a,
			       const vvp_vector4_t&b,
			       vvp_bit4_t val_if_equal);
extern vvp_bit4_t compare_gtge_signed(const vvp_vector4_t&a,
				      const vvp_vector4_t&b,
				      vvp_bit4_t val_if_equal);
extern vvp_vector4_t coerce_to_width(const vvp_vector4_t&that, unsigned width);

/*
 * These functions extract the value of the vector as a native type,
 * if possible, and return true to indicate success. If the vector has
 * any X or Z bits, the resulting value will be unchanged and the
 * return value becomes false to indicate an error.
 */
extern bool vector4_to_value(const vvp_vector4_t&a, unsigned long&val);

/* vvp_vector2_t
 */
class vvp_vector2_t {

      friend vvp_vector2_t operator + (const vvp_vector2_t&,
				       const vvp_vector2_t&);
      friend vvp_vector2_t operator * (const vvp_vector2_t&,
				       const vvp_vector2_t&);

    public:
      vvp_vector2_t();
      vvp_vector2_t(const vvp_vector2_t&);
      explicit vvp_vector2_t(const vvp_vector4_t&that);
      vvp_vector2_t(unsigned long val, unsigned wid);
      ~vvp_vector2_t();

      vvp_vector2_t operator = (const vvp_vector2_t&);

      bool is_NaN() const;
      unsigned size() const;
      int value(unsigned idx) const;

    private:
      unsigned long*vec_;
      unsigned wid_;
};

extern vvp_vector2_t operator + (const vvp_vector2_t&, const vvp_vector2_t&);
extern vvp_vector2_t operator * (const vvp_vector2_t&, const vvp_vector2_t&);
extern vvp_vector4_t vector2_to_vector4(const vvp_vector2_t&, unsigned wid);

/*
 * This class represents a scaler value with strength. These are
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

      friend vvp_scalar_t resolve(vvp_scalar_t a, vvp_scalar_t b);

    public:
	// Make a HiZ value.
      explicit vvp_scalar_t();

	// Make an unambiguous value.
      explicit vvp_scalar_t(vvp_bit4_t val, unsigned str);
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

extern vvp_scalar_t resolve(vvp_scalar_t a, vvp_scalar_t b);
extern ostream& operator<< (ostream&, vvp_scalar_t);

/*
 * This class is a way to carry vectors of strength modeled
 * values. The 8 in the name is the number of possible distinct values
 * a well defined bit may have. When you add in ambiguous values, the
 * number of distinct values span the vvp_scalar_t.
 *
 * a vvp_vector8_t object can be created from a vvp_vector4_t and a
 * strength value. The vvp_vector8_t bits have the values of the input
 * vector, all with the strength specified. If no strength is
 * specified, then the conversion from bit4 to a scalar will use the
 * Verilog convention default of strong (6).
 */
class vvp_vector8_t {

    public:
      explicit vvp_vector8_t(unsigned size =0);
	// Make a vvp_vector8_t from a vector4 and a specified strength.
      vvp_vector8_t(const vvp_vector4_t&that, unsigned str =6);
      explicit vvp_vector8_t(const vvp_vector4_t&that,
			     unsigned str0,
			     unsigned str1);

      ~vvp_vector8_t();

      unsigned size() const { return size_; }
      vvp_scalar_t value(unsigned idx) const;
      void set_bit(unsigned idx, vvp_scalar_t val);

	// Test that the vectors are exactly equal
      bool eeq(const vvp_vector8_t&that) const;

      vvp_vector8_t(const vvp_vector8_t&that);
      vvp_vector8_t& operator= (const vvp_vector8_t&that);

    private:
      unsigned size_;
      vvp_scalar_t*bits_;
};

  /* Resolve uses the default Verilog resolver algorithm to resolve
     two drive vectors to a single output. */
extern vvp_vector8_t resolve(const vvp_vector8_t&a, const vvp_vector8_t&b);
  /* This function implements the strength reduction implied by
     Verilog standard resistive devices. */
extern vvp_vector8_t resistive_reduction(const vvp_vector8_t&a);
  /* The reduce4 function converts a vector8 to a vector4, losing
     strength information in the process. */
extern vvp_vector4_t reduce4(const vvp_vector8_t&that);
  /* Print a vector8 value to a stream. */
extern ostream& operator<< (ostream&, const vvp_vector8_t&);

/*
 * This class implements a pointer that points to an item within a
 * target. The ptr() method returns a pointer to the vvp_net_t, and
 * the port() method returns a 0-3 value that selects the input within
 * the vvp_net_t. Use this pointer to point only to the inputs of
 * vvp_net_t objects. To point to vvp_net_t objects as a whole, use
 * vvp_net_t* pointers.
 */
class vvp_net_ptr_t {

    public:
      vvp_net_ptr_t();
      vvp_net_ptr_t(vvp_net_t*ptr, unsigned port);
      ~vvp_net_ptr_t() { }

      vvp_net_t* ptr();
      const vvp_net_t* ptr() const;
      unsigned  port() const;

      bool nil() const;

    private:
      unsigned long bits_;
};

/*
 * Alert! Ugly details. Protective clothing recommended!
 * The vvp_net_ptr_t encodes the bits of a C pointer, and two bits of
 * port identifer into an unsigned long. This works only if vvp_net_t*
 * values are always aligned on 4byte boundaries.
 */

inline vvp_net_ptr_t::vvp_net_ptr_t()
{
      bits_ = 0;
}

inline vvp_net_ptr_t::vvp_net_ptr_t(vvp_net_t*ptr, unsigned port)
{
      bits_ = reinterpret_cast<unsigned long> (ptr);
      assert( (bits_ & 3) == 0 );
      assert( (port & ~3) == 0 );
      bits_ |= port;
}

inline vvp_net_t* vvp_net_ptr_t::ptr()
{
      return reinterpret_cast<vvp_net_t*> (bits_ & ~3UL);
}

inline const vvp_net_t* vvp_net_ptr_t::ptr() const
{
      return reinterpret_cast<const vvp_net_t*> (bits_ & ~3UL);
}

inline unsigned vvp_net_ptr_t::port() const
{
      return bits_ & 3;
}

inline bool vvp_net_ptr_t::nil() const
{
      return bits_ == 0;
}


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
      vvp_net_ptr_t port[4];
      vvp_net_ptr_t out;
      vvp_net_fun_t*fun;
      long fun_flags;
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
 * default behavior for recv_vec8 is to reduce the operand to a
 * vvp_vector4_t and pass it on to the recv_vec4 method.
 */
class vvp_net_fun_t {

    public:
      vvp_net_fun_t();
      virtual ~vvp_net_fun_t();

      virtual void recv_vec4(vvp_net_ptr_t port, vvp_vector4_t bit);
      virtual void recv_vec8(vvp_net_ptr_t port, vvp_vector8_t bit);
      virtual void recv_real(vvp_net_ptr_t port, double bit);
      virtual void recv_long(vvp_net_ptr_t port, long bit);

	// Part select variants of above
      virtual void recv_vec4_pv(vvp_net_ptr_t p, vvp_vector4_t bit,
				unsigned base, unsigned wid, unsigned vwid);

    private: // not implemented
      vvp_net_fun_t(const vvp_net_fun_t&);
      vvp_net_fun_t& operator= (const vvp_net_fun_t&);
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

      void recv_vec4(vvp_net_ptr_t port, vvp_vector4_t bit);

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

      void recv_vec4(vvp_net_ptr_t port, vvp_vector4_t bit);

    private:
      unsigned wid_;
      unsigned rep_;
};

/* vvp_fun_drive
 * This node function takes an input vvp_vector4_t as input, and
 * repeats that value as a vvp_vector8_t with all the bits given the
 * strength of the drive. This is the way vvp_scaler8_t objects are
 * created. Input 0 is the value to be drived (vvp_vector4_t) and
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

      void recv_vec4(vvp_net_ptr_t port, vvp_vector4_t bit);
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

      void recv_vec4(vvp_net_ptr_t port, vvp_vector4_t bit);

    private:
      unsigned width_;
};

/* vvp_fun_part
 * This node takes a part select of the input vector. Input 0 is the
 * vector to be selected from, and input 1 is the location where the
 * select starts. Input 2, which is typically constant, is the width
 * of the result.
 */
class vvp_fun_part  : public vvp_net_fun_t {

    public:
      vvp_fun_part(unsigned base, unsigned wid);
      ~vvp_fun_part();

    public:
      void recv_vec4(vvp_net_ptr_t port, vvp_vector4_t bit);

    private:
      unsigned base_;
      unsigned wid_;
};

/* vvp_fun_part_pv
 * This node takes a vector input and turns it into the part select of
 * a wider output network. It used the recv_vec4_pv methods of the
 * destination nodes to propagate the part select.
 */
class vvp_fun_part_pv  : public vvp_net_fun_t {

    public:
      vvp_fun_part_pv(unsigned base, unsigned wid, unsigned vec_wid);
      ~vvp_fun_part_pv();

    public:
      void recv_vec4(vvp_net_ptr_t port, vvp_vector4_t bit);

    private:
      unsigned base_;
      unsigned wid_;
      unsigned vwid_;
};

/*
 * This part select is more flexible in that it takes the vector to
 * part in port 0, and the base of the part in port 1. The width of
 * the part to take out is fixed.
 */
class vvp_fun_part_var  : public vvp_net_fun_t {

    public:
      explicit vvp_fun_part_var(unsigned wid);
      ~vvp_fun_part_var();

    public:
      void recv_vec4(vvp_net_ptr_t port, vvp_vector4_t bit);

    private:
      unsigned base_;
      unsigned wid_;
      vvp_vector4_t source_;
	// Save the last output, for detecting change.
      vvp_vector4_t ref_;
};

/* vvp_fun_signal
 * This node is the place holder in a vvp network for signals,
 * including nets of various sort. The output from a signal follows
 * the type of its port-0 input. If vvp_vector4_t values come in
 * through port-0, then vvp_vector4_t values are propagated. If
 * vvp_vector8_t values come in through port-0, then vvp_vector8_t
 * values are propaged. Thus, this node is sloghtly polymorphic.
 *
 * If the signal is a net (i.e. a wire or tri) then this node will
 * have an input that is the data source. The data source will connect
 * through port-0
 *
 * If the signal is a reg, then there will be no netlist input, the
 * values will be written by behavioral statements. The %set and
 * %assign statements will write through port-0
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
 *          to port-0 (or port-1 if continuous assing is active) will
 *          propagate starting at the next input change.
 */
class vvp_fun_signal  : public vvp_net_fun_t {

    public:
      explicit vvp_fun_signal(unsigned wid);

      void recv_vec4(vvp_net_ptr_t port, vvp_vector4_t bit);
      void recv_vec8(vvp_net_ptr_t port, vvp_vector8_t bit);
      void recv_long(vvp_net_ptr_t port, long bit);

	// Part select variants of above
      void recv_vec4_pv(vvp_net_ptr_t port, vvp_vector4_t bit,
			unsigned base, unsigned wid, unsigned vwid);

	// Get information about the vector value.
      unsigned   size() const;
      vvp_bit4_t value(unsigned idx) const;
      vvp_scalar_t scalar_value(unsigned idx) const;
      vvp_vector4_t vec4_value() const;

	// Commands
      void deassign();
      void release(vvp_net_ptr_t port, bool net);

    public:
      struct __vpiCallback*vpi_callbacks;

	/* The %force/link instruction needs a place to write the
	   source node of the force, so that subsequent %force and
	   %release instructions can undo the link as needed. */
      struct vvp_net_t*force_link;

    private:
      vvp_vector4_t bits4_;
      vvp_vector8_t bits8_;
      bool type_is_vector8_() const { return bits8_.size() > 0; }

	// This is true until at least one propagation happens.
      bool needs_init_;

      bool continuous_assign_active_;

      vvp_vector4_t force_;
      bool force_active_;

    private:
      void run_vpi_callbacks();
};

/*
 * Wide Functors:
 * Wide functors represent special devices that may have more then 4
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
 * wide_fun_core, which carries the infastructure for the thread. The
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

    protected:
      void propagate_vec4(const vvp_vector4_t&bit, vvp_time64_t delay =0);
      unsigned port_count() const;
      vvp_vector4_t& value(unsigned);

    private:
	// the derived class implements this to receive an indication
	// that one of the port input values changed.
      virtual void recv_vec4_from_inputs(unsigned port) =0;

      friend class vvp_wide_fun_t;
      void dispatch_vec4_from_input_(unsigned port, vvp_vector4_t bit);

    private:
	// Back-point to the vvp_net_t that points to me.
      vvp_net_t*ptr_;
	// Structure to track the input values from the input functors.
      unsigned nports_;
      vvp_vector4_t*port_values_;

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

      void recv_vec4(vvp_net_ptr_t port, vvp_vector4_t bit);

    private:
      vvp_wide_fun_core*core_;
      unsigned port_base_;
};


inline void vvp_send_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&val)
{
      while (struct vvp_net_t*cur = ptr.ptr()) {
	    vvp_net_ptr_t next = cur->port[ptr.port()];

	    if (cur->fun)
		  cur->fun->recv_vec4(ptr, val);

	    ptr = next;
      }
}
extern void vvp_send_vec8(vvp_net_ptr_t ptr, vvp_vector8_t val);
extern void vvp_send_real(vvp_net_ptr_t ptr, double val);
extern void vvp_send_long(vvp_net_ptr_t ptr, long val);

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
 * written. This address is given in cannonical units; 0 is the LSB, 1
 * is the next bit, and so on.
 *
 * The <vwid> is the width of the destination vector that this part is
 * part of. This is used by intermediate nodes, i.e. resolvers, to
 * know how wide to pad with Z, if it needs to transform the part to a
 * mirror of the destination vector.
 */
extern void vvp_send_vec4_pv(vvp_net_ptr_t ptr, vvp_vector4_t val,
			     unsigned base, unsigned wid, unsigned vwid);

/*
 * $Log: vvp_net.h,v $
 * Revision 1.36  2005/06/20 01:28:14  steve
 *  Inline some commonly called vvp_vector4_t methods.
 *
 * Revision 1.35  2005/06/19 18:42:00  steve
 *  Optimize the LOAD_VEC implementation.
 *
 * Revision 1.34  2005/06/15 00:47:15  steve
 *  Resolv do not propogate inputs that do not change.
 *
 * Revision 1.33  2005/06/14 00:42:06  steve
 *  Accomodate fussy compilers.
 *
 * Revision 1.32  2005/06/13 00:54:04  steve
 *  More unified vec4 to hex string functions.
 *
 * Revision 1.31  2005/06/12 15:13:37  steve
 *  Support resistive mos devices.
 *
 * Revision 1.30  2005/06/12 00:44:49  steve
 *  Implement nmos and pmos devices.
 *
 * Revision 1.29  2005/06/02 16:02:11  steve
 *  Add support for notif0/1 gates.
 *  Make delay nodes support inertial delay.
 *  Add the %force/link instruction.
 *
 * Revision 1.28  2005/05/24 01:43:27  steve
 *  Add a sign-extension node.
 *
 * Revision 1.27  2005/05/09 00:36:58  steve
 *  Force part base out of bounds if index is invalid.
 *
 * Revision 1.26  2005/05/08 23:40:14  steve
 *  Add support for variable part select.
 *
 * Revision 1.25  2005/05/07 03:14:50  steve
 *  ostream insert for vvp_vector4_t objects.
 *
 * Revision 1.24  2005/04/25 04:42:17  steve
 *  vvp_fun_signal eliminates duplicate propagations.
 *
 * Revision 1.23  2005/04/13 06:34:20  steve
 *  Add vvp driver functor for logic outputs,
 *  Add ostream output operators for debugging.
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
 * Revision 1.16  2005/02/13 05:26:30  steve
 *  tri0 and tri1 resolvers must replace HiZ with 0/1 after resolution.
 *
 * Revision 1.15  2005/02/12 06:13:22  steve
 *  Add debug dumps for vectors, and fix vvp_scaler_t make from BIT4_X values.
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
 * Revision 1.7  2005/01/16 04:19:08  steve
 *  Reimplement comparators as vvp_vector4_t nodes.
 *
 * Revision 1.6  2005/01/09 20:11:16  steve
 *  Add the .part/pv node and related functionality.
 *
 * Revision 1.5  2005/01/01 02:12:34  steve
 *  vvp_fun_signal propagates vvp_vector8_t vectors when appropriate.
 *
 * Revision 1.4  2004/12/31 06:00:06  steve
 *  Implement .resolv functors, and stub signals recv_vec8 method.
 *
 * Revision 1.3  2004/12/29 23:45:13  steve
 *  Add the part concatenation node (.concat).
 *
 *  Add a vvp_event_anyedge class to handle the special
 *  case of .event statements of edge type. This also
 *  frees the posedge/negedge types to handle all 4 inputs.
 *
 *  Implement table functor recv_vec4 method to receive
 *  and process vectors.
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
#endif
