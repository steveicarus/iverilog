/*
 * Copyright (c) 2001-2010 Stephan Boettcher <stephan@nevis.columbia.edu>
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

#include "delay.h"
#include <string.h>
#include <assert.h>

inline static unsigned dmin(unsigned d1, unsigned d2)
{
      return (d1 < d2) ? d1 : d2;
}

inline static unsigned dmax(unsigned d1, unsigned d2)
{
      return (d1 > d2) ? d1 : d2;
}

typedef const unsigned char tab_t;
//                            01 0x 0z 10    1x 1z x0 x1    xz z0 z1 zx
static tab_t tab_1 [16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static tab_t tab_4 [16] = { 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 3, 2, 1, 0, 3, 2};
static tab_t tab_6 [16] = { 1, 0, 3, 2, 1, 0, 4, 2, 1, 0, 5, 2, 1, 0, 5, 2};
static tab_t tab_12[16] = { 1, 0, 6, 2, 1, 0, 8, 4, 9, 7,11,10, 5, 3,11,10};

inline unsigned vvp_delay_size(vvp_delay_t del)
{
      return del ? del->size() : 0;
}

inline vvp_delay_s::vvp_delay_s(const unsigned char *t)
{
      memcpy((void*)tab, (void*)t, sizeof(tab));
}

vvp_delay_s::vvp_delay_s(unsigned d)
{
      memcpy((void*)tab, (void*)tab_1, sizeof(tab));
      del[0] = d;
}
vvp_delay_2_s::vvp_delay_2_s(unsigned r, unsigned f)
      : vvp_delay_s(tab_4)
{
      del[0] = r;
      del[1] = f;
      del[2] = dmax(r, f);
      del[3] = dmin(r, f);
}

vvp_delay_3_s::vvp_delay_3_s(unsigned r, unsigned f, unsigned z)
      : vvp_delay_s(tab_6)
{
      del[0] = r;
      del[1] = f;
      del[2] = z;
      del[3] = dmin(r, z);
      del[4] = dmin(f, z);
      del[5] = dmin(r, f);
}

vvp_delay_6_s::vvp_delay_6_s(unsigned r, unsigned f, unsigned rz,
			     unsigned zr, unsigned fz, unsigned zf)
      : vvp_delay_12_s(r, f, rz, zr, fz, zf,
		       dmin(r, rz),
		       dmax(r, zr),
		       dmin(f, fz),
		       dmax(f, zf),
		       dmax(rz, fz),
		       dmin(zr, zf))
{}

vvp_delay_12_s::vvp_delay_12_s(unsigned r, unsigned f, unsigned rz,
			       unsigned zr, unsigned fz, unsigned zf,
			       unsigned rx, unsigned xr, unsigned fx,
			       unsigned xf, unsigned xz, unsigned zx)
      : vvp_delay_s(tab_12)
{
      del[0]  = r;
      del[1]  = f;
      del[2]  = rz;
      del[3]  = zr;
      del[4]  = fz;
      del[5]  = zf;
      del[6]  = rx;
      del[7]  = xr;
      del[8]  = fx;
      del[9]  = xf;
      del[10] = xz;
      del[11] = zx;
}

vvp_delay_t vvp_delay_new(unsigned n, unsigned *dels)
{
      switch (n) {
	  default:
	      assert(0);
	  case 0:
	    return 0;
	  case 1:
	    return new vvp_delay_s(dels[0]);
	  case 2:
	    return new vvp_delay_2_s(dels[0], dels[1]);
	  case 3:
	    return new vvp_delay_3_s(dels[0], dels[1], dels[2]);
	  case 6:
	    return new vvp_delay_6_s(dels[0], dels[1], dels[2],
				     dels[3], dels[4], dels[5]);
	  case 12:
	    return new vvp_delay_12_s(dels[0], dels[1], dels[2],
				      dels[3], dels[4], dels[5],
				      dels[6], dels[7], dels[8],
				      dels[9], dels[10], dels[11]);
      }
}

void vvp_delay_delete(vvp_delay_t del)
{
      switch (vvp_delay_size(del)) {
	  case 1: delete del; break;
	  case 4: delete static_cast<vvp_delay_2_s *>(del); break;
	  case 6: delete static_cast<vvp_delay_3_s *>(del); break;
	  case 12: delete static_cast<vvp_delay_12_s *>(del); break;
      }
}

vvp_delay_t vvp_delay_add(vvp_delay_t d1, vvp_delay_t d2)
{
      unsigned s1 = vvp_delay_size(d1);
      unsigned s2 = vvp_delay_size(d2);
      vvp_delay_t out = s1 > s2 ? d1 : d2;
      if (s1==0 || s2==0)
	    return out;

      vvp_delay_t oth = s1 > s2 ? d2 : d1;
      unsigned    s   = s1 > s2 ? s1 : s2;
      unsigned    so  = s1 > s2 ? s2 : s1;

      if (s==so)
	    for (unsigned i=0; i<s; i++)
		  out->del[i] = oth->del[i];
      else switch (so) {
	  case 1:
	    for (unsigned i=0; i<s; i++)
		  out->del[i] = oth->del[0];
	    break;

	  case 4:
	    switch (s) {
		case 6:
		  out->del[0] = oth->del[0];
		  out->del[1] = oth->del[1];
		  out->del[2] = oth->del[4];
		  out->del[3] = oth->del[4];
		  out->del[4] = oth->del[4];
		  out->del[5] = oth->del[4];
		  break;
		case 12:
		  out->del[ 0] = oth->del[0];
		  out->del[ 1] = oth->del[1];
		  out->del[ 2] = oth->del[0];
		  out->del[ 3] = oth->del[0];
		  out->del[ 4] = oth->del[1];
		  out->del[ 5] = oth->del[1];
		  out->del[ 6] = oth->del[0];
		  out->del[ 7] = oth->del[0];
		  out->del[ 8] = oth->del[1];
		  out->del[ 9] = oth->del[1];
		  out->del[10] = oth->del[2];
		  out->del[11] = oth->del[3];
		  break;
	    }
	  case 6:
	    out->del[ 0] = oth->del[0];
	    out->del[ 1] = oth->del[1];
	    out->del[ 2] = oth->del[2];
	    out->del[ 3] = oth->del[0];
	    out->del[ 4] = oth->del[2];
	    out->del[ 5] = oth->del[1];
	    out->del[ 6] = oth->del[3];
	    out->del[ 7] = oth->del[0];
	    out->del[ 8] = oth->del[4];
	    out->del[ 9] = oth->del[1];
	    out->del[10] = oth->del[2];
	    out->del[11] = oth->del[5];
	    break;
      }

      vvp_delay_delete(oth);
      return out;
}

vvp_delay_t vvp_delay_set(vvp_delay_t tgt, vvp_delay_t src, unsigned mask)
{
      unsigned stgt = vvp_delay_size(tgt);
      unsigned ssrc = vvp_delay_size(src);

      if (stgt == 0)
	    return src;
      if (ssrc == 0)
	    return tgt;

      if (stgt == ssrc) {
	    for (unsigned i=0; i<stgt; i++)
		  if (!(mask & (1<<i)))
			tgt->del[i] = src->del[i];

	    vvp_delay_delete(src);
	    return tgt;
      }
#if 0 // later
      if (mask) {
	    static bool done_that = false;
	    if (!done_that) {
		  vvp_printf(VVP_PRINT_WARNING,
			     "Warning:"
			     " partial replacement of delay values"
			     " of different size"
			     " not supported\n"
			     "   either replace all edges,"
			     " or specify the same number of values\n" );
		  done_that = true;
	    }
      }
#endif

      vvp_delay_delete(tgt);
      return src;
}
