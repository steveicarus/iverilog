#ifndef __event_H
#define __event_H
/*
 * Copyright (c) 2000-2010 Stephen Williams (steve@icarus.com)
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

# include  "functor.h"

/*
 *  Event / edge detection functors
 */

struct event_functor_s: public edge_inputs_functor_s, public waitable_hooks_s {
      typedef unsigned short edge_t;
      explicit event_functor_s(edge_t e);
      virtual ~event_functor_s();
      virtual void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);
      edge_t edge;
};

#define VVP_EDGE(a,b) (1<<(((a)<<2)|(b)))

const event_functor_s::edge_t vvp_edge_posedge
      = VVP_EDGE(0,1)
      | VVP_EDGE(0,2)
      | VVP_EDGE(0,3)
      | VVP_EDGE(2,1)
      | VVP_EDGE(3,1)
      ;

const event_functor_s::edge_t vvp_edge_negedge
      = VVP_EDGE(1,0)
      | VVP_EDGE(1,2)
      | VVP_EDGE(1,3)
      | VVP_EDGE(2,0)
      | VVP_EDGE(3,0)
      ;

const event_functor_s::edge_t vvp_edge_anyedge = 0x7bde;
const event_functor_s::edge_t vvp_edge_none = 0;

/*
 * This is a functor to represent named events. This functor has no
 * inputs, and no output. It is a functor so that the %wait and %set
 * instructions can get at it.
 */
struct named_event_functor_s  : public waitable_hooks_s, public functor_s {

    public:
      explicit named_event_functor_s();
      ~named_event_functor_s();
      void set(vvp_ipoint_t ipt, bool push, unsigned val, unsigned str =0);

      struct __vpiHandle* handle;
};

/*
 * Callback functors.
 */
struct callback_functor_s *vvp_fvector_make_callback
                    (vvp_fvector_t, event_functor_s::edge_t = vvp_edge_none);

#endif // __event_H
