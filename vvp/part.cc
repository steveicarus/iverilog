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
#ident "$Id: part.cc,v 1.1 2004/12/11 02:31:30 steve Exp $"

# include  "compile.h"
# include  "vvp_net.h"
# include  <stdlib.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <assert.h>

vvp_fun_part::vvp_fun_part(unsigned base, unsigned wid)
: base_(base), wid_(wid)
{
}

vvp_fun_part::~vvp_fun_part()
{
}

void vvp_fun_part::recv_vec4(vvp_net_ptr_t port, vvp_vector4_t bit)
{
      assert(port.port() == 0);

      vvp_vector4_t res(wid_);

      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    if (idx + base_ < bit.size())
		  res.set_bit(idx, bit.value(base_+idx));
	    else
		  res.set_bit(idx, BIT4_X);
      }

}

void compile_part_select(char*label, char*source,
			 unsigned base, unsigned wid)
{
      vvp_fun_part*fun = new vvp_fun_part(base, wid);

      vvp_net_t*net = new vvp_net_t;
      net->fun = fun;

      define_functor_symbol(label, net);
      free(label);

      input_connect(net, 0, source);
}

/*
 * $Log: part.cc,v $
 * Revision 1.1  2004/12/11 02:31:30  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 */

