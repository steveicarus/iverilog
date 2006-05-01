/*
 * Copyright (c) 2005 Stephen Williams (steve@icarus.com)
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
#ident "$Id: reduce.cc,v 1.3 2006/05/01 18:44:08 steve Exp $"
#endif

# include  "compile.h"
# include  "schedule.h"
# include  <limits.h>
# include  <stdio.h>
# include  <assert.h>
# include  <stdlib.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif

class vvp_reduce_and  : public vvp_net_fun_t {

    public:
      vvp_reduce_and();
      ~vvp_reduce_and();
      void recv_vec4(vvp_net_ptr_t prt, const vvp_vector4_t&bit);
};

vvp_reduce_and::vvp_reduce_and()
{
}

vvp_reduce_and::~vvp_reduce_and()
{
}

void vvp_reduce_and::recv_vec4(vvp_net_ptr_t prt, const vvp_vector4_t&bit)
{
      vvp_bit4_t res =  BIT4_1;

      for (unsigned idx = 0 ;  idx < bit.size() ;  idx += 1)
	    res = res & bit.value(idx);

      vvp_vector4_t rv (1, res);
      vvp_send_vec4(prt.ptr()->out, rv);
}

class vvp_reduce_or  : public vvp_net_fun_t {

    public:
      vvp_reduce_or();
      ~vvp_reduce_or();
      void recv_vec4(vvp_net_ptr_t prt, const vvp_vector4_t&bit);
};

vvp_reduce_or::vvp_reduce_or()
{
}

vvp_reduce_or::~vvp_reduce_or()
{
}

void vvp_reduce_or::recv_vec4(vvp_net_ptr_t prt, const vvp_vector4_t&bit)
{
      vvp_bit4_t res =  BIT4_0;

      for (unsigned idx = 0 ;  idx < bit.size() ;  idx += 1)
	    res = res | bit.value(idx);

      vvp_vector4_t rv (1, res);
      vvp_send_vec4(prt.ptr()->out, rv);
}

class vvp_reduce_xor  : public vvp_net_fun_t {

    public:
      vvp_reduce_xor();
      ~vvp_reduce_xor();
      void recv_vec4(vvp_net_ptr_t prt, const vvp_vector4_t&bit);
};

vvp_reduce_xor::vvp_reduce_xor()
{
}

vvp_reduce_xor::~vvp_reduce_xor()
{
}

void vvp_reduce_xor::recv_vec4(vvp_net_ptr_t prt, const vvp_vector4_t&bit)
{
      vvp_bit4_t res =  BIT4_0;

      for (unsigned idx = 0 ;  idx < bit.size() ;  idx += 1)
	    res = res ^ bit.value(idx);

      vvp_vector4_t rv (1, res);
      vvp_send_vec4(prt.ptr()->out, rv);
}

class vvp_reduce_nand  : public vvp_net_fun_t {

    public:
      vvp_reduce_nand();
      ~vvp_reduce_nand();
      void recv_vec4(vvp_net_ptr_t prt, const vvp_vector4_t&bit);
};

vvp_reduce_nand::vvp_reduce_nand()
{
}

vvp_reduce_nand::~vvp_reduce_nand()
{
}

void vvp_reduce_nand::recv_vec4(vvp_net_ptr_t prt, const vvp_vector4_t&bit)
{
      vvp_bit4_t res =  BIT4_1;

      for (unsigned idx = 0 ;  idx < bit.size() ;  idx += 1)
	    res = res & bit.value(idx);

      vvp_vector4_t rv (1, res);
      vvp_send_vec4(prt.ptr()->out, rv);
}

class vvp_reduce_nor  : public vvp_net_fun_t {

    public:
      vvp_reduce_nor();
      ~vvp_reduce_nor();
      void recv_vec4(vvp_net_ptr_t prt, const vvp_vector4_t&bit);
};

vvp_reduce_nor::vvp_reduce_nor()
{
}

vvp_reduce_nor::~vvp_reduce_nor()
{
}

void vvp_reduce_nor::recv_vec4(vvp_net_ptr_t prt, const vvp_vector4_t&bit)
{
      vvp_bit4_t res =  BIT4_0;

      for (unsigned idx = 0 ;  idx < bit.size() ;  idx += 1)
	    res = res | bit.value(idx);

      vvp_vector4_t rv (1, ~res);
      vvp_send_vec4(prt.ptr()->out, rv);
}

class vvp_reduce_xnor  : public vvp_net_fun_t {

    public:
      vvp_reduce_xnor();
      ~vvp_reduce_xnor();
      void recv_vec4(vvp_net_ptr_t prt, const vvp_vector4_t&bit);
};

vvp_reduce_xnor::vvp_reduce_xnor()
{
}

vvp_reduce_xnor::~vvp_reduce_xnor()
{
}

void vvp_reduce_xnor::recv_vec4(vvp_net_ptr_t prt, const vvp_vector4_t&bit)
{
      vvp_bit4_t res =  BIT4_0;

      for (unsigned idx = 0 ;  idx < bit.size() ;  idx += 1)
	    res = res ^ bit.value(idx);

      vvp_vector4_t rv (1, ~res);
      vvp_send_vec4(prt.ptr()->out, rv);
}

static void make_reduce(char*label, vvp_net_fun_t*red, struct symb_s arg)
{
      vvp_net_t*ptr = new vvp_net_t;
      ptr->fun = red;

      define_functor_symbol(label, ptr);
      free(label);

      input_connect(ptr, 0, arg.text);
}

void compile_reduce_and(char*label, struct symb_s arg)
{
      vvp_reduce_and* reduce = new vvp_reduce_and;
      make_reduce(label, reduce, arg);
}

void compile_reduce_or(char*label, struct symb_s arg)
{
      vvp_reduce_or* reduce = new vvp_reduce_or;
      make_reduce(label, reduce, arg);
}

void compile_reduce_xor(char*label, struct symb_s arg)
{
      vvp_reduce_xor* reduce = new vvp_reduce_xor;
      make_reduce(label, reduce, arg);
}

void compile_reduce_nand(char*label, struct symb_s arg)
{
      vvp_reduce_nand* reduce = new vvp_reduce_nand;
      make_reduce(label, reduce, arg);
}

void compile_reduce_nor(char*label, struct symb_s arg)
{
      vvp_reduce_nor* reduce = new vvp_reduce_nor;
      make_reduce(label, reduce, arg);
}

void compile_reduce_xnor(char*label, struct symb_s arg)
{
      vvp_reduce_xnor* reduce = new vvp_reduce_xnor;
      make_reduce(label, reduce, arg);
}

/*
 * $Log: reduce.cc,v $
 * Revision 1.3  2006/05/01 18:44:08  steve
 *  Reduce steps to make logic output.
 *
 * Revision 1.2  2005/06/22 00:04:49  steve
 *  Reduce vvp_vector4 copies by using const references.
 *
 * Revision 1.1  2005/02/03 04:55:13  steve
 *  Add support for reduction logic gates.
 *
 */
