/*
 * Copyright (c) 2005 Stephen Williams (steve@icarus.com)
 *
 * (This is a rewrite of code that was ...
 * Copyright (c) 2001 Stephan Boettcher <stephan@nevis.columbia.edu>)
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
#ident "$Id: udp.cc,v 1.28 2005/04/03 05:45:51 steve Exp $"
#endif

#include "udp.h"
#include "schedule.h"
#include "symbols.h"
#include "compile.h"
#include <assert.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static symbol_table_t udp_table;

struct vvp_udp_s *udp_find(const char *label)
{
      symbol_value_t v = sym_get_value(udp_table, label);
      return (struct vvp_udp_s *)v.ptr;
}

vvp_udp_s::vvp_udp_s(char*label, char*name, unsigned ports, bool sequ)
{
      assert(!sequ); // XXXX sequential UDPs not supported yet.

      if (!udp_table)
	    udp_table = new_symbol_table();

      assert(!udp_find(label));

      symbol_value_t v;
      v.ptr = this;
      sym_set_value(udp_table, label, v);

      name_ = name;
      ports_ = ports;
      levels0_ = 0;
      levels1_ = 0;
      nlevels0_ = 0;
      nlevels1_ = 0;
}

vvp_udp_s::~vvp_udp_s()
{
      if (levels0_) delete[] levels0_;
      if (levels1_) delete[] levels1_;
}

unsigned vvp_udp_s::port_count() const
{
      return ports_;
}

vvp_bit4_t vvp_udp_s::test_levels(const udp_levels_table&cur)
{
      for (unsigned idx = 0 ;  idx < nlevels0_ ;  idx += 1) {
	    if (cur.mask0 != levels0_[idx].mask0)
		  continue;
	    if (cur.mask1 != levels0_[idx].mask1)
		  continue;
	    if (cur.maskx != levels0_[idx].maskx)
		  continue;

	    return BIT4_0;
      }

      for (unsigned idx = 0 ;  idx < nlevels1_ ;  idx += 1) {
	    if (cur.mask0 != levels1_[idx].mask0)
		  continue;
	    if (cur.mask1 != levels1_[idx].mask1)
		  continue;
	    if (cur.maskx != levels1_[idx].maskx)
		  continue;

	    return BIT4_1;
      }

      return BIT4_X;
}

void vvp_udp_s::compile_table(char**tab)
{
      unsigned nrows0 = 0, nrows1 = 0;

	/* First run through the table to figure out the number of
	   rows I need for each kind of table. */
      for (unsigned idx = 0 ;  tab[idx] ;  idx += 1) {
	    assert(strlen(tab[idx]) == ports_ + 1);
	    switch (tab[idx][ports_]) {
		case '0':
		  nrows0 += 1;
		  break;
		case '1':
		  nrows1 += 1;
		  break;
		case 'x':
		  break;
		default:
		  assert(0);
	    }
      }

      nlevels0_ = nrows0;
      levels0_ = new udp_levels_table[nlevels0_];

      nlevels1_ = nrows1;
      levels1_ = new udp_levels_table[nlevels1_];

      nrows0 = 0;
      nrows1 = 0;
      for (unsigned idx = 0 ;  tab[idx] ;  idx += 1) {
	    struct udp_levels_table cur;
	    cur.mask0 = 0;
	    cur.mask1 = 0;
	    cur.maskx = 0;
	    assert(ports_ <= sizeof(cur.mask0));
	    for (unsigned pp = 0 ;  pp < ports_ ;  pp += 1) {
		  unsigned long mask_bit = 1UL << pp;
		  switch (tab[idx][pp]) {
		      case '0':
			cur.mask0 |= mask_bit;
			break;
		      case '1':
			cur.mask1 |= mask_bit;
			break;
		      case 'x':
			cur.maskx |= mask_bit;
			break;
		      default:
			assert(0);
		  }
	    }

	    switch (tab[idx][ports_]) {
		case '0':
		  levels0_[nrows0++] = cur;
		  break;
		case '1':
		  levels1_[nrows1++] = cur;
		  break;
		default:
		  break;
	    }
      }

      assert(nrows0 == nlevels0_);
      assert(nrows1 == nlevels1_);
}

vvp_udp_fun_core::vvp_udp_fun_core(vvp_net_t*net,
				   vvp_udp_s*def,
				   vvp_delay_t*del)
: vvp_wide_fun_core(net, def->port_count())
{
      def_ = def;
      delay_ = del;
      cur_out_ = BIT4_X;
	// Assume initially that all the inputs are 1'bx
      current_.mask0 = 0;
      current_.mask1 = 0;
      current_.maskx = ~ ((-1UL) << port_count());
}

vvp_udp_fun_core::~vvp_udp_fun_core()
{
}

void vvp_udp_fun_core::recv_vec4_from_inputs(unsigned port)
{
	/* For now, assume udps are 1-bit wide. */
      assert(value(port).size() == 1);

      unsigned long mask = 1UL << port;

      switch (value(port).value(0)) {

	  case BIT4_0:
	    current_.mask0 |= mask;
	    current_.mask1 &= ~mask;
	    current_.maskx &= ~mask;
	    break;
	  case BIT4_1:
	    current_.mask0 &= ~mask;
	    current_.mask1 |= mask;
	    current_.maskx &= ~mask;
	    break;
	  default:
	    current_.mask0 &= ~mask;
	    current_.mask1 &= ~mask;
	    current_.maskx |= mask;
	    break;
      }

      vvp_bit4_t out_bit = def_->test_levels(current_);
      vvp_vector4_t out (1);
      out.set_bit(0, out_bit);

      if (delay_)
	    propagate_vec4(out, delay_->get_delay(cur_out_, out_bit));
      else
	    propagate_vec4(out);

      cur_out_ = out_bit;
}


/*
 * This function is called by the parser in response to a .udp
 * node. We create the nodes needed to integrate the UDP into the
 * netlist. The definition should be parsed already.
 */
void compile_udp_functor(char*label, char*type,
			 vvp_delay_t*delay,
			 unsigned argc, struct symb_s*argv)
{
      struct vvp_udp_s *def = udp_find(type);
      assert(def);
      free(type);

      vvp_net_t*ptr = new vvp_net_t;
      vvp_udp_fun_core*core = new vvp_udp_fun_core(ptr, def, delay);
      ptr->fun = core;
      define_functor_symbol(label, ptr);
      free(label);

      wide_inputs_connect(core, argc, argv);
      free(argv);
}

/*
 * $Log: udp.cc,v $
 * Revision 1.28  2005/04/03 05:45:51  steve
 *  Rework the vvp_delay_t class.
 *
 * Revision 1.27  2005/04/01 06:02:45  steve
 *  Reimplement combinational UDPs.
 *
 */
