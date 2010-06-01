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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include  "compile.h"
# include  "schedule.h"
# include  "dff.h"
# include  <climits>
# include  <cstdio>
# include  <cassert>
# include  <cstdlib>

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
		  vvp_send_vec4(port.ptr()->out, d_, 0);
	    break;

	  case 2: // CE
	    assert(bit.size() == 1);
	    enable_ = bit.value(0);
	    break;

	  case 3: // Asynch-D
	    d_ = bit;
	    vvp_send_vec4(port.ptr()->out, d_, 0);
	    break;
      }
}

void compile_dff(char*label, struct symb_s arg_d,
		 struct symb_s arg_c,
		 struct symb_s arg_e,
		 struct symb_s arg_a)
{
      vvp_net_t*ptr = new vvp_net_t;
      vvp_dff*fun = new vvp_dff(false, false);

      ptr->fun = fun;
      define_functor_symbol(label, ptr);
      free(label);
      input_connect(ptr, 0, arg_d.text);
      input_connect(ptr, 1, arg_c.text);
      input_connect(ptr, 2, arg_e.text);
      input_connect(ptr, 3, arg_a.text);
}
