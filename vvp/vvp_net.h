#ifndef __vvp_net_H
#define __vvp_net_H
/*
 * Copyright (c) 2004 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vvp_net.h,v 1.8 2005/01/22 17:36:15 steve Exp $"

# include  <assert.h>

/* Data types */
class  vvp_scaler_t;

/* Basic netlist types. */
class  vvp_net_t;
class  vvp_net_ptr_t;
class  vvp_net_fun_t;

/* Core net function types. */
class  vvp_fun_concat;
class  vvp_fun_drive;
class  vvp_fun_part;

/*
 * This is the set of Verilog 4-value bit values. Scalers have this
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

/*
 * This class represents scaler values collected into vectors. The
 * vector values can be accessed individually, or treated as a
 * unit. in any case, the elements of the vector are addressed from
 * zero(LSB) to size-1(MSB).
 *
 * No strength values are stored here, if strengths are needed, use a
 * collection of vvp_scaler_t objects instead.
 */
class vvp_vector4_t {

    public:
      explicit vvp_vector4_t(unsigned size =0);

      ~vvp_vector4_t();

      unsigned size() const { return size_; }
      vvp_bit4_t value(unsigned idx) const;
      void set_bit(unsigned idx, vvp_bit4_t val);

      vvp_vector4_t(const vvp_vector4_t&that);
      vvp_vector4_t& operator= (const vvp_vector4_t&that);

    private:
      unsigned size_;
      union {
	    unsigned long bits_val_;
	    unsigned long*bits_ptr_;
      };
};

extern vvp_bit4_t compare_gtge(const vvp_vector4_t&a,
			       const vvp_vector4_t&b,
			       vvp_bit4_t val_if_equal);
extern vvp_bit4_t compare_gtge_signed(const vvp_vector4_t&a,
				      const vvp_vector4_t&b,
				      vvp_bit4_t val_if_equal);

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
 */
class vvp_scaler_t {

      friend vvp_scaler_t resolve(vvp_scaler_t a, vvp_scaler_t b);

    public:
	// Make a HiZ value.
      explicit vvp_scaler_t();

	// Make an unambiguous value.
      explicit vvp_scaler_t(vvp_bit4_t val, unsigned str);

	// Get the vvp_bit4_t version of the value
      vvp_bit4_t value() const;

    private:
      unsigned char value_;
};

extern vvp_scaler_t resolve(vvp_scaler_t a, vvp_scaler_t b);


/*
 * This class is a way to carry vectors of strength modeled
 * values. The 8 in the name is the number of possible distinct values
 * a well defined bit may have. When you add in ambiguous values, the
 * number of distinct values span the vvp_scaler_t.
 *
 * a vvp_vector8_t object can be created from a vvp_vector4_t and a
 * strength value. The vvp_vector8_t bits have the values of the input
 * vector, all with the strength specified.
 */
class vvp_vector8_t {

    public:
      explicit vvp_vector8_t(unsigned size =0);
	// Make a vvp_vector8_t from a vector4 and a specified strength.
      explicit vvp_vector8_t(const vvp_vector4_t&that, unsigned str);

      ~vvp_vector8_t();

      unsigned size() const { return size_; }
      vvp_scaler_t value(unsigned idx) const;
      void set_bit(unsigned idx, vvp_scaler_t val);

      vvp_vector8_t(const vvp_vector8_t&that);
      vvp_vector8_t& operator= (const vvp_vector8_t&that);

    private:
      unsigned size_;
      vvp_scaler_t*bits_;
};

extern vvp_vector8_t resolve(const vvp_vector8_t&a, const vvp_vector8_t&b);
extern vvp_vector4_t reduce4(const vvp_vector8_t&that);

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

extern void vvp_send_vec4(vvp_net_ptr_t ptr, vvp_vector4_t val);
extern void vvp_send_vec8(vvp_net_ptr_t ptr, vvp_vector8_t val);
extern void vvp_send_real(vvp_net_ptr_t ptr, double val);
extern void vvp_send_long(vvp_net_ptr_t ptr, long val);

/*
 * Part-vector versions of above functions.
 */
extern void vvp_send_vec4_pv(vvp_net_ptr_t ptr, vvp_vector4_t val,
			     unsigned base, unsigned wid, unsigned vwid);

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
      void recv_vec8(vvp_net_ptr_t port, vvp_vector8_t bit);

    private:
      unsigned wid_[4];
      vvp_vector4_t val_;
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

      void recv_vec4(vvp_net_ptr_t port, vvp_vector4_t bit);
      void recv_long(vvp_net_ptr_t port, long bit);

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

	// Get information about the vector value.
      unsigned   size() const;
      vvp_bit4_t value(unsigned idx) const;

	// Commands
      void deassign();
      void release(vvp_net_ptr_t port, bool net);

    public:
      struct __vpiCallback*vpi_callbacks;

    private:
      vvp_vector4_t bits4_;
      vvp_vector8_t bits8_;
      bool type_is_vector8_() const;

      bool continuous_assign_active_;

      vvp_vector4_t force_;
      bool force_active_;

    private:
      void run_vpi_callbacks();
};

/*
 * $Log: vvp_net.h,v $
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
