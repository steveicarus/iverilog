#ifndef __event_H
#define __event_H
/*
 * Copyright (c) 2004 Stephen Williams (steve@icarus.com)
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
#ident "$Id: event.h,v 1.11 2005/06/22 00:04:49 steve Exp $"
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

    public:
      waitable_hooks_s() : threads(0) { }
      vthread_t threads;

    protected:
      void run_waiting_threads_();
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

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit);

    private:
      vvp_bit4_t bits_[4];
      edge_t edge_;
};

extern const vvp_fun_edge::edge_t vvp_edge_posedge;
extern const vvp_fun_edge::edge_t vvp_edge_negedge;
extern const vvp_fun_edge::edge_t vvp_edge_none;

/*
 * The vvp_fun_anyedge functor checks to see if any value in an input
 * vector changes. Unlike the vvp_fun_edge, which watches for the LSB
 * of its inputs to change in a particular direction, the anyedge
 * functor looks at the entire input vector for any change.
 */
class vvp_fun_anyedge : public vvp_net_fun_t, public waitable_hooks_s {

    public:
      explicit vvp_fun_anyedge();
      virtual ~vvp_fun_anyedge();

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit);

    private:
      vvp_vector4_t bits_[4];
};

/*
 * This functor triggers anytime any input is set, no matter what the
 * value. This is similar to a named event, but it has no handle.
 */
class vvp_fun_event_or : public vvp_net_fun_t, public waitable_hooks_s {

    public:
      explicit vvp_fun_event_or();
      ~vvp_fun_event_or();

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit);

    private:
};

/*
 * A named event is simpler then a vvp_fun_edge in that it triggers on
 * any input at all to port-0. The idea here is that behavioral code
 * can use a %set/v instruction to trigger the event.
 */
class vvp_named_event : public vvp_net_fun_t, public waitable_hooks_s {

    public:
      explicit vvp_named_event(struct __vpiHandle*eh);
      ~vvp_named_event();

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit);

    private:
      struct __vpiHandle*handle_;
};


/*
 * $Log: event.h,v $
 * Revision 1.11  2005/06/22 00:04:49  steve
 *  Reduce vvp_vector4 copies by using const references.
 *
 * Revision 1.10  2005/06/17 23:47:02  steve
 *  threads member for waitable_hook_s needs initailizing.
 *
 * Revision 1.9  2005/05/25 05:44:51  steve
 *  Handle event/or with specific, efficient nodes.
 *
 * Revision 1.8  2004/12/29 23:45:13  steve
 *  Add the part concatenation node (.concat).
 *
 *  Add a vvp_event_anyedge class to handle the special
 *  case of .event statements of edge type. This also
 *  frees the posedge/negedge types to handle all 4 inputs.
 *
 *  Implement table functor recv_vec4 method to receive
 *  and process vectors.
 *
 * Revision 1.7  2004/12/18 18:52:44  steve
 *  Rework named events and event/or.
 *
 * Revision 1.6  2004/12/11 02:31:29  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 */
#endif // __event_H
