#ifndef IVL_arith_H
#define IVL_arith_H
/*
 * Copyright (c) 2001-2021 Stephen Williams (steve@icarus.com)
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

      void recv_vec4_pv(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
			unsigned base, unsigned vwid, vvp_context_t ctx);

    protected:
      void dispatch_operand_(vvp_net_ptr_t ptr, const vvp_vector4_t&bit);

    protected:
      unsigned wid_;

      vvp_vector4_t op_a_;
      vvp_vector4_t op_b_;
	// Precalculated X result for propagation.
      vvp_vector4_t x_val_;
};

class vvp_arith_abs : public vvp_net_fun_t {
    public:
      explicit vvp_arith_abs();
      ~vvp_arith_abs();

      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                     vvp_context_t);
      void recv_real(vvp_net_ptr_t ptr, double bit,
                     vvp_context_t);

      void recv_vec4_pv(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
			unsigned base, unsigned vwid, vvp_context_t ctx);

    private:
};

class vvp_arith_cast_int : public vvp_net_fun_t {
    public:
      explicit vvp_arith_cast_int(unsigned wid);
      ~vvp_arith_cast_int();

      void recv_real(vvp_net_ptr_t ptr, double bit,
                     vvp_context_t);

    private:
      unsigned wid_;
};

class vvp_arith_cast_real : public vvp_net_fun_t {
    public:
      explicit vvp_arith_cast_real(bool signed_flag);
      ~vvp_arith_cast_real();

      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                     vvp_context_t);

      void recv_vec4_pv(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
			unsigned base, unsigned vwid, vvp_context_t ctx);

    private:
      bool signed_;
};

class vvp_arith_cast_vec2 : public vvp_net_fun_t {
    public:
      explicit vvp_arith_cast_vec2(unsigned wid);
      ~vvp_arith_cast_vec2();

      void recv_real(vvp_net_ptr_t ptr, double bit,
                     vvp_context_t);
      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                     vvp_context_t);

      void recv_vec4_pv(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
			unsigned base, unsigned vwid, vvp_context_t ctx);

    private:
      unsigned wid_;
};

class vvp_arith_div : public vvp_arith_ {

    public:
      explicit vvp_arith_div(unsigned wid, bool signed_flag);
      ~vvp_arith_div();
      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                     vvp_context_t);
    private:
      void wide4_(vvp_net_ptr_t ptr);
      bool signed_flag_;
};

class vvp_arith_mod : public vvp_arith_ {

    public:
      explicit vvp_arith_mod(unsigned wid, bool signed_flag);
      ~vvp_arith_mod();
      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                     vvp_context_t);
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
      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                     vvp_context_t);

};

class vvp_cmp_nee  : public vvp_arith_ {

    public:
      explicit vvp_cmp_nee(unsigned wid);
      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                     vvp_context_t);

};

class vvp_cmp_eq  : public vvp_arith_ {

    public:
      explicit vvp_cmp_eq(unsigned wid);
      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                     vvp_context_t);

};

class vvp_cmp_eqx  : public vvp_arith_ {

    public:
      explicit vvp_cmp_eqx(unsigned wid);
      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                     vvp_context_t);

};

class vvp_cmp_eqz  : public vvp_arith_ {

    public:
      explicit vvp_cmp_eqz(unsigned wid);
      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                     vvp_context_t);

};

class vvp_cmp_ne  : public vvp_arith_ {

    public:
      explicit vvp_cmp_ne(unsigned wid);
      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                     vvp_context_t);

};

class vvp_cmp_weq  : public vvp_arith_ {

    public:
      explicit vvp_cmp_weq(unsigned wid);
      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                     vvp_context_t);

};

class vvp_cmp_wne  : public vvp_arith_ {

    public:
      explicit vvp_cmp_wne(unsigned wid);
      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                     vvp_context_t);

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
      void recv_vec4_base_(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
			   vvp_bit4_t out_if_equal);
    private:
      bool signed_flag_;
};

class vvp_cmp_ge  : public vvp_cmp_gtge_base_ {

    public:
      explicit vvp_cmp_ge(unsigned wid, bool signed_flag);

      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                     vvp_context_t);

};

class vvp_cmp_gt  : public vvp_cmp_gtge_base_ {

    public:
      explicit vvp_cmp_gt(unsigned wid, bool signed_flag);

      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                     vvp_context_t);
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
      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                     vvp_context_t);
    private:
      void wide_(vvp_net_ptr_t ptr);
};

class vvp_arith_pow  : public vvp_arith_ {

    public:
      explicit vvp_arith_pow(unsigned wid, bool signed_flag);
      ~vvp_arith_pow();
      void recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
                     vvp_context_t);
    private:
      bool signed_flag_;
};

class vvp_arith_sub  : public vvp_arith_ {

    public:
      explicit vvp_arith_sub(unsigned wid);
      ~vvp_arith_sub();
      virtual void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                             vvp_context_t);

};

class vvp_arith_sum  : public vvp_arith_ {

    public:
      explicit vvp_arith_sum(unsigned wid);
      ~vvp_arith_sum();
      virtual void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                             vvp_context_t);

};

class vvp_shiftl  : public vvp_arith_ {

    public:
      explicit vvp_shiftl(unsigned wid);
      ~vvp_shiftl();
      virtual void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                             vvp_context_t);
};

class vvp_shiftr  : public vvp_arith_ {

    public:
      explicit vvp_shiftr(unsigned wid, bool signed_flag);
      ~vvp_shiftr();
      virtual void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                             vvp_context_t);

    private:
      bool signed_flag_;
};

/*
 * Base class for real valued expressions. These are similar to the
 * vector expression classes, but the inputs are collected from the
 * recv_real method.
 */
class vvp_arith_real_  : public vvp_net_fun_t {

    public:
      explicit vvp_arith_real_();

    protected:
      void dispatch_operand_(vvp_net_ptr_t ptr, double bit);

    protected:
      double op_a_;
      double op_b_;
};


class vvp_arith_sum_real : public vvp_arith_real_ {

    public:
      explicit vvp_arith_sum_real();
      ~vvp_arith_sum_real();
      void recv_real(vvp_net_ptr_t ptr, double bit,
                     vvp_context_t);
};

class vvp_arith_div_real : public vvp_arith_real_ {

    public:
      explicit vvp_arith_div_real();
      ~vvp_arith_div_real();
      void recv_real(vvp_net_ptr_t ptr, double bit,
                     vvp_context_t);
};

class vvp_arith_mod_real : public vvp_arith_real_ {

    public:
      explicit vvp_arith_mod_real();
      ~vvp_arith_mod_real();
      void recv_real(vvp_net_ptr_t ptr, double bit,
                     vvp_context_t);
};

class vvp_arith_mult_real : public vvp_arith_real_ {

    public:
      explicit vvp_arith_mult_real();
      ~vvp_arith_mult_real();
      void recv_real(vvp_net_ptr_t ptr, double bit,
                     vvp_context_t);
};

class vvp_arith_pow_real : public vvp_arith_real_ {

    public:
      explicit vvp_arith_pow_real();
      ~vvp_arith_pow_real();
      void recv_real(vvp_net_ptr_t ptr, double bit,
                     vvp_context_t);
};

class vvp_arith_sub_real : public vvp_arith_real_ {

    public:
      explicit vvp_arith_sub_real();
      ~vvp_arith_sub_real();
      void recv_real(vvp_net_ptr_t ptr, double bit,
                     vvp_context_t);
};

class vvp_cmp_eq_real  : public vvp_arith_real_ {

    public:
      explicit vvp_cmp_eq_real();
      void recv_real(vvp_net_ptr_t ptr, const double bit,
                     vvp_context_t);
};

class vvp_cmp_ne_real  : public vvp_arith_real_ {

    public:
      explicit vvp_cmp_ne_real();
      void recv_real(vvp_net_ptr_t ptr, const double bit,
                     vvp_context_t);
};

class vvp_cmp_ge_real  : public vvp_arith_real_ {

    public:
      explicit vvp_cmp_ge_real();
      void recv_real(vvp_net_ptr_t ptr, const double bit,
                     vvp_context_t);
};

class vvp_cmp_gt_real  : public vvp_arith_real_ {

    public:
      explicit vvp_cmp_gt_real();
      void recv_real(vvp_net_ptr_t ptr, const double bit,
                     vvp_context_t);
};

#endif /* IVL_arith_H */
