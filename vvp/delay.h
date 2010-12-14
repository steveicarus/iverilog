#ifndef __delay_H                                      /* -*- c++ -*- */
#define __delay_H
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

#include "pointers.h"

/*
**  vvp_delay_t del;
**
**  del = vvp_delay_new(n, dels);
**         make a delay from n delay specs in array dels.
**         n = 0, 1, 2, 3, 6, 12.
**
**  unsigned vvp_delay_get(del, from, to);
**         tells the delay for the edge (from->to).
**
**  del = NULL;
**         new delay with zero delay.
**
**  del = new vvp_delay_s(delay);
**         new delay with one spec for all edges.
**
**  del = new vvp_delay_2_s(delay, delay);
**         new delay with two specs for rise and fall delays.
**
**  del = new vvp_delay_3_s(delay);
**         new delay with three specs for rise, fall, and highz delays.
**
**  del = new vvp_delay_6_s(delay, del...);
**         new delay with six specs for all 01z edge delays.
**
**  del = new vvp_delay_12_s(delay, del...);
**         new delay with twelve specs for all edge delays.
**
**  void vvp_delsy_delete(del);
**         delete a delay.
**
**  del = vvp_delay_add(del1, del2);
**         add the delay spaces.  del1 and del2 are deleted.
**
**  del = vvp_delay_set(tgt, src, mask);
**         set then non-masked edges of delay tgt from src.
**         tgt and src are deleted.
*/

struct vvp_delay_s {
      vvp_delay_s(unsigned);
      unsigned delay(unsigned char idx) { return del[tab[idx]]; }
      unsigned size() { return tab[14]+1; }
    protected:
      vvp_delay_s(const unsigned char *t);
    private:
      unsigned char tab[16];
    public:
      unsigned del[1];
};

struct vvp_delay_2_s : public vvp_delay_s {
      vvp_delay_2_s(unsigned, unsigned);
      unsigned dell[4-1];
};

struct vvp_delay_3_s : public vvp_delay_s  {
      vvp_delay_3_s(unsigned, unsigned, unsigned);
      unsigned dell[6-1];
};

struct vvp_delay_12_s : public vvp_delay_s {
      vvp_delay_12_s(unsigned, unsigned, unsigned,
		     unsigned, unsigned, unsigned,
		     unsigned, unsigned, unsigned,
		     unsigned, unsigned, unsigned);
      unsigned dell[12-1];
};

struct vvp_delay_6_s : public vvp_delay_12_s  {
      vvp_delay_6_s(unsigned, unsigned, unsigned,
		    unsigned, unsigned, unsigned);
};

inline static
unsigned vvp_delay_get(vvp_delay_t del, unsigned char oval, unsigned char nval)
{
      unsigned char idx = nval | (oval << 2);
      return del->delay(idx);
}

vvp_delay_t vvp_delay_new(unsigned n, unsigned *dels);
void vvp_delay_delete(vvp_delay_t);
vvp_delay_t vvp_delay_add(vvp_delay_t, vvp_delay_t);
vvp_delay_t vvp_delay_set(vvp_delay_t tgt, vvp_delay_t src,
			  unsigned mask = 0);

#endif // __delay_H
