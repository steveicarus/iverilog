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
#ident "$Id: vvm_force.cc,v 1.1 2000/04/22 04:20:20 steve Exp $"
#endif

# include  "vvm_gates.h"
# include  <assert.h>

vvm_force::vvm_force(unsigned w)
: width_(w)
{
      bits_ = new vpip_bit_t[width_];
      target_ = new vvm_nexus*[width_];
      for (unsigned idx = 0 ;  idx < width_ ;  idx += 1)
	    target_[idx] = 0;
}

vvm_force::~vvm_force()
{
      delete[]bits_;
      delete[]target_;
}

void vvm_force::init_I(unsigned key, vpip_bit_t val)
{
      assert(key < width_);
      bits_[key] = val;
}

void vvm_force::take_value(unsigned key, vpip_bit_t val)
{
      assert(key < width_);
      if (bits_[key] == val)
	    return;

      bits_[key] = val;
      target_[key]->force_assign(val);
}

void vvm_force::force(unsigned key, vvm_nexus*tgt)
{
      assert(key < width_);
      assert(target_[key] == 0);
      target_[key] = tgt;
      target_[key]->force_assign(bits_[key]);
}


/*
 * $Log: vvm_force.cc,v $
 * Revision 1.1  2000/04/22 04:20:20  steve
 *  Add support for force assignment.
 *
 */

