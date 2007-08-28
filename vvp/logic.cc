/*
 * Copyright (c) 2001-2007 Stephen Williams (steve@icarus.com)
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
#ident "$Id: logic.cc,v 1.38 2007/02/12 05:08:27 steve Exp $"
#endif

# include  "logic.h"
# include  "compile.h"
# include  "bufif.h"
# include  "npmos.h"
# include  "schedule.h"
# include  "delay.h"
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
void table_functor_s::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&val)
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

vvp_fun_boolean_::vvp_fun_boolean_(unsigned wid)
{
      net_ = 0;
      for (unsigned idx = 0 ;  idx < 4 ;  idx += 1)
	    input_[idx] = vvp_vector4_t(wid);
}

vvp_fun_boolean_::~vvp_fun_boolean_()
{
}

void vvp_fun_boolean_::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit)
{
      unsigned port = ptr.port();
      if (input_[port] .eeq( bit ))
	    return;

      input_[ptr.port()] = bit;
      if (net_ == 0) {
	    net_ = ptr.ptr();
	    schedule_generic(this, 0, false);
      }
}

void vvp_fun_boolean_::recv_vec4_pv(vvp_net_ptr_t ptr, const vvp_vector4_t&bit,
				    unsigned base, unsigned wid, unsigned vwid)
{
      unsigned port = ptr.port();

      assert(bit.size() == wid);
      assert(base + bit.size() <= vwid);

      if (input_[port].subvalue(base, wid) .eeq( bit ))
	    return;

      input_[port] .set_vec(base, bit);
      if (net_ == 0) {
	    net_ = ptr.ptr();
	    schedule_generic(this, 0, false);
      }
}

vvp_fun_and::vvp_fun_and(unsigned wid, bool invert)
: vvp_fun_boolean_(wid), invert_(invert)
{
}

vvp_fun_and::~vvp_fun_and()
{
}

void vvp_fun_and::run_run()
{
      vvp_net_t*ptr = net_;
      net_ = 0;

      vvp_vector4_t result (input_[0]);

      for (unsigned idx = 0 ;  idx < result.size() ;  idx += 1) {
	    vvp_bit4_t bitbit = result.value(idx);
	    for (unsigned pdx = 1 ;  pdx < 4 ;  pdx += 1) {
		  if (input_[pdx].size() < idx) {
			bitbit = BIT4_X;
			break;
		  }

		  bitbit = bitbit & input_[pdx].value(idx);
	    }

	    if (invert_)
		  bitbit = ~bitbit;
	    result.set_bit(idx, bitbit);
      }

      vvp_send_vec4(ptr->out, result);
}

vvp_fun_eeq::vvp_fun_eeq(unsigned wid, bool invert)
: vvp_fun_boolean_(wid), invert_(invert)
{
}

vvp_fun_eeq::~vvp_fun_eeq()
{
}

void vvp_fun_eeq::run_run()
{
      vvp_net_t*ptr = net_;
      net_ = 0;

      vvp_vector4_t result (input_[0]);

      for (unsigned idx = 0 ;  idx < result.size() ;  idx += 1) {
	    vvp_bit4_t bitbit = result.value(idx);
	    bitbit = (bitbit == input_[1].value(idx))? BIT4_1 : BIT4_0;
	    if (invert_)
		  bitbit = ~bitbit;

	    result.set_bit(idx, bitbit);
      }

      vvp_send_vec4(ptr->out, result);
}

vvp_fun_buf::vvp_fun_buf()
{
      net_ = 0;
      count_functors_table += 1;
}

vvp_fun_buf::~vvp_fun_buf()
{
}

/*
 * The buf functor is very simple--change the z bits to x bits in the
 * vector it passes, and propagate the result.
 */
void vvp_fun_buf::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit)
{
      if (ptr.port() != 0)
	    return;

      if (input_ .eeq( bit ))
	    return;

      input_ = bit;

      if (net_ == 0) {
	    net_ = ptr.ptr();
	    schedule_generic(this, 0, false);
      }
}

void vvp_fun_buf::run_run()
{
      vvp_net_t*ptr = net_;
      net_ = 0;

      vvp_vector4_t tmp (input_);
      tmp.change_z2x();
      vvp_send_vec4(ptr->out, tmp);
}

vvp_fun_bufz::vvp_fun_bufz()
{
      count_functors_table += 1;
}

vvp_fun_bufz::~vvp_fun_bufz()
{
}

/*
 * The bufz is similar to the buf device, except that it does not
 * bother translating z bits to x.
 */
void vvp_fun_bufz::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit)
{
      if (ptr.port() != 0)
	    return;

      vvp_send_vec4(ptr.ptr()->out, bit);
}

void vvp_fun_bufz::recv_real(vvp_net_ptr_t ptr, double bit)
{
      if (ptr.port() != 0)
	    return;

      vvp_send_real(ptr.ptr()->out, bit);
}

vvp_fun_muxr::vvp_fun_muxr()
: a_(0.0), b_(0.0)
{
      count_functors_table += 1;
      select_ = 2;
}

vvp_fun_muxr::~vvp_fun_muxr()
{
}

void vvp_fun_muxr::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit)
{
	/* The real valued mux can only take in the select as a
	   vector4_t. the muxed data is rea. */
      if (ptr.port() != 2)
	    return;

      assert(bit.size() == 1);

      switch (bit.value(0)) {
	  case BIT4_0:
	    select_ = 0;
	    break;
	  case BIT4_1:
	    select_ = 1;
	    break;
	  default:
	    select_ = 2;
      }

      switch (select_) {
	  case 0:
	    vvp_send_real(ptr.ptr()->out, a_);
	    break;
	  case 1:
	    vvp_send_real(ptr.ptr()->out, b_);
	    break;
	  default:
	    if (a_ == b_) {
		  vvp_send_real(ptr.ptr()->out, a_);
	    } else {
		    // Should send NaN?
		  vvp_send_real(ptr.ptr()->out, 0.0);
	    }
	    break;
      }
}

void vvp_fun_muxr::recv_real(vvp_net_ptr_t ptr, double bit)
{
      switch (ptr.port()) {
	  case 0:
	    if (a_ == bit)
		  break;

	    a_ = bit;
	    if (select_ == 0)
		  vvp_send_real(ptr.ptr()->out, a_);
	    break;

	  case 1:
	    if (b_ == bit)
		  break;

	    b_ = bit;
	    if (select_ == 1)
		  vvp_send_real(ptr.ptr()->out, b_);
	    break;

	  default:
	    assert(0);
      }
}

vvp_fun_muxz::vvp_fun_muxz(unsigned wid)
: a_(wid), b_(wid)
{
      count_functors_table += 1;
      select_ = 2;
      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    a_.set_bit(idx, BIT4_X);
	    b_.set_bit(idx, BIT4_X);
      }
}

vvp_fun_muxz::~vvp_fun_muxz()
{
}

void vvp_fun_muxz::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit)
{
      switch (ptr.port()) {
	  case 0:
	    a_ = bit;
	    break;
	  case 1:
	    b_ = bit;
	    break;
	  case 2:
	    assert(bit.size() == 1);
	    switch (bit.value(0)) {
		case BIT4_0:
		  select_ = 0;
		  break;
		case BIT4_1:
		  select_ = 1;
		  break;
		default:
		  select_ = 2;
	    }
	    break;
	  default:
	    return;
      }

      switch (select_) {
	  case 0:
	    vvp_send_vec4(ptr.ptr()->out, a_);
	    break;
	  case 1:
	    vvp_send_vec4(ptr.ptr()->out, b_);
	    break;
	  default:
	      {
		    unsigned min_size = a_.size();
		    unsigned max_size = a_.size();
		    if (b_.size() < min_size)
			  min_size = b_.size();
		    if (b_.size() > max_size)
			  max_size = b_.size();

		    vvp_vector4_t res (max_size);

		    for (unsigned idx = 0 ;  idx < min_size ;  idx += 1) {
			  if (a_.value(idx) == b_.value(idx))
				res.set_bit(idx, a_.value(idx));
			  else
				res.set_bit(idx, BIT4_X);
		    }

		    for (unsigned idx = min_size ;  idx < max_size ;  idx += 1)
			  res.set_bit(idx, BIT4_X);

		    vvp_send_vec4(ptr.ptr()->out, res);
	      }
	    break;
      }
}

vvp_fun_not::vvp_fun_not()
{
      net_ = 0;
      count_functors_table += 1;
}

vvp_fun_not::~vvp_fun_not()
{
}

/*
 * The buf functor is very simple--change the z bits to x bits in the
 * vector it passes, and propagate the result.
 */
void vvp_fun_not::recv_vec4(vvp_net_ptr_t ptr, const vvp_vector4_t&bit)
{
      if (ptr.port() != 0)
	    return;

      if (input_ .eeq( bit ))
	    return;

      input_ = bit;
      if (net_ == 0) {
	    net_ = ptr.ptr();
	    schedule_generic(this, 0, false);
      }
}

void vvp_fun_not::run_run()
{
      vvp_net_t*ptr = net_;
      net_ = 0;

      vvp_vector4_t result (input_);

      for (unsigned idx = 0 ;  idx < result.size() ;  idx += 1) {
	    vvp_bit4_t bitbit = ~ result.value(idx);
	    result.set_bit(idx, bitbit);
      }

      vvp_send_vec4(ptr->out, result);
}

vvp_fun_or::vvp_fun_or(unsigned wid, bool invert)
: vvp_fun_boolean_(wid), invert_(invert)
{
}

vvp_fun_or::~vvp_fun_or()
{
}

void vvp_fun_or::run_run()
{
      vvp_net_t*ptr = net_;
      net_ = 0;

      vvp_vector4_t result (input_[0]);

      for (unsigned idx = 0 ;  idx < result.size() ;  idx += 1) {
	    vvp_bit4_t bitbit = result.value(idx);
	    for (unsigned pdx = 1 ;  pdx < 4 ;  pdx += 1) {
		  if (input_[pdx].size() < idx) {
			bitbit = BIT4_X;
			break;
		  }

		  bitbit = bitbit | input_[pdx].value(idx);
	    }

	    if (invert_)
		  bitbit = ~bitbit;
	    result.set_bit(idx, bitbit);
      }

      vvp_send_vec4(ptr->out, result);
}

vvp_fun_xor::vvp_fun_xor(unsigned wid, bool invert)
: vvp_fun_boolean_(wid), invert_(invert)
{
}

vvp_fun_xor::~vvp_fun_xor()
{
}

void vvp_fun_xor::run_run()
{
      vvp_net_t*ptr = net_;
      net_ = 0;

      vvp_vector4_t result (input_[0]);

      for (unsigned idx = 0 ;  idx < result.size() ;  idx += 1) {
	    vvp_bit4_t bitbit = result.value(idx);
	    for (unsigned pdx = 1 ;  pdx < 4 ;  pdx += 1) {
		  if (input_[pdx].size() < idx) {
			bitbit = BIT4_X;
			break;
		  }

		  bitbit = bitbit ^ input_[pdx].value(idx);
	    }

	    if (invert_)
		  bitbit = ~bitbit;
	    result.set_bit(idx, bitbit);
      }

      vvp_send_vec4(ptr->out, result);
}

/*
 * The parser calls this function to create a logic functor. I allocate a
 * functor, and map the name to the vvp_ipoint_t address for the
 * functor. Also resolve the inputs to the functor.
 */

void compile_functor(char*label, char*type, unsigned width,
		     unsigned ostr0, unsigned ostr1,
		     unsigned argc, struct symb_s*argv)
{
      vvp_net_fun_t* obj = 0;
      bool strength_aware = false;

      if (strcmp(type, "OR") == 0) {
	    obj = new vvp_fun_or(width, false);

      } else if (strcmp(type, "AND") == 0) {
	    obj = new vvp_fun_and(width, false);

      } else if (strcmp(type, "BUF") == 0) {
	    obj = new vvp_fun_buf();

      } else if (strcmp(type, "BUFIF0") == 0) {
	    obj = new vvp_fun_bufif(true,false, ostr0, ostr1);
	    strength_aware = true;

      } else if (strcmp(type, "BUFIF1") == 0) {
	    obj = new vvp_fun_bufif(false,false, ostr0, ostr1);
	    strength_aware = true;

      } else if (strcmp(type, "NAND") == 0) {
	    obj = new vvp_fun_and(width, true);

      } else if (strcmp(type, "NOR") == 0) {
	    obj = new vvp_fun_or(width, true);

      } else if (strcmp(type, "NOTIF0") == 0) {
	    obj = new vvp_fun_bufif(true,true, ostr0, ostr1);
	    strength_aware = true;

      } else if (strcmp(type, "NOTIF1") == 0) {
	    obj = new vvp_fun_bufif(false,true, ostr0, ostr1);
	    strength_aware = true;

      } else if (strcmp(type, "BUFZ") == 0) {
	    obj = new vvp_fun_bufz();

      } else if (strcmp(type, "MUXR") == 0) {
	    obj = new vvp_fun_muxr;

      } else if (strcmp(type, "MUXX") == 0) {
	    obj = new table_functor_s(ft_MUXX);

      } else if (strcmp(type, "MUXZ") == 0) {
	    obj = new vvp_fun_muxz(width);

      } else if (strcmp(type, "NMOS") == 0) {
	    obj = new vvp_fun_pmos(true);

      } else if (strcmp(type, "PMOS") == 0) {
	    obj = new vvp_fun_pmos(false);

      } else if (strcmp(type, "RNMOS") == 0) {
	    obj = new vvp_fun_rpmos(true);

      } else if (strcmp(type, "RPMOS") == 0) {
	    obj = new vvp_fun_rpmos(false);

      } else if (strcmp(type, "EEQ") == 0) {
	    obj = new vvp_fun_eeq(width, false);

      } else if (strcmp(type, "NOT") == 0) {
	    obj = new vvp_fun_not();

      } else if (strcmp(type, "XNOR") == 0) {
	    obj = new vvp_fun_xor(width, true);

      } else if (strcmp(type, "XOR") == 0) {
	    obj = new vvp_fun_xor(width, false);

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

      inputs_connect(net, argc, argv);
      free(argv);

	/* If both the strengths are the default strong drive, then
	   there is no need for a specialized driver. Attach the label
	   to this node and we are finished. */
      if (strength_aware || ostr0 == 6 && ostr1 == 6) {
	    define_functor_symbol(label, net);
	    free(label);
	    return;
      }

      vvp_net_t*net_drv = new vvp_net_t;
      vvp_net_fun_t*obj_drv = new vvp_fun_drive(BIT4_X, ostr0, ostr1);
      net_drv->fun = obj_drv;

	/* Point the gate to the drive node. */
      net->out = vvp_net_ptr_t(net_drv, 0);

      define_functor_symbol(label, net_drv);
      free(label);
}
