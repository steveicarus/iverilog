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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: vvm_compare.cc,v 1.2 2000/03/22 04:26:41 steve Exp $"
#endif

# include  "vvm_gates.h"
# include  <assert.h>

vvm_compare::vvm_compare(unsigned wid)
: width_(wid)
{
      gt_ = StX;
      lt_ = StX;

      ibits_ = new vpip_bit_t[2*width_];
      for (unsigned idx = 0 ;  idx < 2*width_ ;  idx += 1)
	    ibits_[idx] = StX;
}

vvm_compare::~vvm_compare()
{
      delete[]ibits_;
}

void vvm_compare::init_DataA(unsigned idx, vpip_bit_t val)
{
      assert(idx < width_);
      ibits_[idx] = val;
}

void vvm_compare::init_DataB(unsigned idx, vpip_bit_t val)
{
      assert(idx < width_);
      ibits_[width_+idx] = val;
}

vvm_nexus::drive_t* vvm_compare::config_ALB_out()
{
      return &out_lt_;
}

vvm_nexus::drive_t* vvm_compare::config_ALEB_out()
{
      return &out_le_;
}

vvm_nexus::drive_t* vvm_compare::config_AGB_out()
{
      return &out_gt_;
}

vvm_nexus::drive_t* vvm_compare::config_AGEB_out()
{
      return &out_ge_;
}

unsigned vvm_compare::key_DataA(unsigned idx) const
{
      assert(idx < width_);
      return idx;
}

unsigned vvm_compare::key_DataB(unsigned idx) const
{
      assert(idx < width_);
      return width_ + idx;
}

void vvm_compare::take_value(unsigned key, vpip_bit_t val)
{
      if (ibits_[key] == val)
	    return;

      ibits_[key] = val;
      compute_();
}

void vvm_compare::compute_()
{
      vpip_bit_t gt = St0;
      vpip_bit_t lt = St0;

      vpip_bit_t*a = ibits_;
      vpip_bit_t*b = ibits_+width_;

      for (unsigned idx = 0 ;  idx < width_ ;  idx += 1) {
	    gt = greater_with_cascade(a[idx], b[idx], gt);
	    lt = less_with_cascade(a[idx], b[idx], lt);
      }

      if ((gt_ == gt) && (lt_ == lt)) return;
      gt_ = gt;
      lt_ = lt;
      out_lt_.set_value(lt_);
      out_le_.set_value(B_NOT(gt_));
      out_gt_.set_value(gt_);
      out_ge_.set_value(B_NOT(lt_));
}

/*
 * $Log: vvm_compare.cc,v $
 * Revision 1.2  2000/03/22 04:26:41  steve
 *  Replace the vpip_bit_t with a typedef and
 *  define values for all the different bit
 *  values, including strengths.
 *
 * Revision 1.1  2000/03/17 17:25:53  steve
 *  Adder and comparator in nexus style.
 *
 */

