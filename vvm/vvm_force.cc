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
#ident "$Id: vvm_force.cc,v 1.4 2001/07/25 03:10:50 steve Exp $"
#endif

# include "config.h"

# include  "vvm_gates.h"
# include  <assert.h>

vvm_force::vvm_force(unsigned w)
: width_(w)
{
      force_flag_ = true;
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
      if (! target_[key]) return;

      if (force_flag_)
	    target_[key]->force_assign(val);
      else
	    target_[key]->cassign(val);
}

void vvm_force::assign(unsigned key, vvm_nexus*tgt)
{
      assert(key < width_);
      assert(target_[key] == 0);
      force_flag_ = false;
      target_[key] = tgt;
      target_[key]->cassign_set(this, key);
      target_[key]->cassign(bits_[key]);
}

void vvm_force::force(unsigned key, vvm_nexus*tgt)
{
      assert(key < width_);
      assert(target_[key] == 0);
      force_flag_ = true;
      target_[key] = tgt;
      target_[key]->force_set(this, key);
      target_[key]->force_assign(bits_[key]);
}

/*
 * This method is to be called from the vvm_nexus only when it has
 * been told to release me.
 */
void vvm_force::release(unsigned key)
{
      assert(target_[key]);
      target_[key] = 0;
}

/*
 * $Log: vvm_force.cc,v $
 * Revision 1.4  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.3  2000/05/11 23:37:27  steve
 *  Add support for procedural continuous assignment.
 *
 * Revision 1.2  2000/04/23 03:45:25  steve
 *  Add support for the procedural release statement.
 *
 * Revision 1.1  2000/04/22 04:20:20  steve
 *  Add support for force assignment.
 *
 */

