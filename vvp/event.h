#ifndef __event_H
#define __event_H
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
#ident "$Id: event.h,v 1.6 2004/12/11 02:31:29 steve Exp $"
#endif

# include  "vvp_net.h"
# include  "pointers.h"

/*
 *  Event / edge detection functors
 */

/*
 * A "waitable" functor is one that the %wait instruction can wait
 * on. This includes the infrastructure needed to hold threads.
 */
struct waitable_hooks_s {
      vthread_t threads;
};

/*
 * The vvp_fun_edge functor detects events that are edges of various
 * types. This should be hooked to a vvp_net_t that is connected to
 * the output of a signal that we wish to watch for edges.
 */
class vvp_fun_edge : public vvp_net_fun_t, public waitable_hooks_s {

    public:
      typedef unsigned short edge_t;
      explicit vvp_fun_edge(edge_t e);

      virtual ~vvp_fun_edge();

      void recv_vec4(vvp_net_ptr_t port, vvp_vector4_t bit);

    private:
      vvp_vector4_t bits_;
      edge_t edge_;
};

extern const vvp_fun_edge::edge_t vvp_edge_posedge;
extern const vvp_fun_edge::edge_t vvp_edge_negedge;
extern const vvp_fun_edge::edge_t vvp_edge_anyedge;
extern const vvp_fun_edge::edge_t vvp_edge_none;

#if 0
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
#endif

/*
 * $Log: event.h,v $
 * Revision 1.6  2004/12/11 02:31:29  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 * Revision 1.5  2004/10/04 01:10:59  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.4  2002/08/12 01:35:08  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.3  2002/07/17 18:30:01  steve
 *  Fix uninitialized thread pointer in named event.
 *
 * Revision 1.2  2002/05/19 05:18:16  steve
 *  Add callbacks for vpiNamedEvent objects.
 *
 * Revision 1.1  2001/11/06 03:07:22  steve
 *  Code rearrange. (Stephan Boettcher)
 *
 */
#endif // __event_H
