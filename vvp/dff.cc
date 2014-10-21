/*
 * Copyright (c) 2005-2010 Stephen Williams (steve@icarus.com)
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
# include  "dff.h"
# include  <climits>
# include  <cstdio>
# include  <cassert>
# include  <cstdlib>
# include  <iostream>

vvp_dff::vvp_dff(bool invert_clk, bool invert_ce)
: iclk_(invert_clk), ice_(invert_ce)
{
      clk_cur_ = BIT4_X;
      enable_ = BIT4_X;
}

vvp_dff::~vvp_dff()
{
}

void vvp_dff::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                        vvp_context_t)
{
      vvp_bit4_t tmp;

      switch (port.port()) {

	  case 0: // D
	    d_ = bit;
	    break;

	      /* This is a clock input */
	  case 1: // CLK
	    assert(bit.size() == 1);
	    if (enable_ != BIT4_1)
		  break;
	    tmp = clk_cur_;
	    clk_cur_ = bit.value(0);
	    if (clk_cur_ == BIT4_1 && tmp != BIT4_1)
		  port.ptr()->send_vec4(d_, 0);
	    break;

	  case 2: // CE
	    assert(bit.size() == 1);
	    enable_ = bit.value(0);
	    break;

	  case 3: // Asynch-D
	    assert(0);
	    break;
      }
}

/*
 * The recv_clear and recv_set function respond to asynchronout
 * clear/set input by propagating the desired output.
 *
 * NOTE: Don't touch the d_ value, because that tracks the D input,
 * which may be needed when the device is clocked afterwards.
 */
void vvp_dff::recv_clear(vvp_net_ptr_t port)
{
      vvp_vector4_t tmp = d_;
      for (unsigned idx = 0 ; idx < d_.size() ; idx += 1)
	    tmp.set_bit(idx, BIT4_0);

      port.ptr()->send_vec4(tmp, 0);
}

void vvp_dff::recv_set(vvp_net_ptr_t port)
{
      vvp_vector4_t tmp = d_;
      for (unsigned idx = 0 ; idx < d_.size() ; idx += 1)
	    tmp.set_bit(idx, BIT4_1);

      port.ptr()->send_vec4(tmp, 0);
}

vvp_dff_aclr::vvp_dff_aclr(bool invert_clk, bool invert_ce)
: vvp_dff(invert_clk, invert_ce)
{
      a_ = BIT4_X;
}

void vvp_dff_aclr::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
			     vvp_context_t ctx)
{
      if (port.port() == 3) {

	    assert(bit.size()==1);
	    if (a_ == bit.value(0))
		  return;

	    a_ = bit.value(0);
	    recv_clear(port);

      } else {
	    vvp_dff::recv_vec4(port, bit, ctx);
      }
}

vvp_dff_aset::vvp_dff_aset(bool invert_clk, bool invert_ce)
: vvp_dff(invert_clk, invert_ce)
{
      a_ = BIT4_X;
}

void vvp_dff_aset::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
			     vvp_context_t ctx)
{
      if (port.port() == 3) {

	    assert(bit.size()==1);
	    if (a_ == bit.value(0))
		  return;

	    a_ = bit.value(0);
	    recv_set(port);

      } else {
	    vvp_dff::recv_vec4(port, bit, ctx);
      }
}

void compile_dff(char*label, struct symb_s arg_d,
		 struct symb_s arg_c,
		 struct symb_s arg_e)
{
      vvp_net_t*ptr = new vvp_net_t;
      vvp_dff*fun = new vvp_dff(false, false);

      ptr->fun = fun;
      define_functor_symbol(label, ptr);
      free(label);
      input_connect(ptr, 0, arg_d.text);
      input_connect(ptr, 1, arg_c.text);
      input_connect(ptr, 2, arg_e.text);
}

void compile_dff_aclr(char*label, struct symb_s arg_d,
		      struct symb_s arg_c,
		      struct symb_s arg_e,
		      struct symb_s arg_a)
{
      vvp_net_t*ptr = new vvp_net_t;
      vvp_dff*fun = new vvp_dff_aclr(false, false);

      ptr->fun = fun;
      define_functor_symbol(label, ptr);
      free(label);
      input_connect(ptr, 0, arg_d.text);
      input_connect(ptr, 1, arg_c.text);
      input_connect(ptr, 2, arg_e.text);
      input_connect(ptr, 3, arg_a.text);
}

void compile_dff_aset(char*label, struct symb_s arg_d,
		      struct symb_s arg_c,
		      struct symb_s arg_e,
		      struct symb_s arg_a)
{
      vvp_net_t*ptr = new vvp_net_t;
      vvp_dff*fun = new vvp_dff_aset(false, false);

      ptr->fun = fun;
      define_functor_symbol(label, ptr);
      free(label);
      input_connect(ptr, 0, arg_d.text);
      input_connect(ptr, 1, arg_c.text);
      input_connect(ptr, 2, arg_e.text);
      input_connect(ptr, 3, arg_a.text);
}
