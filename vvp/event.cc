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
#ifdef HAVE_CVS_IDENT
#ident "$Id: event.cc,v 1.15 2004/12/11 02:31:29 steve Exp $"
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

const vvp_fun_edge::edge_t vvp_edge_anyedge = 0x7bde;
const vvp_fun_edge::edge_t vvp_edge_none    = 0;

vvp_fun_edge::vvp_fun_edge(edge_t e)
: edge_(e)
{
      threads = 0;
}

vvp_fun_edge::~vvp_fun_edge()
{
}

void vvp_fun_edge::recv_vec4(vvp_net_ptr_t port, vvp_vector4_t bit)
{
	// XXXX for now, only support first port.
      assert(port.port() == 0);

	/* See what kind of edge this represents. */
      edge_t mask = VVP_EDGE(bits_.value(0), bit.value(0));

	/* Save the current input for the next time around. */
      bits_ = bit;

      if (threads && (edge_ & mask)) {
	    vthread_t tmp = threads;
	    threads = 0;
	    vthread_schedule_list(tmp);
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
      vvp_fun_edge::edge_t edge = vvp_edge_none;

      if (type) {
	    if (strcmp(type,"posedge") == 0)
		  edge = vvp_edge_posedge;
	    else if (strcmp(type,"negedge") == 0)
		  edge = vvp_edge_negedge;
	    else if (strcmp(type,"edge") == 0)
		  edge = vvp_edge_anyedge;

	    assert(argc <= 4);
	    free(type);
      }

      vvp_fun_edge*fun = new vvp_fun_edge(edge);

      vvp_net_t* ptr = new vvp_net_t;
      ptr->fun = fun;

      define_functor_symbol(label, ptr);
      free(label);

      inputs_connect(ptr, argc, argv);
}

/*
 * This handles the compile of named events. This functor has no
 * inputs, it is only accessed by behavioral trigger statements, which
 * in vvp are %set instructions.
 */
void compile_named_event(char*label, char*name)
{
      fprintf(stderr, "XXXX compile_named_event not implemented\n");

      vvp_net_t*fdx = 0;

      vpiHandle obj = vpip_make_named_event(name, fdx);

      compile_vpi_symbol(label, obj);
      vpip_attach_to_current_scope(obj);

      free(label);
      free(name);
}

/*
 * $Log: event.cc,v $
 * Revision 1.15  2004/12/11 02:31:29  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 */
