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
#ident "$Id: event.cc,v 1.23 2006/12/09 19:06:53 steve Exp $"
#endif

# include  "event.h"
# include  "compile.h"
# include  "vthread.h"
# include  "schedule.h"
# include  "vpi_priv.h"

# include  <string.h>
# include  <assert.h>
# include  <stdlib.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif

# include <iostream>

void waitable_hooks_s::run_waiting_threads_()
{
      if (threads == 0)
	    return;

      vthread_t tmp = threads;
      threads = 0;
      vthread_schedule_list(tmp);
}

inline vvp_fun_edge::edge_t VVP_EDGE(vvp_bit4_t from, vvp_bit4_t to)
{
      return 1 << ((from << 2) | to);
}

const vvp_fun_edge::edge_t vvp_edge_posedge
      = VVP_EDGE(BIT4_0,BIT4_1)
      | VVP_EDGE(BIT4_0,BIT4_X)
      | VVP_EDGE(BIT4_0,BIT4_Z)
      | VVP_EDGE(BIT4_X,BIT4_1)
      | VVP_EDGE(BIT4_Z,BIT4_1)
      ;

const vvp_fun_edge::edge_t vvp_edge_negedge
      = VVP_EDGE(BIT4_1,BIT4_0)
      | VVP_EDGE(BIT4_1,BIT4_X)
      | VVP_EDGE(BIT4_1,BIT4_Z)
      | VVP_EDGE(BIT4_X,BIT4_0)
      | VVP_EDGE(BIT4_Z,BIT4_0)
      ;

const vvp_fun_edge::edge_t vvp_edge_none    = 0;

vvp_fun_edge::vvp_fun_edge(edge_t e, bool debug_flag)
: edge_(e), debug_(debug_flag)
{
      bits_[0] = BIT4_X;
      bits_[1] = BIT4_X;
      bits_[2] = BIT4_X;
      bits_[3] = BIT4_X;
}

vvp_fun_edge::~vvp_fun_edge()
{
}

void vvp_fun_edge::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
	/* See what kind of edge this represents. */
      edge_t mask = VVP_EDGE(bits_[port.port()], bit.value(0));

	/* Save the current input for the next time around. */
      bits_[port.port()] = bit.value(0);

      if ((edge_ == vvp_edge_none) || (edge_ & mask)) {
	    run_waiting_threads_();

	    vvp_net_t*net = port.ptr();
	    vvp_send_vec4(net->out, bit);
      }
}


vvp_fun_anyedge::vvp_fun_anyedge(bool debug_flag)
: debug_(debug_flag)
{
      for (unsigned idx = 0 ;  idx < 4 ;  idx += 1)
	    bitsr_[idx] = 0.0;
}

vvp_fun_anyedge::~vvp_fun_anyedge()
{
}

void vvp_fun_anyedge::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
      unsigned pdx = port.port();
      bool flag = false;

      if (bits_[pdx].size() != bit.size()) {
	    flag = true;

      } else {
	    for (unsigned idx = 0 ;  idx < bit.size() ;  idx += 1) {
		  if (bits_[pdx].value(idx) != bit.value(idx)) {
			flag = true;
			break;
		  }
	    }
      }

      if (flag) {
	    bits_[pdx] = bit;
	    run_waiting_threads_();
	    vvp_net_t*net = port.ptr();
	    vvp_send_vec4(net->out, bit);
      }
}

void vvp_fun_anyedge::recv_real(vvp_net_ptr_t port, double bit)
{
      unsigned pdx = port.port();
      bool flag = false;

      if (bitsr_[pdx] != bit) {
	    flag = true;
	    bitsr_[pdx] = bit;
      }

      if (flag) {
	    run_waiting_threads_();
	    vvp_net_t*net = port.ptr();
	    vvp_send_vec4(net->out, vvp_vector4_t());
      }
}

vvp_fun_event_or::vvp_fun_event_or()
{
}

vvp_fun_event_or::~vvp_fun_event_or()
{
}

void vvp_fun_event_or::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
      run_waiting_threads_();
      vvp_net_t*net = port.ptr();
      vvp_send_vec4(net->out, bit);
}

vvp_named_event::vvp_named_event(struct __vpiHandle*h)
{
      handle_ = h;
}

vvp_named_event::~vvp_named_event()
{
}

void vvp_named_event::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
      run_waiting_threads_();
      vvp_net_t*net = port.ptr();
      vvp_send_vec4(net->out, bit);

      vpip_run_named_event_callbacks(handle_);
}

/*
**  Create an event functor
**  edge:  compile_event(label, type, argc, argv, debug_flag)
**  or:    compile_event(label, NULL, argc, argv, debug_flag)
**
**  Named events are handled elsewhere.
*/

static void compile_event_or(char*label, unsigned argc, struct symb_s*argv);

void compile_event(char*label, char*type,
		   unsigned argc, struct symb_s*argv,
		   bool debug_flag)
{
      vvp_net_fun_t*fun = 0;

      if (type == 0) {
	    compile_event_or(label, argc, argv);
	    return;
      }

      if (strcmp(type,"edge") == 0) {

	    free(type);
	    fun = new vvp_fun_anyedge(debug_flag);

      } else {

	    vvp_fun_edge::edge_t edge = vvp_edge_none;

	    if (strcmp(type,"posedge") == 0)
		  edge = vvp_edge_posedge;
	    else if (strcmp(type,"negedge") == 0)
		  edge = vvp_edge_negedge;

	    assert(argc <= 4);
	    free(type);

	    fun = new vvp_fun_edge(edge, debug_flag);
      }

      vvp_net_t* ptr = new vvp_net_t;
      ptr->fun = fun;

      define_functor_symbol(label, ptr);
      free(label);

      inputs_connect(ptr, argc, argv);
}

static void compile_event_or(char*label, unsigned argc, struct symb_s*argv)
{
      vvp_net_fun_t*fun = new vvp_fun_event_or;
      vvp_net_t* ptr = new vvp_net_t;
      ptr->fun = fun;

      define_functor_symbol(label, ptr);
      free(label);

	/* This is a very special case. Point all the source inputs to
	   the same input. It doesn't matter that the streams get
	   tangled because data values are irrelevant. */
      for (unsigned idx = 0 ;  idx < argc ;  idx += 1) {
	    input_connect(ptr, 0, argv[idx].text);
      }
}

/*
 * This handles the compile of named events. This functor has no
 * inputs, it is only accessed by behavioral trigger statements, which
 * in vvp are %set instructions.
 */
void compile_named_event(char*label, char*name)
{
      vvp_net_t*ptr = new vvp_net_t;

      vpiHandle obj = vpip_make_named_event(name, ptr);
      ptr->fun = new vvp_named_event(obj);

      define_functor_symbol(label, ptr);
      compile_vpi_symbol(label, obj);
      vpip_attach_to_current_scope(obj);

      free(label);
      free(name);
}

/*
 * $Log: event.cc,v $
 * Revision 1.23  2006/12/09 19:06:53  steve
 *  Handle vpiRealVal reads of signals, and real anyedge events.
 *
 * Revision 1.22  2006/11/22 06:10:05  steve
 *  Fix spurious event from net8 that is forced.
 *
 * Revision 1.21  2006/02/21 04:57:26  steve
 *  Callbacks for named event triggers.
 *
 * Revision 1.20  2005/06/22 00:04:49  steve
 *  Reduce vvp_vector4 copies by using const references.
 *
 * Revision 1.19  2005/06/17 23:47:02  steve
 *  threads member for waitable_hook_s needs initailizing.
 *
 * Revision 1.18  2005/05/25 05:44:51  steve
 *  Handle event/or with specific, efficient nodes.
 *
 * Revision 1.17  2004/12/29 23:45:13  steve
 *  Add the part concatenation node (.concat).
 *
 *  Add a vvp_event_anyedge class to handle the special
 *  case of .event statements of edge type. This also
 *  frees the posedge/negedge types to handle all 4 inputs.
 *
 *  Implement table functor recv_vec4 method to receive
 *  and process vectors.
 *
 * Revision 1.16  2004/12/18 18:52:44  steve
 *  Rework named events and event/or.
 *
 * Revision 1.15  2004/12/11 02:31:29  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 */
