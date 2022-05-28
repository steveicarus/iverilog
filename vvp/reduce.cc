/*
 * Copyright (c) 2005-2016 Stephen Williams (steve@icarus.com)
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

# include  "compile.h"
# include  "schedule.h"
# include  <climits>
# include  <cstdio>
# include  <cassert>
# include  <cstdlib>

/*
 * All the reduction operations take a single vector input and produce
 * a scalar result. The vvp_reduce_base class codifies these general
 * characteristics, leaving only the calculation of the result for the
 * base class. This can be used in both statically and automatically
 * allocated scopes, as bits_ is only used for temporary storage.
 */
class vvp_reduce_base : public vvp_net_fun_t {

    public:
      vvp_reduce_base();
      virtual ~vvp_reduce_base();

      void recv_vec4(vvp_net_ptr_t prt, const vvp_vector4_t&bit,
                     vvp_context_t context);
      void recv_vec4_pv(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
			unsigned base, unsigned vwid, vvp_context_t context);

      virtual vvp_bit4_t calculate_result() const =0;

    protected:
      vvp_vector4_t bits_;
};

vvp_reduce_base::vvp_reduce_base()
{
}

vvp_reduce_base::~vvp_reduce_base()
{
}

void vvp_reduce_base::recv_vec4(vvp_net_ptr_t prt, const vvp_vector4_t&bit,
                                vvp_context_t context)
{
      bits_ = bit;
      vvp_bit4_t res =  calculate_result();
      vvp_vector4_t rv (1, res);
      prt.ptr()->send_vec4(rv, context);
}

void vvp_reduce_base::recv_vec4_pv(vvp_net_ptr_t prt, const vvp_vector4_t&bit,
				   unsigned base, unsigned vwid, vvp_context_t context)
{
      if (bits_.size() == 0) {
	    bits_ = vvp_vector4_t(vwid);
      }
      assert(bits_.size() == vwid);

      bits_.set_vec(base, bit);
      vvp_bit4_t res = calculate_result();
      vvp_vector4_t rv (1, res);
      prt.ptr()->send_vec4(rv, context);
}

class vvp_reduce_and  : public vvp_reduce_base {

    public:
      vvp_reduce_and();
      ~vvp_reduce_and();
      vvp_bit4_t calculate_result() const;
};

vvp_reduce_and::vvp_reduce_and()
{
}

vvp_reduce_and::~vvp_reduce_and()
{
}

vvp_bit4_t vvp_reduce_and::calculate_result() const
{
      vvp_bit4_t res =  BIT4_1;

      for (unsigned idx = 0 ;  idx < bits_.size() ;  idx += 1)
	    res = res & bits_.value(idx);

      return res;
}

class vvp_reduce_or  : public vvp_reduce_base {

    public:
      vvp_reduce_or();
      ~vvp_reduce_or();
      vvp_bit4_t calculate_result() const;
};

vvp_reduce_or::vvp_reduce_or()
{
}

vvp_reduce_or::~vvp_reduce_or()
{
}

vvp_bit4_t vvp_reduce_or::calculate_result() const
{
      vvp_bit4_t res =  BIT4_0;

      for (unsigned idx = 0 ;  idx < bits_.size() ;  idx += 1)
	    res = res | bits_.value(idx);

      return res;
}

class vvp_reduce_xor  : public vvp_reduce_base {

    public:
      vvp_reduce_xor();
      ~vvp_reduce_xor();
      vvp_bit4_t calculate_result() const;
};

vvp_reduce_xor::vvp_reduce_xor()
{
}

vvp_reduce_xor::~vvp_reduce_xor()
{
}

vvp_bit4_t vvp_reduce_xor::calculate_result() const
{
      vvp_bit4_t res =  BIT4_0;

      for (unsigned idx = 0 ;  idx < bits_.size() ;  idx += 1)
	    res = res ^ bits_.value(idx);

      return res;
}

class vvp_reduce_nand  : public vvp_reduce_base {

    public:
      vvp_reduce_nand();
      ~vvp_reduce_nand();
      vvp_bit4_t calculate_result() const;
};

vvp_reduce_nand::vvp_reduce_nand()
{
}

vvp_reduce_nand::~vvp_reduce_nand()
{
}

vvp_bit4_t vvp_reduce_nand::calculate_result() const
{
      vvp_bit4_t res =  BIT4_1;

      for (unsigned idx = 0 ;  idx < bits_.size() ;  idx += 1)
	    res = res & bits_.value(idx);

      return ~res;
}

class vvp_reduce_nor  : public vvp_reduce_base {

    public:
      vvp_reduce_nor();
      ~vvp_reduce_nor();
      vvp_bit4_t calculate_result() const;
};

vvp_reduce_nor::vvp_reduce_nor()
{
}

vvp_reduce_nor::~vvp_reduce_nor()
{
}

vvp_bit4_t vvp_reduce_nor::calculate_result() const
{
      vvp_bit4_t res =  BIT4_0;

      for (unsigned idx = 0 ;  idx < bits_.size() ;  idx += 1)
	    res = res | bits_.value(idx);

      return ~res;
}

class vvp_reduce_xnor  : public vvp_reduce_base {

    public:
      vvp_reduce_xnor();
      ~vvp_reduce_xnor();
      vvp_bit4_t calculate_result() const;
};

vvp_reduce_xnor::vvp_reduce_xnor()
{
}

vvp_reduce_xnor::~vvp_reduce_xnor()
{
}

vvp_bit4_t vvp_reduce_xnor::calculate_result() const
{
      vvp_bit4_t res =  BIT4_0;

      for (unsigned idx = 0 ;  idx < bits_.size() ;  idx += 1)
	    res = res ^ bits_.value(idx);

      return ~res;
}

static void make_reduce(char*label, vvp_net_fun_t*red, const struct symb_s&arg)
{
      vvp_net_t*ptr = new vvp_net_t;
      ptr->fun = red;

      define_functor_symbol(label, ptr);
      free(label);

      input_connect(ptr, 0, arg.text);
}

void compile_reduce_and(char*label, const struct symb_s&arg)
{
      vvp_reduce_and* reduce = new vvp_reduce_and;
      make_reduce(label, reduce, arg);
}

void compile_reduce_or(char*label, const struct symb_s&arg)
{
      vvp_reduce_or* reduce = new vvp_reduce_or;
      make_reduce(label, reduce, arg);
}

void compile_reduce_xor(char*label, const struct symb_s&arg)
{
      vvp_reduce_xor* reduce = new vvp_reduce_xor;
      make_reduce(label, reduce, arg);
}

void compile_reduce_nand(char*label, const struct symb_s&arg)
{
      vvp_reduce_nand* reduce = new vvp_reduce_nand;
      make_reduce(label, reduce, arg);
}

void compile_reduce_nor(char*label, const struct symb_s&arg)
{
      vvp_reduce_nor* reduce = new vvp_reduce_nor;
      make_reduce(label, reduce, arg);
}

void compile_reduce_xnor(char*label, const struct symb_s&arg)
{
      vvp_reduce_xnor* reduce = new vvp_reduce_xnor;
      make_reduce(label, reduce, arg);
}
