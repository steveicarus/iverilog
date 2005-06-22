#ifndef __arith_H
#define __arith_H
/*
 * Copyright (c) 2001-2005 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: arith.h,v 1.31 2005/06/22 00:04:48 steve Exp $"
#endif

# include  "vvp_net.h"

/*
 * Base class for arithmetic functors.
 * The wid constructor is used to size the output. This includes
 * precalculating an X value. Most arithmetic nodes can handle
 * whatever width comes in, given the knowledge of the output width.
 *
 * The width is also used to make initial values for the op_a_ and
 * op_b_ operands. Most arithmetic operators expect the widths of the
 * inputs to match, and since only one input at a time changes, the
 * other will need to be initialized to X.
 */
class vvp_arith_  : public vvp_net_fun_t {

    public:
      explicit vvp_arith_(unsigned wid);

    protected:
      void dispatch_operand_(vvp_net_ptr_t ptr, vvp_vector4_t bit);

    protected:
      unsigned wid_;

      vvp_vector4_t op_a_;
      vvp_vector4_t op_b_;
	// Precalculated X result for propagation.
      vvp_vector4_t x_val_;
};

class vvp_arith_div : public vvp_arith_ {

    public:
      explicit vvp_arith_div(unsigned wid, bool signed_flag);
      ~vvp_arith_div();
      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit);
    private:
      void wide_(vvp_net_ptr_t ptr);
      bool signed_flag_;
};

class vvp_arith_mod : public vvp_arith_ {

    public:
      explicit vvp_arith_mod(unsigned wid, bool signed_flag);
      ~vvp_arith_mod();
      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit);
    private:
      void wide_(vvp_net_ptr_t ptr);
      bool signed_flag_;
};

/* vvp_cmp_* objects...
 * the vvp_cmp_* objects all are special vvp_arith_ objects in that
 * their widths are only for their inputs. The output widths are all
 * exactly 1 bit.
 */

class vvp_cmp_eeq  : public vvp_arith_ {

    public:
      explicit vvp_cmp_eeq(unsigned wid);
      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit);

};

class vvp_cmp_nee  : public vvp_arith_ {

    public:
      explicit vvp_cmp_nee(unsigned wid);
      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit);

};

class vvp_cmp_eq  : public vvp_arith_ {

    public:
      explicit vvp_cmp_eq(unsigned wid);
      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit);

};

class vvp_cmp_ne  : public vvp_arith_ {

    public:
      explicit vvp_cmp_ne(unsigned wid);
      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit);

};


/*
 * This base class implements both GT and GE comparisons. The derived
 * GT and GE call the recv_vec4_base_ method with a different
 * out_if_equal argument that reflects the different expectations.
 */
class vvp_cmp_gtge_base_ : public vvp_arith_ {

    public:
      explicit vvp_cmp_gtge_base_(unsigned wid, bool signed_flag);

    protected:
      void recv_vec4_base_(vvp_net_ptr_t ptr, vvp_vector4_t bit,
			   vvp_bit4_t out_if_equal);
    private:
      bool signed_flag_;
};

class vvp_cmp_ge  : public vvp_cmp_gtge_base_ {

    public:
      explicit vvp_cmp_ge(unsigned wid, bool signed_flag);

      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit);

};

class vvp_cmp_gt  : public vvp_cmp_gtge_base_ {

    public:
      explicit vvp_cmp_gt(unsigned wid, bool signed_flag);

      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit);
};

/*
 * NOTE: The inputs to the vvp_arith_mult are not necessarily the same
 * width as the output. This is different from the typical vvp_arith_
 * object. Perhaps that means this isn't quite a vvp_arith_ object?
 */
class vvp_arith_mult  : public vvp_arith_ {

    public:
      explicit vvp_arith_mult(unsigned wid);
      ~vvp_arith_mult();
      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit);
    private:
      void wide_(vvp_net_ptr_t ptr);
};

class vvp_arith_sub  : public vvp_arith_ {

    public:
      explicit vvp_arith_sub(unsigned wid);
      ~vvp_arith_sub();
      virtual void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit);

};

class vvp_arith_sum  : public vvp_arith_ {

    public:
      explicit vvp_arith_sum(unsigned wid);
      ~vvp_arith_sum();
      virtual void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit);

};

class vvp_shiftl  : public vvp_arith_ {

    public:
      explicit vvp_shiftl(unsigned wid);
      ~vvp_shiftl();
      virtual void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit);
};

class vvp_shiftr  : public vvp_arith_ {

    public:
      explicit vvp_shiftr(unsigned wid);
      ~vvp_shiftr();
      virtual void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit);
};

/*
 * $Log: arith.h,v $
 * Revision 1.31  2005/06/22 00:04:48  steve
 *  Reduce vvp_vector4 copies by using const references.
 *
 * Revision 1.30  2005/06/11 18:11:18  steve
 *  Remove unneeded references to functor.h
 *
 * Revision 1.29  2005/03/19 06:23:49  steve
 *  Handle LPM shifts.
 *
 * Revision 1.28  2005/03/12 06:42:28  steve
 *  Implement .arith/mod.
 *
 * Revision 1.27  2005/03/09 05:52:04  steve
 *  Handle case inequality in netlists.
 *
 * Revision 1.26  2005/02/19 01:32:52  steve
 *  Implement .arith/div.
 *
 * Revision 1.25  2005/02/04 05:13:02  steve
 *  Add wide .arith/mult, and vvp_vector2_t vectors.
 *
 * Revision 1.24  2005/01/28 05:34:25  steve
 *  Add vector4 implementation of .arith/mult.
 *
 * Revision 1.23  2005/01/22 16:21:11  steve
 *  Implement vectored CMP_EQ and NE
 *
 * Revision 1.22  2005/01/22 01:06:20  steve
 *  Implement the .cmp/eeq LPM node.
 *
 * Revision 1.21  2005/01/16 04:19:08  steve
 *  Reimplement comparators as vvp_vector4_t nodes.
 *
 * Revision 1.20  2004/12/11 02:31:29  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 */
#endif
