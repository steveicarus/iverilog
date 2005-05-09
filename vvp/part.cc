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
#ident "$Id: part.cc,v 1.5 2005/05/09 00:36:58 steve Exp $"

# include  "compile.h"
# include  "vvp_net.h"
# include  <stdlib.h>
# include  <limits.h>
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

      vvp_send_vec4(port.ptr()->out, res);
}

vvp_fun_part_pv::vvp_fun_part_pv(unsigned b, unsigned w, unsigned v)
: base_(b), wid_(w), vwid_(v)
{
}

vvp_fun_part_pv::~vvp_fun_part_pv()
{
}

void vvp_fun_part_pv::recv_vec4(vvp_net_ptr_t port, vvp_vector4_t bit)
{
      assert(port.port() == 0);
      assert(bit.size() == wid_);
      vvp_send_vec4_pv(port.ptr()->out, bit, base_, wid_, vwid_);
}

vvp_fun_part_var::vvp_fun_part_var(unsigned w)
: base_(0), wid_(w)
{
}

vvp_fun_part_var::~vvp_fun_part_var()
{
}

void vvp_fun_part_var::recv_vec4(vvp_net_ptr_t port, vvp_vector4_t bit)
{
      unsigned long tmp;
      switch (port.port()) {
	  case 0:
	    source_ = bit;
	    break;
	  case 1:
	    tmp = ULONG_MAX;
	    vector4_to_value(bit, tmp);
	    if (tmp == base_) return;
	    base_ = tmp;
	    break;
	  default:
	    assert(0);
	    break;
      }

      vvp_vector4_t res (wid_);

      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    unsigned adr = base_+idx;
	    if (adr >= source_.size())
		  break;

	    res.set_bit(idx, source_.value(adr));
      }

      if (! ref_.eeq(res)) {
	    ref_ = res;
	    vvp_send_vec4(port.ptr()->out, res);
      }
}


/*
 * Given a node functor, create a network node and link it into the
 * netlist. This form assumes nodes with a single input.
 */
void link_node_1(char*label, char*source, vvp_net_fun_t*fun)
{
      vvp_net_t*net = new vvp_net_t;
      net->fun = fun;

      define_functor_symbol(label, net);
      free(label);

      input_connect(net, 0, source);
}

void compile_part_select(char*label, char*source,
			 unsigned base, unsigned wid)
{
      vvp_fun_part*fun = new vvp_fun_part(base, wid);
      link_node_1(label, source, fun);
}

void compile_part_select_pv(char*label, char*source,
			    unsigned base, unsigned wid,
			    unsigned vector_wid)
{
      vvp_fun_part_pv*fun = new vvp_fun_part_pv(base, wid, vector_wid);
      link_node_1(label, source, fun);
}

void compile_part_select_var(char*label, char*source, char*var,
			     unsigned wid)
{
      vvp_fun_part_var*fun = new vvp_fun_part_var(wid);
      vvp_net_t*net = new vvp_net_t;
      net->fun = fun;

      define_functor_symbol(label, net);
      free(label);

      input_connect(net, 0, source);
      input_connect(net, 1, var);
}

/*
 * $Log: part.cc,v $
 * Revision 1.5  2005/05/09 00:36:58  steve
 *  Force part base out of bounds if index is invalid.
 *
 * Revision 1.4  2005/05/08 23:40:14  steve
 *  Add support for variable part select.
 *
 * Revision 1.3  2005/01/09 20:11:16  steve
 *  Add the .part/pv node and related functionality.
 *
 * Revision 1.2  2004/12/29 23:44:39  steve
 *  Fix missing output propagation of part node.
 *
 * Revision 1.1  2004/12/11 02:31:30  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 */

