/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: event.cc,v 1.6 2002/05/18 02:34:11 steve Exp $"
#endif

# include  "event.h"
# include  "compile.h"
# include  "vthread.h"
# include  "schedule.h"

# include  <string.h>
# include  <assert.h>
# include  <stdlib.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif

event_functor_s::event_functor_s(edge_t e) 
      : edge(e), threads(0) 
{}

event_functor_s::~event_functor_s() 
{}

/*
 * Receive a value into an event functor, whether an edge or event/or.
 * Detect edges, if any, then schedule awakened threads.
 */
void event_functor_s::set(vvp_ipoint_t ptr, bool, unsigned val, unsigned)
{
      old_ival = ival;
      put(ptr, val);
	/* Only go through the effort if there is someone interested
	   in the results... */
      if (threads || out) {

	    bool edge_p = true;

	      /* If there is an edge detect lookup table, then use the
		 old input and new input to detect whether this is the
		 requested edge. If there is no edge table, then any
		 set is a match. */
	    if (edge) {
		  
		  unsigned pp = ipoint_port(ptr);
		  
		  unsigned oval = (old_ival >> 2*pp) & 3;
		  unsigned nval = (ival     >> 2*pp) & 3;
		  edge_p = edge & VVP_EDGE(oval,nval);
	    }

	      /* If we detect an edge, then schedule any threads that
		 are attached to this event, then propagate the
		 positive detect to the output.

		 Note that only other events (notably event/or
		 functors) can be connected to event outputs. */

	    if (edge_p) {
		  vthread_t tmp = threads;
		  threads = 0;
		  vthread_schedule_list(tmp);
		  
		  if (out) {
			functor_set(out, 0, St0, true);
		  }
		    // only one output?  Why not propagate?
		    //schedule_assign(out, 0, 0);
	    }
      }
}

/*
**  Create an event functor
**  edge:  compile_event(label, type, argc, argv)
**  or:    compile_event(label, NULL, argc, argv)
**
**  Named events are handled elsewhere.
*/

void compile_event(char*label, char*type,
		   unsigned argc, struct symb_s*argv)
{
      event_functor_s::edge_t edge = vvp_edge_none;

      if (argc && type) {
	    if (strcmp(type,"posedge") == 0)
		  edge = vvp_edge_posedge;
	    else if (strcmp(type,"negedge") == 0)
		  edge = vvp_edge_negedge;
	    else if (strcmp(type,"edge") == 0)
		  edge = vvp_edge_anyedge;

	    assert(argc <= 4 || edge == vvp_edge_none);
      }

      free(type);

      functor_t obj = new event_functor_s(edge);

      vvp_ipoint_t fdx = functor_allocate(1);
      functor_define(fdx, obj);
      define_functor_symbol(label, fdx);
      free(label);

	/* Run through the arguments looking for the functors that are
	   connected to my input ports. For each source functor that I
	   find, connect the output of that functor to the indexed
	   input by inserting myself (complete with the port number in
	   the vvp_ipoint_t) into the list that the source heads.

	   If the source functor is not declared yet, then don't do
	   the link yet. Save the reference to be resolved later. */
      
      if (edge != vvp_edge_none)
	    inputs_connect(fdx, argc, argv);
      else
	    // Are we sure that we have those .event/or 
	    // drivers exclusively?
	    for (unsigned i=0; i<argc; i++) {
		  inputs_connect(fdx, 1, argv+i);
	    }

      free(argv);
}

/*
 * This handles the compile of named events. This functor has no
 * inputs, it is only accessed by behavioral trigger statements, which
 * in vvp are %set instructions.
 */
void compile_named_event(char*label, char*name)
{
      functor_t obj = new event_functor_s(vvp_edge_none);

      vvp_ipoint_t fdx = functor_allocate(1);
      functor_define(fdx, obj);
      define_functor_symbol(label, fdx);

      vpiHandle vpi = vpip_make_named_event(name);
      vpip_attach_to_current_scope(vpi);

      free(label);
}

/*
 * $Log: event.cc,v $
 * Revision 1.6  2002/05/18 02:34:11  steve
 *  Add vpi support for named events.
 *
 *  Add vpi_mode_flag to track the mode of the
 *  vpi engine. This is for error checking.
 *
 * Revision 1.5  2002/03/17 03:24:34  steve
 *  Clean up edge detection code.
 *
 * Revision 1.4  2002/01/24 04:17:46  steve
 *  Spelling in comment.
 *
 * Revision 1.3  2001/12/29 23:59:06  steve
 *  push events through event/or lists.
 *
 * Revision 1.2  2001/11/16 04:22:27  steve
 *  include stdlib.h for portability.
 *
 * Revision 1.1  2001/11/06 03:07:22  steve
 *  Code rearrange. (Stephan Boettcher)
 *
 */
