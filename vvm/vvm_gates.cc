/*
 * Copyright (c) 1999-2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: vvm_gates.cc,v 1.15 2000/05/09 21:16:35 steve Exp $"
#endif

# include  "vvm_gates.h"

vvm_out_event::vvm_out_event(vpip_bit_t v, vvm_nexus::drive_t*o)
: output_(o), val_(v)
{
}

vvm_out_event::~vvm_out_event()
{
}

void vvm_out_event::event_function()
{
      output_->set_value(val_);
}

vvm_1bit_out::vvm_1bit_out(unsigned d)
: delay_(d)
{
      drive0_ = St0;
      drive1_ = St1;
      driveX_ = StX;
      driveZ_ = HiZ;
}

vvm_1bit_out::~vvm_1bit_out()
{
}

void vvm_1bit_out::drive0(vpip_bit_t v)
{
      drive0_ = v;
}

void vvm_1bit_out::drive1(vpip_bit_t v)
{
      drive1_ = v;
}

void vvm_1bit_out::driveX(vpip_bit_t v)
{
      driveX_ = v;
}

void vvm_1bit_out::driveZ(vpip_bit_t v)
{
      driveZ_ = v;
}

void vvm_1bit_out::output(vpip_bit_t val)
{
      if (B_IS0(val))
	    val = drive0_;
      else if (B_IS1(val))
	    val = drive1_;
      else if (B_ISZ(val))
	    val = driveZ_;
      else
	    val = driveX_;

      vvm_event*ev = new vvm_out_event(val, this);
      ev -> schedule(delay_);
}


vpip_bit_t compute_and(const vpip_bit_t*inp, unsigned count)
{
      vpip_bit_t outval = inp[0];
      for (unsigned i = 1 ;  i < count ;  i += 1)
	    outval = B_AND(outval, inp[i]);
      return outval;
}

vpip_bit_t compute_or(const vpip_bit_t*inp, unsigned count)
{
      vpip_bit_t outval = inp[0];
      for (unsigned i = 1 ;  i < count ;  i += 1)
	    outval = B_OR(outval, inp[i]);
      return outval;
}

vpip_bit_t compute_nor(const vpip_bit_t*inp, unsigned count)
{
      vpip_bit_t outval = inp[0];
      for (unsigned i = 1 ;  i < count ;  i += 1)
	    outval = B_OR(outval, inp[i]);
      return B_NOT(outval);
}

vpip_bit_t compute_xor(const vpip_bit_t*inp, unsigned count)
{
      vpip_bit_t outval = inp[0];
      for (unsigned i = 1; i < count; i++)
	    outval = B_XOR(outval, inp[i]);
      return outval;
}

vpip_bit_t compute_nand(const vpip_bit_t*inp, unsigned count)
{
      return B_NOT(compute_and(inp,count));
}

vpip_bit_t compute_xnor(const vpip_bit_t*inp, unsigned count)
{
      return B_NOT(compute_xor(inp,count));
}

vvm_and2::vvm_and2(unsigned long d)
: vvm_1bit_out(d)
{
}

vvm_and2::~vvm_and2()
{
}

void vvm_and2::init_I(unsigned idx, vpip_bit_t val)
{
      assert(idx < 2);
      input_[idx] = val;
}

void vvm_and2::start()
{
      output(B_AND(input_[0], input_[1]));
}

void vvm_and2::take_value(unsigned key, vpip_bit_t val)
{
      if (input_[key] == val)
	    return;

      input_[key] = val;
      output(B_AND(input_[0], input_[1]));
}


vvm_buf::vvm_buf(unsigned long d)
: vvm_1bit_out(d)
{
}

vvm_buf::~vvm_buf()
{
}

void vvm_buf::init_I(unsigned, vpip_bit_t)
{
}

void vvm_buf::take_value(unsigned, vpip_bit_t val)
{
      vpip_bit_t outval = val;
      if (B_ISXZ(val))
	    outval = StX;
      else if (B_IS1(val))
	    outval = St1;
      else
	    outval = St0;
      output(outval);
}

vvm_bufif0::vvm_bufif0(unsigned long d)
: vvm_1bit_out(d)
{
      input_[0] = StX;
      input_[1] = StX;
}

vvm_bufif0::~vvm_bufif0()
{
}

void vvm_bufif0::init_I(unsigned key, vpip_bit_t val)
{
      assert(key < 2);
      input_[key] = val;
}

void vvm_bufif0::take_value(unsigned key, vpip_bit_t val)
{
      if (input_[key] == val) return;
      input_[key] = val;

      if (! B_IS0(input_[1]))
	    output(HiZ);
      else if (B_ISXZ(input_[0]))
	    output(StX);
      else if (B_IS1(input_[0]))
	    output(St1);
      else
	    output(St0);
}

vvm_bufif1::vvm_bufif1(unsigned long d)
: vvm_1bit_out(d)
{
      input_[0] = StX;
      input_[1] = StX;
}

vvm_bufif1::~vvm_bufif1()
{
}

void vvm_bufif1::init_I(unsigned key, vpip_bit_t val)
{
      assert(key < 2);
      input_[key] = val;
}

void vvm_bufif1::take_value(unsigned key, vpip_bit_t val)
{
      if (input_[key] == val) return;
      input_[key] = val;

      if (! B_IS1(input_[1]))
	    output(HiZ);
      else if (B_ISXZ(input_[0]))
	    output(StX);
      else if (B_IS1(input_[0]))
	    output(St1);
      else
	    output(St0);
}

vvm_bufz::vvm_bufz()
: vvm_1bit_out(0)
{
}

vvm_bufz::~vvm_bufz()
{
}

void vvm_bufz::init_I(unsigned, vpip_bit_t)
{
}

void vvm_bufz::take_value(unsigned, vpip_bit_t val)
{
      output(val);
}

vvm_eeq::vvm_eeq(unsigned long d)
: vvm_1bit_out(d)
{
}

vvm_eeq::~vvm_eeq()
{
}

void vvm_eeq::init_I(unsigned idx, vpip_bit_t val)
{
      input_[idx] = val;
}

void vvm_eeq::start()
{
      output(compute_());
}

void vvm_eeq::take_value(unsigned key, vpip_bit_t val)
{
      if (input_[key] == val)
	    return;
      input_[key] = val;
      output(compute_());
}

vpip_bit_t vvm_eeq::compute_() const
{
      vpip_bit_t outval = St0;
      if (B_EQ(input_[0], input_[1]))
	    outval = St1;
      return outval;
}

vvm_nor2::vvm_nor2(unsigned long d)
: vvm_1bit_out(d)
{
}

vvm_nor2::~vvm_nor2()
{
}

void vvm_nor2::init_I(unsigned idx, vpip_bit_t val)
{
      assert(idx < 2);
      input_[idx] = val;
}

void vvm_nor2::start()
{
      output(B_NOT(B_OR(input_[0], input_[1])));
}

void vvm_nor2::take_value(unsigned key, vpip_bit_t val)
{
      if (input_[key] == val)
	    return;

      input_[key] = val;
      output(B_NOT(B_OR(input_[0], input_[1])));
}

vvm_not::vvm_not(unsigned long d)
: vvm_1bit_out(d)
{
}

vvm_not::~vvm_not()
{
}

void vvm_not::init_I(unsigned, vpip_bit_t)
{
}

void vvm_not::start()
{
}

void vvm_not::take_value(unsigned, vpip_bit_t val)
{
      output(B_NOT(val));
}


/*
 * $Log: vvm_gates.cc,v $
 * Revision 1.15  2000/05/09 21:16:35  steve
 *  Give strengths to logic and bufz devices.
 *
 * Revision 1.14  2000/05/08 05:27:32  steve
 *  Restore vvm_bufz to working condition.
 *
 * Revision 1.13  2000/04/23 21:15:07  steve
 *  Emit code for the bufif devices.
 *
 * Revision 1.12  2000/03/22 04:26:41  steve
 *  Replace the vpip_bit_t with a typedef and
 *  define values for all the different bit
 *  values, including strengths.
 *
 * Revision 1.11  2000/03/18 02:26:02  steve
 *  Update bufz to nexus style.
 *
 * Revision 1.10  2000/03/18 01:27:00  steve
 *  Generate references into a table of nexus objects instead of
 *  generating lots of isolated nexus objects. Easier on linkers
 *  and compilers,
 *
 *  Add missing nexus support for l-value bit selects,
 *
 *  Detemplatize the vvm_mux type.
 *
 *  Fix up the vvm_nexus destructor to disconnect from drivers.
 *
 * Revision 1.9  2000/03/17 19:24:00  steve
 *  nor2 and and2 optimized gates.
 *
 * Revision 1.8  2000/03/17 03:36:07  steve
 *  Remove some useless template parameters.
 *
 * Revision 1.7  2000/03/16 19:03:04  steve
 *  Revise the VVM backend to use nexus objects so that
 *  drivers and resolution functions can be used, and
 *  the t-vvm module doesn't need to write a zillion
 *  output functions.
 *
 * Revision 1.6  2000/02/24 01:56:28  steve
 *  change not to v_not.
 *
 * Revision 1.5  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.4  1999/12/12 19:47:54  steve
 *  Remove the useless vvm_simulation class.
 *
 * Revision 1.3  1999/12/02 04:54:11  steve
 *  Handle mux sel of X, if inputs are equal.
 *
 * Revision 1.2  1999/11/24 04:38:49  steve
 *  LT and GT fixes from Eric Aardoom.
 *
 * Revision 1.1  1999/11/22 00:30:52  steve
 *  Detemplate some and, or and nor methods.
 *
 */

