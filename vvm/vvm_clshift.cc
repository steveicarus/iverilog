/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vvm_clshift.cc,v 1.4 2002/08/12 01:35:06 steve Exp $"
#endif

# include "config.h"

# include  "vvm_gates.h"

/*
 * This is the implementation of the LMP_CLSHIFT device.
 *
 * The device has input/output width if width_ bits, and a distance
 * input of wdist_ bits. The inputs are stored in the ibits_ array,
 * the data bits first, then the select bits.
 */

vvm_clshift::vvm_clshift(unsigned wid, unsigned wid_dist)
: width_(wid), wdist_(wid_dist)
{
      ibits_ = new vpip_bit_t[width_ + wdist_];
      out_ = new vvm_nexus::drive_t[width_];
      dist_val_ = width_;
      dir_ = St0;

      for (unsigned idx = 0 ;  idx < width_+wdist_ ;  idx += 1)
	    ibits_[idx] = StX;
}

vvm_clshift::~vvm_clshift()
{
      delete[]out_;
      delete[]ibits_;
}

vvm_nexus::drive_t* vvm_clshift::config_rout(unsigned idx)
{
      return out_+idx;
}

unsigned vvm_clshift::key_Data(unsigned idx) const
{
      return idx;
}

unsigned vvm_clshift::key_Distance(unsigned idx) const
{
      return idx+width_;
}

unsigned vvm_clshift::key_Direction(unsigned) const
{
      return 0x20000;
}

void vvm_clshift::init_Data(unsigned idx, vpip_bit_t val)
{
      ibits_[idx] = val;
}

void vvm_clshift::init_Distance(unsigned idx, vpip_bit_t val)
{
      ibits_[width_+idx] = val;
      calculate_dist_();
}

void vvm_clshift::init_Direction(unsigned, vpip_bit_t val)
{
      dir_ = val;
}

void vvm_clshift::take_value(unsigned key, vpip_bit_t val)
{
      if (key == 0x20000) {
	    if (dir_ == val) return;
	    dir_ = val;
	    calculate_dist_();
	    compute_();
	    return;
      }

      if (ibits_[key] == val) return;

      ibits_[key] = val;

      if (key < width_) {
	    if ((dist_val_ + key) >= width_) return;
	    if ((dist_val_ + key) < 0) return;
	    out_[dist_val_ + key].set_value(val);

      } else {
	    calculate_dist_();
	    compute_();
      }
}

void vvm_clshift::compute_()
{ 
	// The dist_val_ is set to width_ if its value is not fully
	// known. In this case, just set the output to unknown.

      if (dist_val_ == (int)width_) {
	    for (unsigned idx = 0 ;  idx < width_ ;  idx += 1)
		  out_[idx].set_value(StX);
	    return;
      }

      for (unsigned idx = 0 ;  idx < width_ ;  idx += 1) {
	    vpip_bit_t val;
	    if ((idx-dist_val_) >= width_) val = St0;
	    else if ((idx-dist_val_) < 0) val = St0;
	    else val = ibits_[idx-dist_val_];
	    out_[idx].set_value(val);
      }
}

void vvm_clshift::calculate_dist_()
{
      int tmp = 0;
      for (unsigned idx = 0 ;  idx < wdist_ ;  idx += 1) {
	    if (B_ISX(ibits_[width_+idx]) || B_ISZ(ibits_[width_+idx])) {
		  tmp = width_;
		  break;
	    }

	    if (B_IS1(ibits_[width_+idx]))
		  tmp |= 1<<idx;

      }

	/* If the shift amount is too large (no matter the direction)
	   then set it to exactly width_ and the compute_ function
	   will handle the case directly. Otherwise, turn the shift
	   amount into a signed value that depends on the direction. */

      if (tmp > (int)width_)
	    tmp = width_;
      else if (B_IS1(dir_))
	    tmp = -tmp;
      dist_val_ = tmp;
}

/*
 * $Log: vvm_clshift.cc,v $
 * Revision 1.4  2002/08/12 01:35:06  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.3  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.2  2000/03/22 04:26:41  steve
 *  Replace the vpip_bit_t with a typedef and
 *  define values for all the different bit
 *  values, including strengths.
 *
 * Revision 1.1  2000/03/17 02:22:03  steve
 *  vvm_clshift implementation without templates.
 *
 */

