/*
 * Copyright (c) 2001-2004 Stephen Williams (steve@icarus.com)
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
#ident "$Id: logic.cc,v 1.17 2005/01/29 17:52:06 steve Exp $"
#endif

# include  "logic.h"
# include  "compile.h"
# include  "bufif.h"
# include  "npmos.h"
# include  "statistics.h"
# include  <string.h>
# include  <assert.h>
# include  <stdlib.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif


/*
 *   Implementation of the table functor, which provides logic with up
 *   to 4 inputs.
 */

table_functor_s::table_functor_s(truth_t t)
: table(t)
{
      count_functors_table += 1;
}

table_functor_s::~table_functor_s()
{
}

/*
 * WARNING: This function assumes that the table generator encodes the
 * values 0/1/x/z the same as the vvp_bit4_t enumeration values.
 */
void table_functor_s::recv_vec4(vvp_net_ptr_t ptr, vvp_vector4_t val)
{
      input_[ptr.port()] = val;

      vvp_vector4_t result (val.size());

      for (unsigned idx = 0 ;  idx < val.size() ;  idx += 1) {

	    unsigned lookup = 0;
	    for (unsigned pdx = 4 ;  pdx > 0 ;  pdx -= 1) {
		  lookup <<= 2;
		  if (idx < input_[pdx-1].size())
			lookup |= input_[pdx-1].value(idx);
	    }

	    unsigned off = lookup / 4;
	    unsigned shift = lookup % 4 * 2;

	    unsigned bit_val = table[off] >> shift;
	    bit_val &= 3;
	    result.set_bit(idx, (vvp_bit4_t)bit_val);
      }

      vvp_send_vec4(ptr.ptr()->out, result);
}

vvp_fun_and::vvp_fun_and()
{
}

vvp_fun_and::~vvp_fun_and()
{
}

void vvp_fun_and::recv_vec4(vvp_net_ptr_t ptr, vvp_vector4_t bit)
{
      input_[ptr.port()] = bit;

      vvp_vector4_t result (bit);

      for (unsigned idx = 0 ;  idx < result.size() ;  idx += 1) {
	    vvp_bit4_t bitbit = BIT4_1;
	    for (unsigned pdx = 0 ;  pdx < 4 ;  pdx += 1) {
		  if (input_[pdx].size() < idx) {
			bitbit = BIT4_X;
			break;
		  }

		  bitbit = bitbit & input_[pdx].value(idx);
	    }

	    result.set_bit(idx, bitbit);
      }

      vvp_send_vec4(ptr.ptr()->out, result);
}

vvp_fun_buf::vvp_fun_buf()
{
      count_functors_table += 1;
}

vvp_fun_buf::~vvp_fun_buf()
{
}

void vvp_fun_buf::recv_vec4(vvp_net_ptr_t ptr, vvp_vector4_t bit)
{
      if (ptr.port() != 0)
	    return;

      for (unsigned idx = 0 ;  idx < bit.size() ;  idx += 1) {
	    if (bit.value(idx) == BIT4_Z)
		  bit.set_bit(idx, BIT4_X);
      }

      vvp_send_vec4(ptr.ptr()->out, bit);
}

vvp_fun_bufz::vvp_fun_bufz()
{
      count_functors_table += 1;
}

vvp_fun_bufz::~vvp_fun_bufz()
{
}

void vvp_fun_bufz::recv_vec4(vvp_net_ptr_t ptr, vvp_vector4_t bit)
{
      if (ptr.port() != 0)
	    return;

      vvp_send_vec4(ptr.ptr()->out, bit);
}

/*
 * The parser calls this function to create a logic functor. I allocate a
 * functor, and map the name to the vvp_ipoint_t address for the
 * functor. Also resolve the inputs to the functor.
 */

void compile_functor(char*label, char*type,
		     vvp_delay_t delay, unsigned ostr0, unsigned ostr1,
		     unsigned argc, struct symb_s*argv)
{
      vvp_net_fun_t* obj = 0;

      if (strcmp(type, "OR") == 0) {
	    obj = new table_functor_s(ft_OR);

      } else if (strcmp(type, "AND") == 0) {
	    obj = new vvp_fun_and();

      } else if (strcmp(type, "BUF") == 0) {
	    obj = new vvp_fun_buf();
#if 0
      } else if (strcmp(type, "BUFIF0") == 0) {
	    obj = new vvp_bufif_s(true,false, ostr0, ostr1);

      } else if (strcmp(type, "BUFIF1") == 0) {
	    obj = new vvp_bufif_s(false,false, ostr0, ostr1);
#endif
      } else if (strcmp(type, "BUFZ") == 0) {
	    obj = new vvp_fun_bufz();
#if 0
      } else if (strcmp(type, "PMOS") == 0) {
	    obj = new vvp_pmos_s;

      } else if (strcmp(type, "NMOS") == 0) {
	    obj= new vvp_nmos_s;

      } else if (strcmp(type, "RPMOS") == 0) {
	    obj = new vvp_rpmos_s;

      } else if (strcmp(type, "RNMOS") == 0) {
	    obj = new vvp_rnmos_s;
#endif
      } else if (strcmp(type, "MUXX") == 0) {
	    obj = new table_functor_s(ft_MUXX);

      } else if (strcmp(type, "MUXZ") == 0) {
	    obj = new table_functor_s(ft_MUXZ);

      } else if (strcmp(type, "EEQ") == 0) {
	    obj = new table_functor_s(ft_EEQ);

      } else if (strcmp(type, "NAND") == 0) {
	    obj = new table_functor_s(ft_NAND);

      } else if (strcmp(type, "NOR") == 0) {
	    obj = new table_functor_s(ft_NOR);

      } else if (strcmp(type, "NOT") == 0) {
	    obj = new table_functor_s(ft_NOT);
#if 0
      } else if (strcmp(type, "NOTIF0") == 0) {
	    obj = new vvp_bufif_s(true,true, ostr0, ostr1);

      } else if (strcmp(type, "NOTIF1") == 0) {
	    obj = new vvp_bufif_s(false,true, ostr0, ostr1);
#endif
      } else if (strcmp(type, "XNOR") == 0) {
	    obj = new table_functor_s(ft_XNOR);

      } else if (strcmp(type, "XOR") == 0) {
	    obj = new table_functor_s(ft_XOR);

      } else {
	    yyerror("invalid functor type.");
	    free(type);
	    free(argv);
	    free(label);
	    return;
      }

      free(type);

      assert(argc <= 4);
      vvp_net_t*net = new vvp_net_t;
      net->fun = obj;

      define_functor_symbol(label, net);
      free(label);

      inputs_connect(net, argc, argv);
      free(argv);
}


/*
 * $Log: logic.cc,v $
 * Revision 1.17  2005/01/29 17:52:06  steve
 *  move AND to buitin instead of table.
 *
 * Revision 1.16  2004/12/31 05:56:36  steve
 *  Add specific BUFZ functor.
 *
 * Revision 1.15  2004/12/29 23:45:13  steve
 *  Add the part concatenation node (.concat).
 *
 *  Add a vvp_event_anyedge class to handle the special
 *  case of .event statements of edge type. This also
 *  frees the posedge/negedge types to handle all 4 inputs.
 *
 *  Implement table functor recv_vec4 method to receive
 *  and process vectors.
 *
 * Revision 1.14  2004/12/11 02:31:29  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 */

