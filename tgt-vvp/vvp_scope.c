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
#ident "$Id: vvp_scope.c,v 1.4 2001/03/25 05:59:47 steve Exp $"
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
static void draw_nexus_input(ivl_nexus_t nex)
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
	    unsigned pdx;
	    ivl_net_logic_t lptr = ivl_scope_log(net, idx);
	    const char*ltype = "?";

	      /* Skip BUFZ objects. Things that have a bufz as input
		 will use the input to bufz instead. */
	    if (ivl_logic_type(lptr) == IVL_LO_BUFZ)
		  continue;

	    switch (ivl_logic_type(lptr)) {

		default:
		  ltype = "?";
		  break;
	    }

	    assert(ivl_logic_pins(lptr) <= 5);
	    fprintf(vvp_out, "L_%s .functor %s",
		    ivl_logic_name(lptr), ltype);

	    for (pdx = 1 ;  pdx < ivl_logic_pins(lptr) ;  pdx += 1) {
		  ivl_nexus_t nex = ivl_logic_pin(lptr, pdx);
		  fprintf(vvp_out, ", ");
		  draw_nexus_input(nex);
	    }

	    fprintf(vvp_out, ";\n");
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

      ivl_scope_children(net, draw_scope, net);
      return 0;
}

/*
 * $Log: vvp_scope.c,v $
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

