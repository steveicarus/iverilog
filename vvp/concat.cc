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
#ident "$Id: concat.cc,v 1.1 2005/01/22 00:01:09 steve Exp $"

# include  "compile.h"
# include  "vvp_net.h"
# include  <stdlib.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <assert.h>


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

void vvp_fun_concat::recv_vec4(vvp_net_ptr_t port, vvp_vector4_t bit)
{
      unsigned pdx = port.port();

      assert(bit.size() == wid_[pdx]);

      unsigned off = 0;
      for (unsigned idx = 0 ;  idx < pdx ;  idx += 1)
	    off += wid_[idx];

      for (unsigned idx = 0 ;  idx < wid_[pdx] ;  idx += 1) {
	    val_.set_bit(off+idx, bit.value(idx));
      }

      vvp_send_vec4(port.ptr()->out, val_);
}

void vvp_fun_concat::recv_vec8(vvp_net_ptr_t port, vvp_vector8_t bit)
{
      recv_vec4(port, reduce4(bit));
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
}


/*
 * $Log: concat.cc,v $
 * Revision 1.1  2005/01/22 00:01:09  steve
 *  Add missing concat.cc to cvs
 *
 */

