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
#ident "$Id: vvp_scope.c,v 1.23 2001/05/02 04:05:16 steve Exp $"
#endif

# include  "vvp_priv.h"
# include  <assert.h>
# include  <malloc.h>

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
      ivl_net_const_t cptr;
      ivl_net_logic_t lptr;
      ivl_signal_t sptr;
      ivl_lpm_t lpm;
      unsigned idx, ndx;

      ivl_nexus_ptr_t driver = 0;

      for (ndx = 0 ;  ndx < ivl_nexus_ptrs(nex) ;  ndx += 1) {
	    ivl_nexus_ptr_t nptr = ivl_nexus_ptr(nex, ndx);
	    unsigned nptr_pin = ivl_nexus_ptr_pin(nptr);

	    lptr = ivl_nexus_ptr_log(nptr);
	    if (lptr && (ivl_logic_type(lptr) == IVL_LO_BUFZ) &&
		(nptr_pin == 0)) {
		  draw_nexus_input(ivl_logic_pin(lptr, 1));

		  assert(driver == 0);
		  driver = nptr;
		  continue;
	    }

	    if (lptr && (ivl_logic_type(lptr) == IVL_LO_PULLDOWN)) {
		  fprintf(vvp_out, "C<0>");
		  assert(driver == 0);
		  driver = nptr;
		  continue;
	    }

	    if (lptr && (ivl_logic_type(lptr) == IVL_LO_PULLUP)) {
		  fprintf(vvp_out, "C<1>");
		  assert(driver == 0);
		  driver = nptr;
		  continue;
	    }

	    if (lptr && (nptr_pin == 0)) {
		  fprintf(vvp_out, "L_%s", ivl_logic_name(lptr));

		  assert(driver == 0);
		  driver = nptr;
		  continue;
	    }

	    sptr = ivl_nexus_ptr_sig(nptr);
	    if (sptr && (ivl_signal_type(sptr) == IVL_SIT_REG)) {
		  fprintf(vvp_out, "V_%s[%u]", ivl_signal_name(sptr),
			  nptr_pin);

		  assert(driver == 0);
		  driver = nptr;
		  continue;
	    }

	    cptr = ivl_nexus_ptr_con(nptr);
	    if (cptr) {
		  const char*bits = ivl_const_bits(cptr);
		  fprintf(vvp_out, "C<%c>", bits[nptr_pin]);
		  driver = nptr;
		  continue;
	    }

	    lpm = ivl_nexus_ptr_lpm(nptr);
	    if (lpm) switch (ivl_lpm_type(lpm)) {

		case IVL_LPM_MUX:
		  for (idx = 0 ;  idx < ivl_lpm_width(lpm) ;  idx += 1)
			if (ivl_lpm_q(lpm, idx) == nex) {
			      fprintf(vvp_out, "L_%s/%u",
				      ivl_lpm_name(lpm), idx);

			      assert(driver == 0);
			      driver = nptr;
			      continue;
			}

	    }

	    assert(ivl_nexus_ptr_drive0(nptr) == IVL_DR_HiZ);
	    assert(ivl_nexus_ptr_drive1(nptr) == IVL_DR_HiZ);
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

      const char*signed_flag = ivl_signal_signed(sig)? "/s" : "";

      fprintf(vvp_out, "V_%s .var%s \"%s\", %d, %d;\n",
	      ivl_signal_name(sig), signed_flag,
	      ivl_signal_basename(sig), msb, lsb);
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

      const char*signed_flag = ivl_signal_signed(sig)? "/s" : "";

      fprintf(vvp_out, "V_%s .net%s \"%s\", %d, %d",
	      ivl_signal_name(sig), signed_flag,
	      ivl_signal_basename(sig), msb, lsb);

      for (idx = 0 ;  idx < ivl_signal_pins(sig) ;  idx += 1) {
	    ivl_nexus_t nex = ivl_signal_pin(sig, idx);
	    fprintf(vvp_out, ", ");
	    draw_nexus_input(nex);
      }

      fprintf(vvp_out, ";\n");
}

static void draw_udp_def(ivl_udp_t udp)
{
  unsigned init;
  int i;

  switch (ivl_udp_init(udp))
    {
    case '0':
      init = 0;
      break;
    case '1':
      init = 1;
      break;
    default:
      init = 2;
      break;
    }

  if (ivl_udp_sequ(udp))
	fprintf(vvp_out, 
		"UDP_%s .udp/sequ \"%s\", %d, %d",
		ivl_udp_name(udp),
		ivl_udp_name(udp),
		ivl_udp_nin(udp),
		init );
  else
	fprintf(vvp_out, 
		"UDP_%s .udp/comb \"%s\", %d",
		ivl_udp_name(udp),
		ivl_udp_name(udp),
		ivl_udp_nin(udp));

  for (i=0; i<ivl_udp_rows(udp); i++)
    fprintf(vvp_out, "\n ,\"%s\"", ivl_udp_row(udp, i) );

  fprintf(vvp_out, ";\n");
}

static void draw_udp_in_scope(ivl_net_logic_t lptr)
{
  unsigned pdx;

  ivl_udp_t udp = ivl_logic_udp(lptr);
  
  static ivl_udp_t *udps = 0x0;
  static int nudps = 0;
  int i;

  for (i=0; i<nudps; i++)
    if (udps[i] == udp)
      break;
  
  if (i >= nudps)
    {
      udps = (ivl_udp_t*)realloc(udps, (nudps+1)*sizeof(ivl_udp_t));
      assert(udps);
      udps[nudps++] = udp;
      draw_udp_def(udp);
    }

  fprintf(vvp_out, "L_%s .udp UDP_%s",
	  ivl_logic_name(lptr), ivl_udp_name(udp));
  
  for (pdx = 1 ;  pdx < ivl_logic_pins(lptr) ;  pdx += 1) 
    {
      ivl_nexus_t nex = ivl_logic_pin(lptr, pdx);
      fprintf(vvp_out, ", ");
      draw_nexus_input(nex);
    }
  
  fprintf(vvp_out, ";\n");
}

static void draw_logic_in_scope(ivl_net_logic_t lptr)
{
	    unsigned pdx;
      const char*ltype = "?";
      char identity_val = '0';


      switch (ivl_logic_type(lptr)) {

          case IVL_LO_UDP:
	    draw_udp_in_scope(lptr);
	    return;

          case IVL_LO_BUFZ:
	/* Skip BUFZ objects. Things that have a bufz as input
	   will use the input to bufz instead. */
	    return;

	  case IVL_LO_PULLDOWN:
	  case IVL_LO_PULLUP:
	      /* Skip pullup and pulldown objects. Things that have
		 pull objects as inputs will instead generate the
		 appropriate C<?> symbol. */
	    return;

	  case IVL_LO_AND:
	    ltype = "AND";
	    identity_val = '1';
	    break;

	  case IVL_LO_BUF:
	    ltype = "BUF";
	    break;

	  case IVL_LO_BUFIF0:
	    ltype = "BUFIF0";
	    break;

	  case IVL_LO_BUFIF1:
	    ltype = "BUFIF1";
	    break;

	  case IVL_LO_NAND:
	    ltype = "NAND";
	    identity_val = '1';
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

	  case IVL_LO_XNOR:
	    ltype = "XNOR";
	    break;

	  case IVL_LO_XOR:
	    ltype = "XOR";
	    break;

	  default:
	    fprintf(stderr, "vvp.tgt: error: Unhandled logic type: %u\n",
		    ivl_logic_type(lptr));
	    ltype = "?";
	    break;
      }

      assert(ivl_logic_pins(lptr) <= 5);

      fprintf(vvp_out, "L_%s .functor %s", ivl_logic_name(lptr), ltype);

      for (pdx = 1 ;  pdx < ivl_logic_pins(lptr) ;  pdx += 1) {
	    ivl_nexus_t nex = ivl_logic_pin(lptr, pdx);
	    fprintf(vvp_out, ", ");
	    draw_nexus_input(nex);
      }

      for ( ;  pdx < 5 ;  pdx += 1) {
	    fprintf(vvp_out, ", C<%c>", identity_val);
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

      } else if ((nany > 4) && ((nneg + npos) == 0)) {
	    unsigned idx;

	    for (idx = 0 ;  idx < nany ;  idx += 4) {
		  unsigned sub, top;

		  fprintf(vvp_out, "E_%s/%u .event edge",
			  ivl_event_name(obj), idx/4);

		  top = idx + 4;
		  if (nany < top)
			top = nany;
		  for (sub = idx ;  sub < top ;  sub += 1) {
			ivl_nexus_t nex = ivl_event_any(obj, sub);
			fprintf(vvp_out, ", ");
			draw_nexus_input(nex);
		  }
		  fprintf(vvp_out, ";\n");
	    }

	    fprintf(vvp_out, "E_%s .event/or E_%s/0",
		    ivl_event_name(obj), ivl_event_name(obj));

	    for (idx = 1 ;  idx < (nany+3)/4 ;  idx += 1)
		  fprintf(vvp_out, ", E_%s/%u", ivl_event_name(obj), idx);

	    fprintf(vvp_out, ";\n");

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

static void draw_lpm_mux(ivl_lpm_t net)
{
      ivl_nexus_t s;
      unsigned idx, width;

	/* XXXX Only support A-B muxes for now. */
      assert(ivl_lpm_size(net) == 2);
      assert(ivl_lpm_selects(net) == 1);

      width = ivl_lpm_width(net);
      s = ivl_lpm_select(net, 0);

      for (idx = 0 ;  idx < width ;  idx += 1) {
	    ivl_nexus_t a = ivl_lpm_data2(net, 0, idx);
	    ivl_nexus_t b = ivl_lpm_data2(net, 1, idx);
	    fprintf(vvp_out, "L_%s/%u .functor MUXZ, ",
		    ivl_lpm_name(net), idx);
	    draw_nexus_input(a);
	    fprintf(vvp_out, ", ");
	    draw_nexus_input(b);
	    fprintf(vvp_out, ", ");
	    draw_nexus_input(s);
	    fprintf(vvp_out, ", C<1>;\n");
      }

}

static void draw_lpm_in_scope(ivl_lpm_t net)
{
      switch (ivl_lpm_type(net)) {
	  case IVL_LPM_MUX:
	    draw_lpm_mux(net);
	    return;

	  default:
	    fprintf(stderr, "XXXX LPM not supported: %s\n",
		    ivl_lpm_name(net));
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

      for (idx = 0 ;  idx < ivl_scope_lpms(net) ;  idx += 1) {
	    ivl_lpm_t lpm = ivl_scope_lpm(net, idx);
	    draw_lpm_in_scope(lpm);
      }

      if (ivl_scope_type(net) == IVL_SCT_TASK)
	    draw_task_definition(net);

      if (ivl_scope_type(net) == IVL_SCT_FUNCTION)
	    draw_func_definition(net);

      ivl_scope_children(net, (ivl_scope_f*) draw_scope, net);
      return 0;
}

/*
 * $Log: vvp_scope.c,v $
 * Revision 1.23  2001/05/02 04:05:16  steve
 *  Remove the init parameter of functors, and instead use
 *  the special C<?> symbols to initialize inputs. This is
 *  clearer and more regular.
 *
 * Revision 1.22  2001/04/30 00:00:27  steve
 *  detect multiple drivers on nexa.
 *
 * Revision 1.21  2001/04/29 23:16:31  steve
 *  Add bufif and pull devices.
 *
 * Revision 1.20  2001/04/26 05:12:02  steve
 *  Implement simple MUXZ for ?: operators.
 *
 * Revision 1.19  2001/04/24 02:59:52  steve
 *  Fix generation of udp/comb definitions.
 *
 * Revision 1.18  2001/04/24 02:23:58  steve
 *  Support for UDP devices in VVP (Stephen Boettcher)
 *
 * Revision 1.17  2001/04/21 02:04:01  steve
 *  Add NAND and XNOR functors.
 *
 * Revision 1.16  2001/04/15 16:37:48  steve
 *  add XOR support.
 *
 * Revision 1.15  2001/04/14 05:11:49  steve
 *  Use event/or for wide anyedge statements.
 *
 * Revision 1.14  2001/04/06 02:28:03  steve
 *  Generate vvp code for functions with ports.
 *
 * Revision 1.13  2001/04/05 01:38:24  steve
 *  Generate signed .net and .var statements.
 *
 * Revision 1.12  2001/04/02 02:28:13  steve
 *  Generate code for task calls.
 *
 * Revision 1.11  2001/04/01 21:34:48  steve
 *  Recognize the BUF device.
 *
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

