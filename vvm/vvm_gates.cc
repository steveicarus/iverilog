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
#ident "$Id: vvm_gates.cc,v 1.22 2001/07/25 03:10:50 steve Exp $"
#endif

# include "config.h"

# include  "vvm_gates.h"
# include  <stdlib.h>
# include  <typeinfo>
# include  <string>
# include  <map>
# include  <sys/time.h>
# include  <unistd.h>

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
      driveX_ = (drive1_&0xf0) | (drive0_&0x0f);
}

void vvm_1bit_out::drive1(vpip_bit_t v)
{
      drive1_ = v;
      driveX_ = (drive1_&0xf0) | (drive0_&0x0f);
}

void vvm_1bit_out::driveZ(vpip_bit_t v)
{
      driveZ_ = v;
}

void vvm_1bit_out::output(vpip_bit_t val)
{
      if (delay_) {
	    vvm_event*ev = new vvm_out_event(val, this);
	    ev -> schedule(delay_);
      } else {
	    set_value(val);
      }
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

vpip_bit_t reduce_strength(vpip_bit_t val)
{
    if (B_IS0(val)) {

	if ((val == Su0) ||
	    (val == St0))
	    return(Pu0);
	else if (val == Pu0)
	    return(We0);
	else if ((val == We0) ||
		 (val == La0))
	    return(Me0);
	else if ((val == Me0) ||
		 (val == Sm0))
	    return(Sm0);

    } else if (B_IS1(val)) {

	if ((val == Su1) ||
	    (val == St1))
	    return(Pu1);
	else if (val == Pu1)
	    return(We1);
	else if ((val == We1) ||
	     (val == La1))
	    return(Me1);
	else if ((val == Me1) ||
		 (val == Sm1))
	    return(Sm1);

    } else if (B_ISX(val)) {

	if ((val == SuX) ||
	    (val == StX))
	    return(PuX);
	else if (val == PuX)
	    return(WeX);
	else if ((val == WeX) ||
		 (val == LaX))
	    return(MeX);
	else if ((val == MeX) ||
		 (val == SmX))
	    return(SmX);

    }

    return(HiZ);
}

vvm_and::vvm_and(unsigned wid, unsigned long d)
: vvm_1bit_out(d), width_(wid)
{
      input_ = new vpip_bit_t[wid];
}

vvm_and::~vvm_and()
{
      delete [] input_;
}

void vvm_and::init_I(unsigned idx, vpip_bit_t val)
{
      input_[idx] = val;
}

void vvm_and::start()
{
      output(compute_and(input_,width_));
}

void vvm_and::take_value(unsigned key, vpip_bit_t val)
{
      assert(key < width_);
      if (input_[key] == val) return;
      input_[key] = val;
      output(compute_and(input_,width_));
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

      if ( B_IS1(input_[1]))
	    output(HiZ);
      else if ( B_ISX(input_[0]) ||
		B_ISZ(input_[0]))
	    output(StX);
      else if (B_IS0(input_[0])) {
	    if (B_IS0(input_[1])) {
	       output(St0);
	    } else {
	       output(StL);
	    }
      } else {
	  if (B_IS0(input_[1])) {
		output(St1);
	    } else {
		output(StH);
	    }
      }
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

      if ( B_IS0(input_[1]))
	    output(HiZ);
      else if ( B_ISX(input_[0]) ||
		B_ISZ(input_[0]))
	    output(StX);
      else if (B_IS0(input_[0])) {
	    if (B_IS1(input_[1])) {
	       output(St0);
	    } else {
	       output(StL);
	    }
      } else {
	  if (B_IS1(input_[1])) {
		output(St1);
	    } else {
		output(StH);
	    }
      }
}

vvm_bufz::vvm_bufz(unsigned delay)
: vvm_1bit_out(delay)
{
}

vvm_bufz::~vvm_bufz()
{
}

void vvm_bufz::init_I(unsigned, vpip_bit_t val)
{
      output(val);
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

vvm_nand::vvm_nand(unsigned wid, unsigned long d)
: vvm_1bit_out(d), width_(wid)
{
      input_ = new vpip_bit_t[wid];
}

vvm_nand::~vvm_nand()
{
      delete [] input_;
}

void vvm_nand::init_I(unsigned idx, vpip_bit_t val)
{
      input_[idx] = val;
}

void vvm_nand::start()
{
      output(compute_nand(input_,width_));
}

void vvm_nand::take_value(unsigned key, vpip_bit_t val)
{
      assert(key < width_);
      if (input_[key] == val) return;
      input_[key] = val;
      output(compute_nand(input_, width_));
}

vvm_nor::vvm_nor(unsigned wid, unsigned long d)
: vvm_1bit_out(d), width_(wid)
{
      input_ = new vpip_bit_t[wid];
}

vvm_nor::~vvm_nor()
{
      delete [] input_;
}

void vvm_nor::init_I(unsigned idx, vpip_bit_t val)
{
      input_[idx] = val;
}

void vvm_nor::start()
{
      output(compute_nor(input_,width_));
}

void vvm_nor::take_value(unsigned key, vpip_bit_t val)
{
      assert(key < width_);
      if (input_[key] == val) return;
      input_[key] = val;
      output(compute_nor(input_, width_));
}

vvm_or::vvm_or(unsigned wid, unsigned long d)
: vvm_1bit_out(d), width_(wid)
{
      input_ = new vpip_bit_t[wid];
}

vvm_or::~vvm_or()
{
      delete [] input_;
}

void vvm_or::init_I(unsigned idx, vpip_bit_t val)
{
      input_[idx] = val;
}

void vvm_or::start()
{
      output(compute_or(input_,width_));
}

void vvm_or::take_value(unsigned key, vpip_bit_t val)
{
      assert(key < width_);
      if (input_[key] == val) return;
      input_[key] = val;
      output(compute_or(input_, width_));
}

vvm_nmos::vvm_nmos(unsigned long d)
: vvm_1bit_out(d)
{
      input_[0] = StX;
      input_[1] = StX;
}

void vvm_nmos::init_I(unsigned key, vpip_bit_t val)
{
      assert(key < 2);
      input_[key] = val;
}

void vvm_nmos::take_value(unsigned key, vpip_bit_t val)
{
 
      if (input_[key] == val) {
	  return;
      }
      input_[key] = val;

      if ( B_IS0(input_[1])) {
	    output(HiZ);
      } else if (B_ISX(input_[0]))
	    output(StX);
      else if (B_ISZ(input_[0])) {
	    output(HiZ);
      } else if (B_IS0(input_[0])) {
	    if (B_IS1(input_[1])) {
		output(St0);
	    } else {
		output(StL);
	    }
      } else {
	    if (B_IS1(input_[1])) {
		output(St1);
	    } else {
		output(StH);
	    }
      }
}

vvm_rnmos::vvm_rnmos(unsigned long d)
: vvm_1bit_out(d)
{
      input_[0] = StX;
      input_[1] = StX;
}

void vvm_rnmos::init_I(unsigned key, vpip_bit_t val)
{
      assert(key < 2);
      input_[key] = val;
}

void vvm_rnmos::take_value(unsigned key, vpip_bit_t val)
{
      if (input_[key] == val) return;
      input_[key] = val;

      if ( B_IS0(input_[1]))
	     output(HiZ);
      else if (B_ISX(input_[0]))
	     output(reduce_strength(input_[0]));
      else if (B_ISZ(input_[0]))
	     output(HiZ);
      else if (B_IS0(input_[0])) {
	    if (B_IS1(input_[1])) {
		output(reduce_strength(input_[0]));
	    } else {
		output(StL);
	    }
      } else {
	    if (B_IS1(input_[1])) {
		output(reduce_strength(input_[0]));
	    } else {
		output(StH);
	    }
      }
}

vvm_pmos::vvm_pmos(unsigned long d)
: vvm_1bit_out(d)
{
      input_[0] = StX;
      input_[1] = StX;
}


void vvm_pmos::init_I(unsigned key, vpip_bit_t val)
{
      assert(key < 2);
      input_[key] = val;
}

void vvm_pmos::take_value(unsigned key, vpip_bit_t val)
{
      if (input_[key] == val) return;
      input_[key] = val;

      if ( B_IS1(input_[1]))
	    output(HiZ);
      else if (B_ISX(input_[0]))
	    output(StX);
      else if (B_ISZ(input_[0]))
	    output(HiZ);
      else if (B_IS0(input_[0])) {
	  if (B_IS0(input_[1])) {
	      output(St0);
	  } else {
	      output(StL);
	  }
      } else {
	  if (B_IS0(input_[1])) {
	      output(St1);
	  } else {
	      output(StH);
	  }
      }
}

vvm_rpmos::vvm_rpmos(unsigned long d)
: vvm_1bit_out(d)
{
      input_[0] = StX;
      input_[1] = StX;
}


void vvm_rpmos::init_I(unsigned key, vpip_bit_t val)
{
      assert(key < 2);
      input_[key] = val;
}

void vvm_rpmos::take_value(unsigned key, vpip_bit_t val)
{
      if (input_[key] == val) return;
      input_[key] = val;

      if ( B_IS1(input_[1]))
	    output(HiZ);
      else if (B_ISX(input_[0]))
	    output(reduce_strength(input_[0]));
      else if (B_ISZ(input_[0]))
	    output(HiZ);
      else if (B_IS0(input_[0])) {
	  if (B_IS0(input_[1])) {
	      output(reduce_strength(input_[0]));
	  } else {
	      output(StL);
	  }
      } else {
	  if (B_IS0(input_[1])) {
	      output(reduce_strength(input_[0]));
	  } else {
	      output(StH);
	  }
      }
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

vvm_nor2::vvm_nor2(unsigned long d)
: vvm_1bit_out(d)
{
}

vvm_nor2::~vvm_nor2()
{
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

vvm_notif0::vvm_notif0(unsigned long d)
: vvm_1bit_out(d)
{
      input_[0] = StX;
      input_[1] = StX;
}

vvm_notif0::~vvm_notif0()
{
}

void vvm_notif0::init_I(unsigned key, vpip_bit_t val)
{
      assert(key < 2);
      input_[key] = val;
}

void vvm_notif0::take_value(unsigned key, vpip_bit_t val)
{
      if (input_[key] == val) return;
      input_[key] = val;

      if ( B_IS1(input_[1]))
	    output(HiZ);
      else if ( B_ISX(input_[0]) ||
		B_ISZ(input_[0]))
	    output(StX);
      else if (B_IS0(input_[0])) {
	    if (B_IS0(input_[1])) {
	       output(St1);
	    } else {
	       output(StH);
	    }
      } else {
	  if (B_IS0(input_[1])) {
	      output(St0);
	  } else {
	      output(StL);
	  }
      }
}

vvm_notif1::vvm_notif1(unsigned long d)
: vvm_1bit_out(d)
{
      input_[0] = StX;
      input_[1] = StX;
}

vvm_notif1::~vvm_notif1()
{
}

void vvm_notif1::init_I(unsigned key, vpip_bit_t val)
{
      assert(key < 2);
      input_[key] = val;
}

void vvm_notif1::take_value(unsigned key, vpip_bit_t val)
{
      if (input_[key] == val) return;
      input_[key] = val;

      if ( B_IS0(input_[1]))
	    output(HiZ);
      else if ( B_ISX(input_[0]) ||
		B_ISZ(input_[0]))
	    output(StX);
      else if (B_IS0(input_[0])) {
	    if (B_IS1(input_[1])) {
	       output(St1);
	    } else {
	       output(StH);
	    }
      } else {
	  if (B_IS1(input_[1])) {
	       output(St0);
	    } else {
	       output(StL);
	    }
      }
}

vvm_xnor::vvm_xnor(unsigned wid, unsigned long d)
: vvm_1bit_out(d), width_(wid)
{
      input_ = new vpip_bit_t[wid];
}

vvm_xnor::~vvm_xnor()
{
      delete [] input_;
}

void vvm_xnor::init_I(unsigned idx, vpip_bit_t val)
{
      input_[idx] = val;
}

void vvm_xnor::start()
{
      output(compute_xnor(input_,width_));
}

void vvm_xnor::take_value(unsigned key, vpip_bit_t val)
{
      assert(key < width_);
      if (input_[key] == val) return;
      input_[key] = val;
      output(compute_xnor(input_, width_));
}

vvm_xor::vvm_xor(unsigned wid, unsigned long d)
: vvm_1bit_out(d), width_(wid)
{
      input_ = new vpip_bit_t[wid];
}

vvm_xor::~vvm_xor()
{
      delete [] input_;
}

void vvm_xor::init_I(unsigned idx, vpip_bit_t val)
{
      input_[idx] = val;
}

void vvm_xor::start()
{
      output(compute_xor(input_,width_));
}

void vvm_xor::take_value(unsigned key, vpip_bit_t val)
{
      assert(key < width_);
      if (input_[key] == val) return;
      input_[key] = val;
      output(compute_xor(input_, width_));
}

/*
 * $Log: vvm_gates.cc,v $
 * Revision 1.22  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.21  2001/01/16 03:57:46  steve
 *  Get rid of gate templates.
 *
 * Revision 1.20  2000/12/10 06:42:00  steve
 *  Support delays on continuous assignment from idents. (PR#40)
 *
 * Revision 1.19  2000/11/11 01:52:09  steve
 *  change set for support of nmos, pmos, rnmos, rpmos, notif0, and notif1
 *  change set to correct behavior of bufif0 and bufif1
 *  (Tim Leight)
 *
 *  Also includes fix for PR#27
 *
 * Revision 1.18  2000/07/11 23:08:33  steve
 *  proper init method for bufz devices.
 *
 * Revision 1.17  2000/07/08 22:39:32  steve
 *  pass zero-delay values immediately.
 *
 * Revision 1.16  2000/05/11 01:37:33  steve
 *  Calculate the X output value from drive0 and drive1
 *
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

