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

# include  "config.h"

# include  "functor.h"
# include  <assert.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdio.h>
# include  <stdlib.h>
# include  <string.h>

struct vvp_fvector_s {
      unsigned size;
      union {
	    vvp_ipoint_t iptrs[1];
	    struct {
		  unsigned size;
		  vvp_ipoint_t iptr;
	    } cont;
      };
};

unsigned vvp_fvector_size(vvp_fvector_t v)
{
      return v->size ? v->size : v->cont.size;
}

vvp_ipoint_t vvp_fvector_get(vvp_fvector_t v, unsigned i)
{
      if (!v->size)
	    return ipoint_index(v->cont.iptr, i);
      if (v->size <= i) {
	    fprintf(stderr, "ERROR: index value (%d) must be less than "
	            "element size (%d).\n", i, v->size);
	    assert(0);
      }
      return v->iptrs[i];
}

void vvp_fvector_set(vvp_fvector_t v, unsigned i, vvp_ipoint_t p)
{
      assert(i < v->size);
      v->iptrs[i] = p;
}

vvp_ipoint_t *vvp_fvector_member(vvp_fvector_t v, unsigned i)
{
      assert(i < v->size);
      return &v->iptrs[i];
}

vvp_fvector_t vvp_fvector_continuous_new(unsigned size, vvp_ipoint_t p)
{
      vvp_fvector_t v = (vvp_fvector_t)
	    malloc(sizeof(struct vvp_fvector_s));
      v->size = 0;
      v->cont.size = size;
      v->cont.iptr = p;
      return v;
}

vvp_fvector_t vvp_fvector_new(unsigned size)
{
      assert(size>0);
      vvp_fvector_t v = (vvp_fvector_t)
	    malloc(sizeof(struct vvp_fvector_s)
		   + (size-1)*sizeof(vvp_ipoint_t));
      assert(v);
      v->size = size;
      memset(v->iptrs, 0, size*sizeof(vvp_ipoint_t));
      return v;
}
