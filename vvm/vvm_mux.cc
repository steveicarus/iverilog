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
#ident "$Id: vvm_mux.cc,v 1.2 2000/03/22 04:26:41 steve Exp $"
#endif

# include  "vvm_gates.h"
# include  <assert.h>

/*
 * The array of bits is arranged as:
 *
 *       oooo ss 0000 1111 2222 3333 ...
 *
 */
vvm_mux::vvm_mux(unsigned w, unsigned s, unsigned sw)
: width_(w), size_(s), selwid_(sw)
{
      bits_ = new vpip_bit_t[width_*size_+selwid_+width_];
      out_  = new vvm_nexus::drive_t[width_];

      for (unsigned idx = 0 ;  idx < width_*size_+selwid_+width_ ;  idx += 1)
	    bits_[idx] = StX;
}

vvm_mux::~vvm_mux()
{
      delete[]out_;
      delete[]bits_;
}

vvm_nexus::drive_t* vvm_mux::config_rout(unsigned idx)
{
      assert(idx < width_);
      return out_+idx;
}

unsigned vvm_mux::key_Sel(unsigned idx) const
{
      assert(idx < selwid_);
      return idx + width_;
}

unsigned vvm_mux::key_Data(unsigned wi, unsigned si) const
{
      assert(si < size_);
      assert(wi < width_);
      return width_ + selwid_ + si*width_ + wi;
}

void vvm_mux::init_Sel(unsigned idx, vpip_bit_t val)
{
      assert(idx < selwid_);
      bits_[width_+idx] = val;
}

void vvm_mux::init_Data(unsigned idx, vpip_bit_t val)
{
      assert(idx < width_*size_);
      bits_[width_+selwid_+idx] = val;
}

void vvm_mux::take_value(unsigned key, vpip_bit_t val)
{
      if (bits_[key] == val)
	    return;

      bits_[key] = val;
      evaluate_();
}

void vvm_mux::evaluate_()
{
      vpip_bit_t*tmp = new vpip_bit_t[width_];

      vpip_bit_t*sel = bits_+width_;
      vpip_bit_t*inp = bits_+width_+selwid_;

      compute_mux(tmp, width_, sel, selwid_, inp, size_);
      for (unsigned idx = 0 ;  idx < width_ ;  idx += 1)
	    if (tmp[idx] != bits_[idx]) {
		  bits_[idx] = tmp[idx];
		  out_[idx].set_value(bits_[idx]);
	    }

      delete[]tmp;
}

void compute_mux(vpip_bit_t*out, unsigned wid,
		 const vpip_bit_t*sel, unsigned swid,
		 const vpip_bit_t*dat, unsigned size)
{
      unsigned tmp = 0;
      for (unsigned idx = 0 ;  idx < swid ;  idx += 1) {
	    if (B_ISXZ(sel[idx])) {
		  tmp = size;
		  break;
	    }

	    if (B_IS1(sel[idx]))
		  tmp |= (1<<idx);
      }

      if (tmp >= size) {

	    if (swid > 1) {
		  for (unsigned idx = 0; idx < wid ;  idx += 1)
			out[idx] = StX;
	    } else {
		  const unsigned b0 = 0;
		  const unsigned b1 = wid;
		  for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
			if (dat[idx+b0] == dat[idx+b1])
			      out[idx] = dat[idx+b0];
			else
			      out[idx] = StX;
		  }
	    }

      } else {
	    unsigned b = tmp * wid;
	    for (unsigned idx = 0; idx < wid ;  idx += 1)
		  out[idx] = dat[idx+b];
      }
}

/*
 * $Log: vvm_mux.cc,v $
 * Revision 1.2  2000/03/22 04:26:41  steve
 *  Replace the vpip_bit_t with a typedef and
 *  define values for all the different bit
 *  values, including strengths.
 *
 * Revision 1.1  2000/03/18 01:27:00  steve
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
 */

