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
#ident "$Id: vvp_scope.c,v 1.10 2001/04/01 01:48:21 steve Exp $"
#endif

# include  "vvp_priv.h"
# include  <assert.h>

/*
 * The draw_scope function draws the major functional items within a
 * scope. This includes the scopes themselves, of course. All the
 * other functions in this file are in support of that task.
 */


/*
 * This function takes a nexus and looks for an input functor. It then
 * draws to the output a string that represents that functor.
 *
 * XXXX This function does not yet support multiple drivers.
 */
void draw_nexus_input(ivl_nexus_t nex)
{
      ivl_net_logic_t lptr;
      ivl_signal_t sptr;
      unsigned ndx;

      for (ndx = 0 ;  ndx < ivl_nexus_ptrs(nex) ;  ndx += 1) {
	    ivl_nexus_ptr_t nptr = ivl_nexus_ptr(nex, ndx);

	    lptr = ivl_nexus_ptr_log(nptr);
	    if (lptr && (ivl_logic_type(lptr) == IVL_LO_BUFZ)) {
		  draw_nexus_input(ivl_logic_pin(lptr, 1));
		  return;
	    }

	    if (lptr && (ivl_nexus_ptr_pin(nptr) == 0)) {
		  fprintf(vvp_out, "L_%s", ivl_logic_name(lptr));
		  return;
	    }

	    sptr = ivl_nexus_ptr_sig(nptr);
	    if (sptr && (ivl_signal_type(sptr) == IVL_SIT_REG)) {
		  fprintf(vvp_out, "V_%s[%u]", ivl_signal_name(sptr),
			  ivl_nexus_ptr_pin(nptr));
		  return;
	    }
      }
}

/*
 * This function draws a reg/int/variable in the scope. This is a very
 * simple device to draw as there are no inputs to connect so no need
 * to scan the nexus.
 */
static void draw_reg_in_scope(ivl_signal_t sig)
{
      int msb = ivl_signal_pins(sig) - 1;
      int lsb = 0;

      fprintf(vvp_out, "V_%s .var \"%s\", %d, %d;\n",
	      ivl_signal_name(sig), ivl_signal_basename(sig),
	      msb, lsb);
}

/*
 * This function draws a net. This is a bit more complicated as we
 * have to find an appropriate functor to connect to the input.
 */
static void draw_net_in_scope(ivl_signal_t sig)
{
      unsigned idx;
      int msb = ivl_signal_pins(sig) - 1;
      int lsb = 0;

      fprintf(vvp_out, "V_%s .net \"%s\", %d, %d",
	      ivl_signal_name(sig), ivl_signal_basename(sig),
	      msb, lsb);

      for (idx = 0 ;  idx < ivl_signal_pins(sig) ;  idx += 1) {
	    ivl_nexus_t nex = ivl_signal_pin(sig, idx);
	    fprintf(vvp_out, ", ");
	    draw_nexus_input(nex);
      }

      fprintf(vvp_out, ";\n");
}

static void draw_logic_in_scope(ivl_net_logic_t lptr)
{
	    unsigned pdx;
      const char*ltype = "?";
      unsigned init_val = 0;

	/* Skip BUFZ objects. Things that have a bufz as input
	   will use the input to bufz instead. */
      if (ivl_logic_type(lptr) == IVL_LO_BUFZ)
	    return;


      switch (ivl_logic_type(lptr)) {

	  case IVL_LO_AND:
	    ltype = "AND";
	    init_val = 0x55;
	    break;

	  case IVL_LO_NOR:
	    ltype = "NOR";
	    break;

	  case IVL_LO_NOT:
	    ltype = "NOT";
	    break;

	  case IVL_LO_OR:
	    ltype = "OR";
	    break;

	  default:
	    ltype = "?";
	    break;
      }

      assert(ivl_logic_pins(lptr) <= 5);

      for (pdx = 1 ; pdx < ivl_logic_pins(lptr) ;  pdx += 1) {
	    unsigned mask = 3 << (pdx - 1);
	    init_val = (init_val & ~mask) | (2 << (pdx - 1));
      }

      fprintf(vvp_out, "L_%s .functor %s, 0x%x",
	      ivl_logic_name(lptr), ltype, init_val);

      for (pdx = 1 ;  pdx < ivl_logic_pins(lptr) ;  pdx += 1) {
	    ivl_nexus_t nex = ivl_logic_pin(lptr, pdx);
	    fprintf(vvp_out, ", ");
	    draw_nexus_input(nex);
      }

      fprintf(vvp_out, ";\n");
}

static void draw_event_in_scope(ivl_event_t obj)
{
      unsigned nany = ivl_event_nany(obj);
      unsigned nneg = ivl_event_nneg(obj);
      unsigned npos = ivl_event_npos(obj);
      if ((nany + nneg + npos) == 0) {
	    fprintf(vvp_out, "E_%s .event \"%s\";\n",
		    ivl_event_name(obj), ivl_event_basename(obj));

      } else {
	    unsigned idx;

	    fprintf(vvp_out, "E_%s .event ", ivl_event_name(obj));

	    if (nany > 0) {
		  assert((nneg + npos) == 0);
		  assert(nany <= 4);

		  fprintf(vvp_out, "edge");

		  for (idx = 0 ;  idx < nany ;  idx += 1) {
			ivl_nexus_t nex = ivl_event_any(obj, idx);
			fprintf(vvp_out, ", ");
			draw_nexus_input(nex);
		  }

	    } else if (nneg > 0) {
		  assert((nany + npos) == 0);
		  fprintf(vvp_out, "negedge");

		  for (idx = 0 ;  idx < nneg ;  idx += 1) {
			ivl_nexus_t nex = ivl_event_neg(obj, idx);
			fprintf(vvp_out, ", ");
			draw_nexus_input(nex);
		  }

	    } else {
		  assert((nany + nneg) == 0);
		  fprintf(vvp_out, "posedge");

		  for (idx = 0 ;  idx < npos ;  idx += 1) {
			ivl_nexus_t nex = ivl_event_pos(obj, idx);
			fprintf(vvp_out, ", ");
			draw_nexus_input(nex);
		  }
	    }

	    fprintf(vvp_out, ";\n");
      }
}

int draw_scope(ivl_scope_t net, ivl_scope_t parent)
{
      unsigned idx;

      if (parent)
	    fprintf(vvp_out, "S_%s .scope \"%s\", S_%s;\n",
		    ivl_scope_name(net), ivl_scope_name(net),
		    ivl_scope_name(parent));
      else
	    fprintf(vvp_out, "S_%s .scope \"%s\";\n",
		    ivl_scope_name(net), ivl_scope_name(net));


	/* Scan the scope for logic devices. For each device, draw out
	   a functor that connects pin 0 to the output, and the
	   remaining pins to inputs. */

      for (idx = 0 ;  idx < ivl_scope_logs(net) ;  idx += 1) {
	    ivl_net_logic_t lptr = ivl_scope_log(net, idx);
	    draw_logic_in_scope(lptr);
      }


	/* Scan the signals (reg and net) and draw the appropriate
	   statements to make the signal function. */

      for (idx = 0 ;  idx < ivl_scope_sigs(net) ;  idx += 1) {
	    ivl_signal_t sig = ivl_scope_sig(net, idx);

	    switch (ivl_signal_type(sig)) {
		case IVL_SIT_REG:
		  draw_reg_in_scope(sig);
		  break;
		default:
		  draw_net_in_scope(sig);
		  break;
	    }
      }

      for (idx = 0 ;  idx < ivl_scope_events(net) ;  idx += 1) {
	    ivl_event_t event = ivl_scope_event(net, idx);
	    draw_event_in_scope(event);
      }

      ivl_scope_children(net, (ivl_scope_f*) draw_scope, net);
      return 0;
}

/*
 * $Log: vvp_scope.c,v $
 * Revision 1.10  2001/04/01 01:48:21  steve
 *  Redesign event information to support arbitrary edge combining.
 *
 * Revision 1.9  2001/03/31 19:29:23  steve
 *  Fix compilation warnings.
 *
 * Revision 1.8  2001/03/29 03:47:13  steve
 *  events can take up to 4 inputs.
 *
 * Revision 1.7  2001/03/28 06:07:40  steve
 *  Add the ivl_event_t to ivl_target, and use that to generate
 *  .event statements in vvp way ahead of the thread that uses it.
 *
 * Revision 1.6  2001/03/27 06:27:41  steve
 *  Generate code for simple @ statements.
 *
 * Revision 1.5  2001/03/25 19:36:12  steve
 *  Draw AND NOR and NOT gates.
 *
 * Revision 1.4  2001/03/25 05:59:47  steve
 *  Recursive make check target.
 *
 * Revision 1.3  2001/03/25 03:53:40  steve
 *  Include signal bit index in functor input.
 *
 * Revision 1.2  2001/03/25 03:25:43  steve
 *  Generate .net statements, and nexus inputs.
 *
 * Revision 1.1  2001/03/21 01:49:43  steve
 *  Scan the scopes of a design, and draw behavioral
 *  blocking  assignments of constants to vectors.
 *
 */

