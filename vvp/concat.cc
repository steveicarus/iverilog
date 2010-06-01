/*
 * Copyright (c) 2004-2010 Stephen Williams (steve@icarus.com)
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
# include  "vvp_net.h"
# include  <cstdlib>
# include  <iostream>
# include  <cassert>


vvp_fun_concat::vvp_fun_concat(unsigned w0, unsigned w1,
			       unsigned w2, unsigned w3)
: val_(w0+w1+w2+w3)
{
      wid_[0] = w0;
      wid_[1] = w1;
      wid_[2] = w2;
      wid_[3] = w3;

      for (unsigned idx = 0 ;  idx < val_.size() ;  idx += 1)
	    val_.set_bit(idx, BIT4_X);
}

vvp_fun_concat::~vvp_fun_concat()
{
}

void vvp_fun_concat::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                               vvp_context_t)
{
      unsigned pdx = port.port();

      if (bit.size() != wid_[pdx]) {
	    cerr << "internal error: port " << pdx
		 << " expects wid=" << wid_[pdx]
		 << ", got wid=" << bit.size() << endl;
	    assert(0);
      }

      unsigned off = 0;
      for (unsigned idx = 0 ;  idx < pdx ;  idx += 1)
	    off += wid_[idx];

      for (unsigned idx = 0 ;  idx < wid_[pdx] ;  idx += 1) {
	    val_.set_bit(off+idx, bit.value(idx));
      }

      vvp_send_vec4(port.ptr()->out, val_, 0);
}

void compile_concat(char*label, unsigned w0, unsigned w1,
		    unsigned w2, unsigned w3,
		    unsigned argc, struct symb_s*argv)
{
      vvp_fun_concat*fun = new vvp_fun_concat(w0, w1, w2, w3);

      vvp_net_t*net = new vvp_net_t;
      net->fun = fun;

      define_functor_symbol(label, net);
      free(label);

      inputs_connect(net, argc, argv);
      free(argv);
}

vvp_fun_repeat::vvp_fun_repeat(unsigned width, unsigned repeat)
: wid_(width), rep_(repeat)
{
}

vvp_fun_repeat::~vvp_fun_repeat()
{
}

void vvp_fun_repeat::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                               vvp_context_t)
{
      assert(bit.size() == wid_/rep_);

      vvp_vector4_t val (wid_);

      for (unsigned rdx = 0 ;  rdx < rep_ ;  rdx += 1) {
	    unsigned off = rdx * bit.size();

	    for (unsigned idx = 0 ; idx < bit.size() ;  idx += 1)
		  val.set_bit(off+idx, bit.value(idx));

      }

      vvp_send_vec4(port.ptr()->out, val, 0);
}

void compile_repeat(char*label, long width, long repeat, struct symb_s arg)
{
      vvp_fun_repeat*fun = new vvp_fun_repeat(width, repeat);

      vvp_net_t*net = new vvp_net_t;
      net->fun = fun;

      define_functor_symbol(label, net);
      free(label);

      input_connect(net, 0, arg.text);
}
